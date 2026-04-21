#pragma once

#include "Condition.h"

namespace Config {
    struct IconData {
        std::string source; // SWF path
        std::string label;  // Export label
        float fadeMaxDistance{ 1800.0f };   // Distance at which alpha reaches 0 (hard render cutoff)
        float fadeStartDistance{ 300.0f }; // Distance at which alpha begins fading
        std::uint32_t maxInstances{ 5 }; // Max simultaneous instances attached for this icon
        float colorR{ 1.0f };
        float colorG{ 1.0f };
        float colorB{ 1.0f };
        bool hasColor{ false };
    };

    class Rule {
    public:
        void AddCondition(std::unique_ptr<Condition> cond) {
            _conditions.push_back(std::move(cond));
        }

        void SetIcon(IconData icon) { _icon = std::move(icon); }

        const IconData &GetIcon() const { return _icon; }

        bool HasConditions() const { return !_conditions.empty(); }

        bool Match(RE::TESObjectREFR* ref) const {
            if (_conditions.empty())
                return true;

            for (const auto &cond : _conditions) {
                if (!cond->Match(ref))
                    return false;
            }
            return true;
        }

    private:
        std::vector<std::unique_ptr<Condition>> _conditions;
        IconData _icon;
    };
}
