#include "ConditionParser.h"
#include "ParseUtil.h"

#include <algorithm>

namespace Config
{

	std::map<std::string, ConditionParser::Builder, CaseInsensitiveCompare> ConditionParser::BuilderMap =
	{
		// ── ref conditions ────────────────────────────────────────────────────

		{ "formId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<SetCondition<RE::FormID>>(std::move(ids),
				[](RE::TESObjectREFR* ref) -> RE::FormID { return ref->GetFormID(); });
		}},

		{ "baseFormId", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			return std::make_unique<SetCondition<RE::FormID>>(std::move(ids),
				[](RE::TESObjectREFR* ref) -> RE::FormID {
					auto* base = ref->GetBaseObject();
					return base ? base->GetFormID() : 0;
				});
		}},

		{ "keywords", [](const Json::Value& val, RE::FormType) -> std::unique_ptr<Condition> {
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			std::vector<RE::BGSKeyword*> kws;
			for (auto id : ids)
				if (auto* kw = RE::TESForm::LookupByID<RE::BGSKeyword>(id))
					kws.push_back(kw);
			if (kws.empty()) return nullptr;
			return std::make_unique<FuncCondition>([kws = std::move(kws)](RE::TESObjectREFR* ref) {
				for (auto* kw : kws)
					if (ref->HasKeyword(kw)) return true;
				return false;
			});
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
			auto ids = ParseUtil::ParseFormIDArray(val);
			if (ids.empty()) return nullptr;
			std::vector<RE::BGSPerk*> perks;
			for (auto id : ids)
				if (auto* perk = RE::TESForm::LookupByID<RE::BGSPerk>(id))
					perks.push_back(perk);
			if (perks.empty()) return nullptr;
			return std::make_unique<FuncCondition>([perks = std::move(perks)](RE::TESObjectREFR* ref) {
				auto* actor = ref->As<RE::Actor>();
				if (!actor) return false;
				for (auto* perk : perks)
					if (actor->HasPerk(perk)) return true;
				return false;
			});
		}},

		// ── actor conditions ──────────────────────────────────────────────────

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

		// ── logical operators ─────────────────────────────────────────────────

		{ "not", [](const Json::Value& val, RE::FormType formType) -> std::unique_ptr<Condition> {
			if (!val.isObject()) return nullptr;
			auto inner = std::make_unique<AllOfCondition>();
			for (const auto& name : val.getMemberNames()) {
				if (name.empty() || name[0] == '$' || name == "formType") continue;
				if (auto it = ConditionParser::BuilderMap.find(name); it != ConditionParser::BuilderMap.end())
					if (auto cond = it->second(val[name], formType))
						inner->Add(std::move(cond));
			}
			if (!inner->HasConditions()) return nullptr;
			return std::make_unique<NotCondition>(std::move(inner));
		}},

		{ "anyOf", [](const Json::Value& val, RE::FormType formType) -> std::unique_ptr<Condition> {
			if (!val.isArray()) return nullptr;
			auto any = std::make_unique<AnyOfCondition>();
			for (const auto& matchObj : val) {
				if (!matchObj.isObject()) continue;
				auto all = std::make_unique<AllOfCondition>();
				for (const auto& name : matchObj.getMemberNames()) {
					if (name.empty() || name[0] == '$' || name == "formType") continue;
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
		iconData.renderDistance = icon.get("renderDistance", iconData.renderDistance).asFloat();
		iconData.fadeDistance = icon.get("fadeDistance", iconData.fadeDistance).asFloat();
		iconData.maxInstances = std::clamp(icon.get("maxInstances", iconData.maxInstances).asUInt(), 1u, 48u);
		parsed.SetIcon(std::move(iconData));

		static const std::unordered_map<std::string, RE::FormType> kFormTypeMap{
			{ "Actor", RE::FormType::ActorCharacter },
			{ "Door",  RE::FormType::Door },
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
			if (name.empty() || name[0] == '$' || name == "formType") {
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
