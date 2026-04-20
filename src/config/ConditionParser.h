#pragma once

#include "Rule.h"

#include <json/json.h>

namespace Config
{
	class ConditionParser
	{
		friend class ConfigManager;

	public:
		ConditionParser() = delete;

		static Rule ParseRule(const Json::Value& rule);

	protected:
		using Builder = SIF::ConditionBuilder;
		static std::map<std::string, Builder, CaseInsensitiveCompare> BuilderMap;

		static std::unique_ptr<Condition> BuildMagicEffect(const Json::Value& val, RE::FormType);
		static std::unique_ptr<Condition> BuildEncounterZone(const Json::Value& val, RE::FormType);
		static std::unique_ptr<Condition> BuildFaction(const Json::Value& val, RE::FormType);
		static std::unique_ptr<Condition> BuildPackage(const Json::Value& val, RE::FormType);
		static std::unique_ptr<EffectMatcher> BuildEffectMatcher(const std::string& name, const Json::Value& val);
		static std::unique_ptr<EncounterZoneMatcher> BuildEncounterZoneMatcher(const std::string& name, const Json::Value& val);
		static std::unique_ptr<FactionMatcher> BuildFactionMatcher(const std::string& name, const Json::Value& val, RE::TESFaction*& outFaction);
	};
}
