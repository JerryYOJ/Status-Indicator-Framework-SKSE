#include "ConditionParser.h"
#include "ParseUtil.h"

#include <algorithm>

namespace Config
{
	std::unique_ptr<Condition> ConditionParser::BuildMagicEffect(const Json::Value& val, RE::FormType)
	{
		auto cond = std::make_unique<MagicEffectCondition>();

		if (val.isObject()) {
			for (const auto& name : val.getMemberNames()) {
				if (name.empty() || name[0] == '$') {
					continue;
				}

				if (auto matcher = BuildEffectMatcher(name, val[name])) {
					cond->AddMatcher(std::move(matcher));
				}
			}
		}

		return cond;
	}

	std::unique_ptr<Condition> ConditionParser::BuildEncounterZone(const Json::Value& val, RE::FormType)
	{
		auto cond = std::make_unique<EncounterZoneCondition>();

		if (val.isObject()) {
			for (const auto& name : val.getMemberNames()) {
				if (name.empty() || name[0] == '$') {
					continue;
				}

				if (auto matcher = BuildEncounterZoneMatcher(name, val[name])) {
					cond->AddMatcher(std::move(matcher));
				}
			}
		}

		return cond;
	}

	std::unique_ptr<EffectMatcher> ConditionParser::BuildEffectMatcher(const std::string& name, const Json::Value& val)
	{
		if (name == "formId") {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (!ids.empty()) {
				return std::make_unique<EffectFormMatch>(std::move(ids),
					[](RE::ActiveEffect* effect) -> RE::TESForm* {
						return effect ? effect->GetBaseObject() : nullptr;
					});
			}
			return nullptr;
		}

		if (name == "keywords") {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) {
				return nullptr;
			}

			std::vector<RE::BGSKeyword*> keywords;
			for (auto id : ids) {
				if (auto* keyword = RE::TESForm::LookupByID<RE::BGSKeyword>(id)) {
					keywords.push_back(keyword);
				}
			}

			if (!keywords.empty()) {
				return std::make_unique<EffectKeywordMatch>(std::move(keywords));
			}
			return nullptr;
		}

		if (name == "school") {
			if (!val.isString()) {
				return nullptr;
			}

			const auto school = RE::ActorValueList::GetSingleton()->LookupActorValueByName(val.asCString());
			if (school != RE::ActorValue::kNone) {
				return std::make_unique<EffectExactMatch<RE::ActorValue>>(school,
					[](RE::ActiveEffect* effect) -> RE::ActorValue {
						auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
						return baseEffect ? baseEffect->GetMagickSkill() : RE::ActorValue::kNone;
					});
			}
			return nullptr;
		}

		if (name == "primaryValue") {
			if (!val.isString()) {
				return nullptr;
			}

			const auto actorValue = RE::ActorValueList::GetSingleton()->LookupActorValueByName(val.asCString());
			if (actorValue != RE::ActorValue::kNone) {
				return std::make_unique<EffectExactMatch<RE::ActorValue>>(actorValue,
					[](RE::ActiveEffect* effect) -> RE::ActorValue {
						auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
						return baseEffect ? baseEffect->data.primaryAV : RE::ActorValue::kNone;
					});
			}
			return nullptr;
		}

		if (name == "secondaryValue") {
			if (!val.isString()) {
				return nullptr;
			}

			const auto actorValue = RE::ActorValueList::GetSingleton()->LookupActorValueByName(val.asCString());
			if (actorValue != RE::ActorValue::kNone) {
				return std::make_unique<EffectExactMatch<RE::ActorValue>>(actorValue,
					[](RE::ActiveEffect* effect) -> RE::ActorValue {
						auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
						return baseEffect ? baseEffect->data.secondaryAV : RE::ActorValue::kNone;
					});
			}
			return nullptr;
		}

		if (name == "isHostile") {
			if (val.isBool()) {
				return std::make_unique<EffectExactMatch<bool>>(val.asBool(),
					[](RE::ActiveEffect* effect) -> bool {
						auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
						return baseEffect && baseEffect->IsHostile();
					});
			}
			return nullptr;
		}

		if (name == "isDetrimental") {
			if (val.isBool()) {
				return std::make_unique<EffectExactMatch<bool>>(val.asBool(),
					[](RE::ActiveEffect* effect) -> bool {
						auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
						return baseEffect && baseEffect->IsDetrimental();
					});
			}
			return nullptr;
		}

		logger::warn("Unknown magicEffect matcher: '{}'", name);
		return nullptr;
	}

	std::unique_ptr<EncounterZoneMatcher> ConditionParser::BuildEncounterZoneMatcher(const std::string& name, const Json::Value& val)
	{
		if (name == "formId") {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (!ids.empty()) {
				return std::make_unique<EncounterZoneFormMatch>(std::move(ids),
					[](RE::BGSEncounterZone* zone) -> RE::TESForm* {
						return zone;
					});
			}
			return nullptr;
		}

		if (name == "locationFormId") {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (!ids.empty()) {
				return std::make_unique<EncounterZoneFormMatch>(std::move(ids),
					[](RE::BGSEncounterZone* zone) -> RE::TESForm* {
						return zone ? zone->data.location : nullptr;
					});
			}
			return nullptr;
		}

		if (name == "level") {
			if (!val.isObject()) {
				return nullptr;
			}

			std::optional<float> min;
			std::optional<float> max;
			if (val["min"].isNumeric()) {
				min = static_cast<float>(val["min"].asDouble());
			}
			if (val["max"].isNumeric()) {
				max = static_cast<float>(val["max"].asDouble());
			}

			if (min || max) {
				return std::make_unique<EncounterZoneLevelIntersectionMatch>(min, max);
			}
			return nullptr;
		}

		if (name == "deltaPlayerLevel") {
			std::optional<float> min;
			std::optional<float> max;

			if (val.isNumeric()) {
				const float exact = static_cast<float>(val.asDouble());
				min = exact;
				max = exact;
			} else if (val.isObject()) {
				if (val["min"].isNumeric()) {
					min = static_cast<float>(val["min"].asDouble());
				}
				if (val["max"].isNumeric()) {
					max = static_cast<float>(val["max"].asDouble());
				}
			}

			if (min || max) {
				return std::make_unique<EncounterZoneDeltaPlayerLevelMatch>(min, max);
			}
			return nullptr;
		}

		if (name == "neverResets") {
			if (val.isBool()) {
				return std::make_unique<EncounterZoneExactMatch<bool>>(val.asBool(),
					[](RE::BGSEncounterZone* zone) -> bool {
						return zone && zone->data.flags.all(RE::ENCOUNTER_ZONE_DATA::Flag::kNeverResets);
					});
			}
			return nullptr;
		}

		if (name == "matchPCBelowMinimumLevel") {
			if (val.isBool()) {
				return std::make_unique<EncounterZoneExactMatch<bool>>(val.asBool(),
					[](RE::BGSEncounterZone* zone) -> bool {
						return zone && zone->data.flags.all(RE::ENCOUNTER_ZONE_DATA::Flag::kMatchPCBelowMinimumLevel);
					});
			}
			return nullptr;
		}

		if (name == "disableCombatBoundary") {
			if (val.isBool()) {
				return std::make_unique<EncounterZoneExactMatch<bool>>(val.asBool(),
					[](RE::BGSEncounterZone* zone) -> bool {
						return zone && zone->data.flags.all(RE::ENCOUNTER_ZONE_DATA::Flag::kDisableCombatBoundary);
					});
			}
			return nullptr;
		}

		logger::warn("Unknown encounterZone matcher: '{}'", name);
		return nullptr;
	}

	std::map<std::string, ConditionParser::Builder, CaseInsensitiveCompare> ConditionParser::BuilderMap =
	{
		{ "formType", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isString()) return nullptr;
			static const std::unordered_map<std::string, RE::FormType> kMap{
				{ "NPC",  RE::FormType::NPC },
				{ "Door", RE::FormType::Door },
			};
			const auto it = kMap.find(val.asString());
			if (it == kMap.end()) return nullptr;
			return std::make_unique<ExactMatch<RE::FormType>>(it->second,
				[](RE::TESObjectREFR* ref) -> RE::FormType {
					auto* base = ref->GetBaseObject();
					return base ? base->GetFormType() : RE::FormType::None;
				});
		}},

		{ "formId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<FormIDCondition>(std::move(ids), false);
		}},

		{ "baseFormId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<FormIDCondition>(std::move(ids), true);
		}},

		{ "keywords", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			std::vector<RE::BGSKeyword*> kws;
			for (auto id : ids)
				if (auto* kw = RE::TESForm::LookupByID<RE::BGSKeyword>(id))
					kws.push_back(kw);
			if (kws.empty()) return nullptr;
			return std::make_unique<KeywordsCondition>(std::move(kws));
		}},

		{ "isQuestAlias", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					return ref->extraList.GetByType<RE::ExtraAliasInstanceArray>() != nullptr;
				});
		}},

		{ "worldspaceFormId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<SetCondition<RE::FormID>>(std::move(ids),
				[](RE::TESObjectREFR* ref) -> RE::FormID {
					auto* cell = ref->GetParentCell();
					if (!cell) return 0;
					auto* ws = cell->GetRuntimeData().worldSpace;
					return ws ? ws->GetFormID() : 0;
				});
		}},

		{ "locationFormId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<SetCondition<RE::FormID>>(std::move(ids),
				[](RE::TESObjectREFR* ref) -> RE::FormID {
					auto* loc = ref->GetCurrentLocation();
					return loc ? loc->GetFormID() : 0;
				});
		}},

		{ "conditionPerk", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			const auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			auto* perk = RE::TESForm::LookupByID<RE::BGSPerk>(ids[0]);
			if (!perk) return nullptr;
			return std::make_unique<PerkCondition>(perk);
		}},

		{ "magicEffect", BuildMagicEffect },
		{ "encounterZone", BuildEncounterZone },

		{ "raceFormId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<SetCondition<RE::FormID>>(std::move(ids),
				[](RE::TESObjectREFR* ref) -> RE::FormID {
					auto* actor = ref->As<RE::Actor>();
					if (!actor) return 0;
					auto* race = actor->GetRace();
					return race ? race->GetFormID() : 0;
				});
		}},

		{ "level", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isObject()) return nullptr;
			std::optional<float> min, max;
			if (val.isMember("min")) min = val["min"].asFloat();
			if (val.isMember("max")) max = val["max"].asFloat();
			if (!min && !max) return nullptr;
			return std::make_unique<RangeCondition>(min, max,
				[](RE::TESObjectREFR* ref) -> float {
					auto* actor = ref->As<RE::Actor>();
					return actor ? static_cast<float>(actor->GetLevel()) : 0.0f;
				});
		}},

		{ "isDead", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					return actor && actor->IsDead();
				});
		}},

		{ "isHostile", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					auto* player = RE::PlayerCharacter::GetSingleton();
					return actor && player && actor->IsHostileToActor(player);
				});
		}},

		{ "isEssential", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					return actor && actor->IsEssential();
				});
		}},

		{ "isProtected", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					return actor && actor->IsProtected();
				});
		}},

		{ "isPlayerTeammate", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					return actor && actor->IsPlayerTeammate();
				});
		}},

		{ "isInCombat", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					return actor && actor->IsInCombat();
				});
		}},

		{ "isWitness", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			if (!val.isBool()) return nullptr;
			return std::make_unique<BoolCondition>(val.asBool(),
				[](RE::TESObjectREFR* ref) -> bool {
					auto* actor = ref->As<RE::Actor>();
					if (!actor) return false;
					auto* player = RE::PlayerCharacter::GetSingleton();
					if (!player) return false;
					auto* crimeExtra = player->extraList.GetByType<RE::ExtraPlayerCrimeList>();
					if (!crimeExtra || !crimeExtra->crimes) return false;
					const auto handle = actor->GetHandle();
					for (auto* crime : *crimeExtra->crimes) {
						if (!crime) continue;
						for (const auto& witness : crime->actorsKnowOfCrime)
							if (witness == handle) return true;
					}
					return false;
				});
		}},

		{ "not", [](const Json::Value& val, RE::FormType formType) -> std::unique_ptr<Condition> {
			auto buildNot = [&](const Json::Value& obj) -> std::unique_ptr<Condition> {
				if (!obj.isObject()) return nullptr;
				auto inner = std::make_unique<AllOfCondition>();
				for (const auto& name : obj.getMemberNames()) {
					if (name.empty() || name[0] == '$') continue;
					if (auto it = ConditionParser::BuilderMap.find(name); it != ConditionParser::BuilderMap.end())
						if (auto cond = it->second(obj[name], formType))
							inner->Add(std::move(cond));
				}
				if (!inner->HasConditions()) return nullptr;
				return std::make_unique<NotCondition>(std::move(inner));
			};

			if (val.isObject()) {
				return buildNot(val);
			} else if (val.isArray()) {
				auto all = std::make_unique<AllOfCondition>();
				for (const auto& elem : val)
					if (auto cond = buildNot(elem))
						all->Add(std::move(cond));
				if (!all->HasConditions()) return nullptr;
				return all;
			}
			return nullptr;
		}},

		{ "anyOf", [](const Json::Value& val, RE::FormType formType) -> std::unique_ptr<Condition> {
			if (!val.isArray()) return nullptr;
			auto any = std::make_unique<AnyOfCondition>();
			for (const auto& matchObj : val) {
				if (!matchObj.isObject()) continue;
				auto all = std::make_unique<AllOfCondition>();
				for (const auto& name : matchObj.getMemberNames()) {
					if (name.empty() || name[0] == '$') continue;
					if (auto it = ConditionParser::BuilderMap.find(name); it != ConditionParser::BuilderMap.end())
						if (auto cond = it->second(matchObj[name], formType))
							all->Add(std::move(cond));
				}
				if (all->HasConditions()) any->Add(std::move(all));
			}
			if (!any->HasConditions()) return nullptr;
			return any;
		}},
	};

	Rule ConditionParser::ParseRule(const Json::Value& ruleJson)
	{
		Rule parsed;

		const auto& icon = ruleJson["icon"];
		const auto& match = ruleJson["match"];
		if (!icon.isObject() || !match.isObject()) {
			return parsed;
		}

		IconData iconData;
		iconData.source = icon.get("source", "").asString();
		iconData.label = icon.get("label", "").asString();
		iconData.fadeMaxDistance = icon.get("fadeMaxDistance", iconData.fadeMaxDistance).asFloat();
		iconData.fadeStartDistance = icon.get("fadeStartDistance", iconData.fadeStartDistance).asFloat();
		iconData.maxInstances = std::clamp(icon.get("maxInstances", iconData.maxInstances).asUInt(), 1u, 48u);
		parsed.SetIcon(std::move(iconData));

		static const std::unordered_map<std::string, RE::FormType> kFormTypeMap{
			{ "NPC",  RE::FormType::NPC },
			{ "Door", RE::FormType::Door },
		};

		RE::FormType formType = RE::FormType::None;
		if (const auto& ft = match["formType"]; ft.isString()) {
			if (const auto it = kFormTypeMap.find(ft.asString()); it != kFormTypeMap.end()) {
				formType = it->second;
			} else {
				logger::warn("Unknown formType: '{}'", ft.asString());
			}
		}

		for (const auto& name : match.getMemberNames()) {
			if (name.empty() || name[0] == '$') {
				continue;
			}

			if (auto it = BuilderMap.find(name); it != BuilderMap.end()) {
				auto cond = it->second(match[name], formType);
				if (cond) {
					parsed.AddCondition(std::move(cond));
				}
			} else {
				logger::warn("Unknown match field: '{}'", name);
			}
		}

		return parsed;
	}
}
