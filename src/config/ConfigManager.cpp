#include "ConfigManager.h"
#include "ConditionParser.h"

#include <json/json.h>

namespace Config {
    bool ConfigManager::RegisterCondition(const char* name, SIF::ConditionBuilder builder) {
        if (!name || !builder) return false;

        if (ConditionParser::BuilderMap.contains(name)) {
            logger::warn("SIF API: condition '{}' already registered", name);
            return false;
        }

        ConditionParser::BuilderMap[name] = builder;
        logger::info("SIF API: registered '{}'", name);
        return true;
    }

    void ConfigManager::LoadExternalConditions() {
        SKSE::GetMessagingInterface()->Dispatch(
            SIF::kMessage_GetAPI,
            static_cast<SIF::IAPI*>(getInstance()),
            sizeof(SIF::IAPI*),
            nullptr
        );
    }

    void ConfigManager::LoadConfigs() {
        const auto dataPath =
            std::filesystem::path{"Data/SKSE/Plugins/SIF"};

        if (!std::filesystem::exists(dataPath)) {
            logger::info("Config folder not found: {}", dataPath.string());
            return;
        }

        LoadExternalConditions();

        std::size_t fileCount = 0;
        for (const auto &entry : std::filesystem::directory_iterator(dataPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                LoadFile(entry.path());
                fileCount++;
            }
        }

        logger::info("Loaded {} config files, {} rules total", fileCount,
            _rules.size());
    }

    void ConfigManager::LoadFile(const std::filesystem::path &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            logger::warn("Failed to open config: {}", path.string());
            return;
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = true;
        std::string errs;

        if (!Json::parseFromStream(builder, file, &root, &errs)) {
            logger::error("JSON parse error in {}: {}", path.filename().string(), errs);
            return;
        }

        const auto &rules = root["rules"];
        if (!rules.isArray()) {
            logger::warn("No 'rules' array in {}", path.filename().string());
            return;
        }

        


        std::size_t added = 0;
        for (const auto &ruleJson : rules) {
            if (!ruleJson.isObject()) continue;

            auto rule = ConditionParser::ParseRule(ruleJson);

            if (rule.GetIcon().source.empty()) {
                logger::warn("Rule in {} has no icon source, skipping", path.filename().string());
                continue;
            }

            _rules.push_back(std::move(rule));
            added++;
        }

        logger::info("[{}] Loaded {} rules", path.filename().string(), added);
    }

    std::vector<const IconData *> ConfigManager::GetIcons(RE::TESObjectREFR* ref) const {
        if (!ref) return {};

        std::vector<const IconData *> matches;

        for (const auto &rule : _rules) {
            if (rule.Match(ref)) {
                matches.push_back(&rule.GetIcon());
            }
        }

        return matches;
    }

    std::unordered_map<std::string, ConfigManager::PoolSpec> ConfigManager::GetLoadedIcons() const {
        std::unordered_map<std::string, PoolSpec> result;

        for (const auto &rule : _rules) {
            const auto &icon = rule.GetIcon();
            if (icon.source.empty() || icon.label.empty())
                continue;

            auto &spec = result[icon.label];
            spec.source       = "Interface/" + icon.source;
            spec.maxInstances = std::max(spec.maxInstances, icon.maxInstances);
        }

        return result;
    }
}
