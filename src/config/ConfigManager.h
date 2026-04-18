#pragma once

#include "Rule.h"

namespace Config {
    struct Settings {
        float actorOffsetZ{ 20.0f };
        float genericOffsetZ{ 20.0f };
        float markerOffsetZ{ 20.0f };
        float iconSpacing{ 30.0f };
        float scaleDepthNear{ 200.0f };
        float scaleDepthFar{ 1000.0f };
        float scaleMin{ 35.0f };
        float scaleMax{ 100.0f };
    };

    class ConfigManager : public SINGLETON<ConfigManager>, public SIF::IAPI {
        friend SINGLETON;

    public:
        virtual bool RegisterCondition(
            const char* name,
            SIF::ConditionBuilder builder
        ) override;

        virtual uint32_t GetVersion() const override {
            return 1;
        }

        void LoadConfigs();
        void LoadSettings();

        const Settings& GetSettings() const { return _settings; }

        // Returns all matching icons for a reference
        std::vector<const IconData *> GetIcons(RE::TESObjectREFR* ref) const;

        struct PoolSpec {
            std::string   source;
            std::uint32_t maxInstances{ 0 };
        };

        // Returns per-label pool specs: how many clips to attach and from which SWF
        std::unordered_map<std::string, PoolSpec> GetLoadedIcons() const;

        std::size_t GetRuleCount() const { return _rules.size(); }

    private:
        ConfigManager() = default;

        void LoadFile(const std::filesystem::path &path);

        void LoadExternalConditions();

        std::vector<Rule> _rules;
        Settings _settings;
    };
}
