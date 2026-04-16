# Configuration Guide

SIF loads rule files from:

```text
Data/SKSE/Plugins/SIF/*.json
```

SWF files are resolved relative to:

```text
Data/Interface/
```

Each rule file contains a `rules` array. Every rule has:

- `icon`: what to draw
- `match`: conditions evaluated against `RE::TESObjectREFR*`

SIF also reads a separate global settings file from:

```text
Data/SKSE/Plugins/SIF.json
```

## Quick Start

```json
{
  "$schema": "./schema.json",
  "rules": [
    {
      "icon": {
        "source": "SIF/status.swf",
        "label": "EssentialIcon"
      },
      "match": {
        "isEssential": true
      }
    }
  ]
}
```

## Icon

| Field | Required | Default | Description |
|---|---|---|---|
| `source` | yes | none | SWF path relative to `Interface/` |
| `label` | yes | none | Export name inside the SWF |
| `fadeMaxDistance` | no | `1800.0` | Hard distance cutoff; icons past this distance are skipped |
| `fadeStartDistance` | no | `300.0` | Distance at which alpha begins fading |
| `maxInstances` | no | `5` | Per-label on-screen instance limit for this rule, clamped to `1..48` |

Notes:

- `fadeStartDistance` and `fadeMaxDistance` are used by the runtime alpha fade.
- `maxInstances` also affects pool attachment. `ConfigManager::GetLoadedIcons()` uses the maximum `maxInstances` seen for each label across all rules.
- The current pool implementation is keyed by `label` only. Do not reuse the same `label` across different `source` SWFs.

## Match

All top-level fields in `match` are AND-combined.

- If `match` is empty, the rule currently matches everything.
- Top-level field names are case-insensitive in the built-in parser.
- Nested field names inside `magicEffect` and `encounterZone` are currently case-sensitive.
- `not` and `anyOf` are supported.
- Keys inside `match`, `magicEffect`, and `encounterZone` that start with `$` are ignored by the parser.

### Reference Conditions

These work on any `TESObjectREFR`.

| Field | Type | Description |
|---|---|---|
| `formType` | string | Base form type. Supported values: `NPC`, `Door` |
| `formId` | string or array | Matches the reference FormID |
| `baseFormId` | string or array | Matches the base form FormID |
| `keywords` | string or array | Keyword FormID(s). All listed keywords must be present |
| `isQuestAlias` | bool | True if the ref has `ExtraAliasInstanceArray` |
| `worldspaceFormId` | string or array | Matches the parent cell worldspace FormID |
| `locationFormId` | string or array | Matches `GetCurrentLocation()` FormID |
| `conditionPerk` | string | Perk FormID whose conditions are evaluated with player as subject and ref as target |

### Actor Conditions

These only match when the ref is an actor.

| Field | Type | Description |
|---|---|---|
| `raceFormId` | string or array | Matches actor race FormID |
| `level` | object | Numeric range using `min` and/or `max` |
| `isDead` | bool | Matches actor death state |
| `isHostile` | bool | Matches hostility to the player |
| `isEssential` | bool | Matches essential flag |
| `isProtected` | bool | Matches protected flag |
| `isPlayerTeammate` | bool | Matches follower/teammate state |
| `isInCombat` | bool | Matches combat state |
| `isWitness` | bool | True if the actor is in any of the player's active crime witness lists |
| `isWitnessedCrimeEstablished` | bool | True if the actor is a witness on one of the player's crimes and that crime's `crimeEstablished` flag is set |

### `magicEffect`

`magicEffect` is a nested actor matcher.

- The ref must be an actor.
- The actor must currently have at least one active effect.
- One active effect must satisfy every field inside `magicEffect`.

Supported fields:

| Field | Type | Description |
|---|---|---|
| `formId` | string or array | Matches the active effect base `EffectSetting` FormID |
| `keywords` | string or array | All listed keywords must exist on the active effect base `EffectSetting` |
| `school` | string | Matches `EffectSetting::GetMagickSkill()` using actor value names such as `Destruction` |
| `primaryValue` | string | Matches `EffectSetting::data.primaryAV` using actor value names such as `Health` |
| `secondaryValue` | string | Matches `EffectSetting::data.secondaryAV` using actor value names |
| `isHostile` | bool | Matches `EffectSetting::IsHostile()` |
| `isDetrimental` | bool | Matches `EffectSetting::IsDetrimental()` |

Example:

```json
{
  "match": {
    "formType": "NPC",
    "magicEffect": {
      "school": "Destruction",
      "primaryValue": "Health",
      "isHostile": true
    }
  }
}
```

### `encounterZone`

`encounterZone` is a nested object matcher.

Current implementation behavior:

- It only matches refs that have a teleport-linked door.
- The zone is resolved from the linked destination ref.
- There is currently no fallback to the source ref, parent cell, or worldspace zone in this matcher path.
- The resolved zone must satisfy every field inside `encounterZone`.

Supported fields:

| Field | Type | Description |
|---|---|---|
| `formId` | string or array | Matches the resolved encounter zone FormID |
| `locationFormId` | string or array | Matches `BGSEncounterZone::data.location` |
| `level` | object | Range intersection against the zone `minLevel`/`maxLevel`. Uses `{ "min": ..., "max": ... }` |
| `deltaPlayerLevel` | number or object | Matches `currentEncounterLevel - playerLevel` where current encounter level is derived from zone min/max and `matchPCBelowMinimumLevel` |
| `neverResets` | bool | Matches `kNeverResets` |
| `matchPCBelowMinimumLevel` | bool | Matches `kMatchPCBelowMinimumLevel` |
| `disableCombatBoundary` | bool | Matches `kDisableCombatBoundary` |

Example:

```json
{
  "match": {
    "formType": "Door",
    "encounterZone": {
      "level": { "min": 20, "max": 40 },
      "deltaPlayerLevel": { "min": 3 },
      "neverResets": true
    }
  }
}
```

### Logical Operators

`not`

- Object form: AND-combines inner fields, then negates the result.
- Array form: each object is independently negated, and all of them must pass.

Example:

```json
{
  "match": {
    "formType": "NPC",
    "not": { "isDead": true }
  }
}
```

`anyOf`

- Takes an array of `match` objects.
- At least one element must match.

Example:

```json
{
  "match": {
    "anyOf": [
      { "isEssential": true },
      { "isPlayerTeammate": true }
    ]
  }
}
```

## FormID Format

SIF expects FormIDs in this format:

```json
"Skyrim.esm|0x000A2C94"
```

Arrays are also accepted:

```json
[
  "Skyrim.esm|0x000A2C94",
  "Dragonborn.esm|0x0001CDB9"
]
```

Behavior:

- Plugin names are resolved through `TESDataHandler`.
- If a resolved FormID is a `BGSListForm`, the matcher checks membership with `HasForm(...)`.
- `keywords` expects keyword FormIDs, not editor IDs.

## Range Fields

Current implementation only accepts object ranges for `level`:

```json
{ "min": 10, "max": 50 }
{ "min": 20 }
{ "max": 80 }
```

Bare numbers are not currently supported for `level`.

`encounterZone.level` also uses object ranges only.

`encounterZone.deltaPlayerLevel` accepts either:

```json
5
{ "min": 0, "max": 10 }
```

## Global Settings

Global display settings are loaded from `Data/SKSE/Plugins/SIF.json`.

```json
{
  "actorOffsetZ": 20.0,
  "markerOffsetZ": 20.0,
  "iconSpacing": 30.0,
  "scaleDepthNear": 200.0,
  "scaleDepthFar": 1000.0,
  "scaleMin": 35.0,
  "scaleMax": 100.0
}
```

| Field | Default | Description |
|---|---|---|
| `actorOffsetZ` | `20.0` | Extra Z offset applied to actor anchors |
| `markerOffsetZ` | `20.0` | Extra Z offset applied to object/load-door anchors |
| `iconSpacing` | `30.0` | Horizontal spacing between multiple icons on one target |
| `scaleDepthNear` | `200.0` | Near depth clamp for icon scaling |
| `scaleDepthFar` | `1000.0` | Far depth clamp for icon scaling |
| `scaleMin` | `35.0` | Minimum icon scale percent |
| `scaleMax` | `100.0` | Maximum icon scale percent |

## Examples

Essential NPCs:

```json
{
  "icon": {
    "source": "SIF/status.swf",
    "label": "EssentialIcon"
  },
  "match": {
    "isEssential": true
  }
}
```

Doors only:

```json
{
  "icon": {
    "source": "SIF/doors.swf",
    "label": "DoorIcon",
    "fadeMaxDistance": 600.0,
    "fadeStartDistance": 400.0,
    "maxInstances": 3
  },
  "match": {
    "formType": "Door"
  }
}
```

Specific NPC by base form:

```json
{
  "icon": {
    "source": "SIF/special.swf",
    "label": "LydiaIcon"
  },
  "match": {
    "baseFormId": "Skyrim.esm|0x000A2C94"
  }
}
```

Hostile NPCs in combat:

```json
{
  "icon": {
    "source": "SIF/combat.swf",
    "label": "CombatIcon"
  },
  "match": {
    "formType": "NPC",
    "isHostile": true,
    "isInCombat": true
  }
}
```

Witnesses on crimes that can still be cleared by killing them:

```json
{
  "icon": {
    "source": "SIF/crime.swf",
    "label": "WhiteWitnessIcon"
  },
  "match": {
    "formType": "NPC",
    "isWitness": true,
    "not": { "isWitnessedCrimeEstablished": true }
  }
}
```

Witnesses on crimes that have already become established:

```json
{
  "icon": {
    "source": "SIF/crime.swf",
    "label": "RedWitnessIcon"
  },
  "match": {
    "formType": "NPC",
    "isWitness": true,
    "isWitnessedCrimeEstablished": true
  }
}
```

Actors with a hostile destruction health effect:

```json
{
  "icon": {
    "source": "SIF/status.swf",
    "label": "FireDOT"
  },
  "match": {
    "formType": "NPC",
    "magicEffect": {
      "school": "Destruction",
      "primaryValue": "Health",
      "isHostile": true
    }
  }
}
```

Teleport doors leading to higher-level encounter zones:

```json
{
  "icon": {
    "source": "SIF/doors.swf",
    "label": "DangerDoor"
  },
  "match": {
    "formType": "Door",
    "encounterZone": {
      "level": { "min": 20 },
      "deltaPlayerLevel": { "min": 5 }
    }
  }
}
```

## Developer API

Other plugins can register custom match fields through `api/SIF_API.h`.

```cpp
#include "SIF_API.h"

SIF::ListenForRegistration([](SKSE::MessagingInterface::Message* msg) {
    if (msg->type == SIF::kMessage_GetAPI) {
        auto* api = static_cast<SIF::IAPI*>(msg->data);
        api->RegisterCondition(
            "myCustomField",
            [](const Json::Value& val, RE::FormType formType) -> std::unique_ptr<SIF::ICondition> {
                return std::make_unique<MyCondition>(val);
            });
    }
});
```

Notes:

- `ConditionBuilder` receives `(const Json::Value&, RE::FormType)`.
- Custom field names are matched case-insensitively.
- The current `SIF::ListenForRegistration` implementation registers against the listener name `"StatusIndicatorFramework"`.

## Schema

Use `schema.json` for rule files under `Data/SKSE/Plugins/SIF/`.

It does not validate the separate global settings file `Data/SKSE/Plugins/SIF.json`.
