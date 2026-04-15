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
	};
}
