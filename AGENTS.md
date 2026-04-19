# AGENTS.md — Status Indicator Framework (SIF)

C++ SKSE plugin for Skyrim Special Edition that draws SWF-based status icons above NPCs and doors based on configurable JSON rules. Built with CommonLibSSE-ng, CMake, vcpkg, and C++23.

## Build

The build environment lives in `.buildenv/`. CMake source root is `.buildenv/CMakeLists.txt`.

```powershell
# Configure (from .buildenv/)
cmake --preset debug
cmake --preset release
cmake --preset releasewithdeb

# Build
cmake --build --preset debug
cmake --build --preset release
```

Requirements: MSVC (`cl.exe`), Ninja, vcpkg with `VCPKG_ROOT` env var set. Dependencies are resolved via vcpkg (see `.buildenv/vcpkg.json`): `commonlibsse-ng`, `minhook`, `spdlog`, `boost-algorithm`, `xbyak`, `jsoncpp`.

Output DLL goes to the path hardcoded in `.buildenv/CMakeLists.txt` line 40 (`OUTPUT_FOLDER`), or `SKYRIM_MODS_FOLDER` / `SKYRIM_FOLDER` env vars if set.

No tests. No linting.

## Architecture

```
src/
├── plugin.cpp              # Entry point (SKSEPluginLoad)
├── config/                 # JSON rule parsing and condition system
│   ├── ConfigManager       # Singleton: loads rules, settings, exposes IAPI
│   ├── ConditionParser     # Parses JSON match fields into Condition objects
│   ├── Condition.h         # Condition types: ExactMatch, Bool, Range, FormID,
│   │                         Keywords, Perk, MagicEffect, EncounterZone, Not, AnyOf, AllOf
│   ├── conditions/         # EffectMatcher and EncounterZoneMatcher hierarchies
│   ├── ParseUtil           # FormID parsing ("Plugin|0xHEX" format)
│   └── Rule.h              # Rule = IconData + list of AND-combined Conditions
├── core/
│   └── IconManager         # Singleton: manages SWF clip pools, positions icons, handles fade
├── hooks/
│   ├── AttachIcons         # MinHook: hooks quest marker attach to pre-load SWF clips
│   └── RenderIcons         # MinHook: hooks quest marker render to draw SIF icons
├── event/
│   └── CellAttachDetach    # SKSE event sink: tracks refs entering/leaving loaded cells
├── RE/                     # Reverse-engineering extras (offsets, FloatingQuestMarker, Crime)
└── swfhelper/              # SWF/Scaleform integration (ImportData)
api/
└── SIF_API.h               # Public header for other plugins to register custom conditions
```

**Data flow:** `ConfigManager` loads `Data/SKSE/Plugins/SIF/*.json` rules at `kDataLoaded`. `AttachIcons` hook pre-attaches SWF clip pools. `CellAttachDetach` tracks active refs. `RenderIcons` hook evaluates rules per-ref each frame and positions/fades matching icons.

## Code Conventions

- **C++23** with MSVC (`/permissive- /Zc:preprocessor`)
- **PCH.h** is precompiled — includes RE/SKSE headers, `SINGLETON<T>` template, `CaseInsensitiveCompare`, offset macros
- **Singletons** use the `SINGLETON<T>` CRTP template from `PCH.h`
- **Condition system** uses `SIF::ICondition` virtual base. Concrete conditions in `Config::` namespace. All conditions take a `RE::TESObjectREFR*` and return `bool`
- **EffectMatcher** hierarchy mirrors Condition but operates on `RE::ActiveEffect*`
- **FormID format** in JSON: `"PluginName.esm|0x00A2C94"` — parsed by `ParseUtil::ParseFormID`
- **Naming:** PascalCase for types/methods, `_camelCase` for member variables, `kPascalCase` for constants
- **Namespaces:** `Config` for config/rule system, `Hooks` for hook installation, `Events` for event sinks, `SIF` for public API
- **No `using namespace`** in headers (except `PCH.h` has `using namespace std::literals` and `namespace logger = SKSE::log`)
- **Logging:** `SKSE::log` (spdlog) — initialized in `plugin.cpp`
- **JSON library:** jsoncpp (`Json::Value`)

## Configuration

User-facing config docs are in `CONFIGURATION.md`. Schema is `schema.json`.

- **Rules:** `Data/SKSE/Plugins/SIF/*.json` — each file has a `rules` array
- **Settings:** `Data/SKSE/Plugins/SIF.json` — global display params (offsets, spacing, scale)
- **SWF files:** resolved relative to `Data/Interface/`

## Plugin API

`api/SIF_API.h` allows other SKSE plugins to register custom match conditions via `SIF::IAPI::RegisterCondition()`. Registration happens through SKSE messaging (`kMessage_GetAPI = 0xD111`). Listener name: `"StatusIndicatorFramework"`.

## Key Implementation Details

- Pool is keyed by `label` only — do not reuse labels across different SWF sources
- `maxInstances` per label is clamped 1..48; `ConfigManager::GetLoadedIcons()` uses max across all rules
- `encounterZone` matcher only works on teleport-linked doors (no fallback to cell/worldspace zone)
- Top-level `match` fields are AND-combined; `anyOf` provides OR; `not` negates
- Match field names are case-insensitive; nested fields in `magicEffect`/`encounterZone` are case-sensitive
- Keys prefixed with `$` are ignored by the JSON parser (for comments/annotations)
- `OFFSET(se, ae)` and `OFFSET_3(se, ae, vr)` macros handle SE/AE/VR variant offsets
