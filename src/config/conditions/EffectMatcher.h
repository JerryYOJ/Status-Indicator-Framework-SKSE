#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Config
{
	template <class T>
	using EnumMap = std::map<std::string, T, CaseInsensitiveCompare>;

	template <class T>
	bool TryGetEnum(const EnumMap<T>& map, const std::string& key, T& outValue)
	{
		if (const auto it = map.find(key); it != map.end()) {
			outValue = it->second;
			return true;
		}
		return false;
	}

	inline const EnumMap<RE::ActorValue> kSchoolMap = {
		{ "Alteration", RE::ActorValue::kAlteration },
		{ "Conjuration", RE::ActorValue::kConjuration },
		{ "Destruction", RE::ActorValue::kDestruction },
		{ "Illusion", RE::ActorValue::kIllusion },
		{ "Restoration", RE::ActorValue::kRestoration },
	};

	inline const EnumMap<RE::ActorValue> kResistanceMap = {
		{ "DamageResist", RE::ActorValue::kDamageResist },
		{ "PoisonResist", RE::ActorValue::kPoisonResist },
		{ "FireResist", RE::ActorValue::kResistFire },
		{ "ElectricResist", RE::ActorValue::kResistShock },
		{ "FrostResist", RE::ActorValue::kResistFrost },
		{ "MagicResist", RE::ActorValue::kResistMagic },
		{ "DiseaseResist", RE::ActorValue::kResistDisease },
	};

	inline const EnumMap<RE::ActorValue> kActorValueMap = {
		{ "Health", RE::ActorValue::kHealth },
		{ "Magicka", RE::ActorValue::kMagicka },
		{ "Stamina", RE::ActorValue::kStamina },
		{ "HealRate", RE::ActorValue::kHealRate },
		{ "MagickaRate", RE::ActorValue::kMagickaRate },
		{ "StaminaRate", RE::ActorValue::kStaminaRate },
		{ "SpeedMult", RE::ActorValue::kSpeedMult },
		{ "CarryWeight", RE::ActorValue::kCarryWeight },
		{ "OneHanded", RE::ActorValue::kOneHanded },
		{ "TwoHanded", RE::ActorValue::kTwoHanded },
		{ "Marksman", RE::ActorValue::kArchery },
		{ "Block", RE::ActorValue::kBlock },
		{ "Smithing", RE::ActorValue::kSmithing },
		{ "HeavyArmor", RE::ActorValue::kHeavyArmor },
		{ "LightArmor", RE::ActorValue::kLightArmor },
		{ "Pickpocket", RE::ActorValue::kPickpocket },
		{ "Lockpicking", RE::ActorValue::kLockpicking },
		{ "Sneak", RE::ActorValue::kSneak },
		{ "Alchemy", RE::ActorValue::kAlchemy },
		{ "Speechcraft", RE::ActorValue::kSpeech },
		{ "Alteration", RE::ActorValue::kAlteration },
		{ "Conjuration", RE::ActorValue::kConjuration },
		{ "Destruction", RE::ActorValue::kDestruction },
		{ "Illusion", RE::ActorValue::kIllusion },
		{ "Restoration", RE::ActorValue::kRestoration },
		{ "Enchanting", RE::ActorValue::kEnchanting },
	};

	inline const EnumMap<RE::MagicSystem::Delivery> kDeliveryTypeMap = {
		{ "Self", RE::MagicSystem::Delivery::kSelf },
		{ "Touch", RE::MagicSystem::Delivery::kTouch },
		{ "Aimed", RE::MagicSystem::Delivery::kAimed },
		{ "TargetActor", RE::MagicSystem::Delivery::kTargetActor },
		{ "TargetLocation", RE::MagicSystem::Delivery::kTargetLocation },
	};

	inline const EnumMap<RE::MagicSystem::CastingType> kCastingTypeMap = {
		{ "ConstantEffect", RE::MagicSystem::CastingType::kConstantEffect },
		{ "FireAndForget", RE::MagicSystem::CastingType::kFireAndForget },
		{ "Concentration", RE::MagicSystem::CastingType::kConcentration },
		{ "Scroll", RE::MagicSystem::CastingType::kScroll },
	};

	inline const EnumMap<RE::EffectSetting::EffectSettingData::Flag> kEffectFlagMap = {
		{ "Hostile", RE::EffectSetting::EffectSettingData::Flag::kHostile },
		{ "Recover", RE::EffectSetting::EffectSettingData::Flag::kRecover },
		{ "Detrimental", RE::EffectSetting::EffectSettingData::Flag::kDetrimental },
		{ "NoDuration", RE::EffectSetting::EffectSettingData::Flag::kNoDuration },
		{ "NoArea", RE::EffectSetting::EffectSettingData::Flag::kNoArea },
		{ "FXPersist", RE::EffectSetting::EffectSettingData::Flag::kFXPersist },
		{ "GoryVisuals", RE::EffectSetting::EffectSettingData::Flag::kGoryVisuals },
		{ "HideInUI", RE::EffectSetting::EffectSettingData::Flag::kHideInUI },
		{ "Painless", RE::EffectSetting::EffectSettingData::Flag::kPainless },
		{ "NoHitEffect", RE::EffectSetting::EffectSettingData::Flag::kNoHitEffect },
		{ "NoDeathDispel", RE::EffectSetting::EffectSettingData::Flag::kNoDeathDispel },
		{ "PowerAffectsMagnitude", RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsMagnitude },
		{ "PowerAffectsDuration", RE::EffectSetting::EffectSettingData::Flag::kPowerAffectsDuration },
	};

	inline const EnumMap<RE::EffectArchetype> kArchetypeMap = {
		{ "ValueModifier", RE::EffectArchetype::kValueModifier },
		{ "Script", RE::EffectArchetype::kScript },
		{ "Dispel", RE::EffectArchetype::kDispel },
		{ "CureDisease", RE::EffectArchetype::kCureDisease },
		{ "Absorb", RE::EffectArchetype::kAbsorb },
		{ "DualValueModifier", RE::EffectArchetype::kDualValueModifier },
		{ "Calm", RE::EffectArchetype::kCalm },
		{ "Demoralize", RE::EffectArchetype::kDemoralize },
		{ "Frenzy", RE::EffectArchetype::kFrenzy },
		{ "Disarm", RE::EffectArchetype::kDisarm },
		{ "CommandSummoned", RE::EffectArchetype::kCommandSummoned },
		{ "Invisibility", RE::EffectArchetype::kInvisibility },
		{ "Light", RE::EffectArchetype::kLight },
		{ "Lock", RE::EffectArchetype::kLock },
		{ "Open", RE::EffectArchetype::kOpen },
		{ "BoundWeapon", RE::EffectArchetype::kBoundWeapon },
		{ "SummonCreature", RE::EffectArchetype::kSummonCreature },
		{ "DetectLife", RE::EffectArchetype::kDetectLife },
		{ "Telekinesis", RE::EffectArchetype::kTelekinesis },
		{ "Paralysis", RE::EffectArchetype::kParalysis },
		{ "Reanimate", RE::EffectArchetype::kReanimate },
		{ "SoulTrap", RE::EffectArchetype::kSoulTrap },
		{ "TurnUndead", RE::EffectArchetype::kTurnUndead },
		{ "Guide", RE::EffectArchetype::kGuide },
		{ "Cloak", RE::EffectArchetype::kCloak },
		{ "SlowTime", RE::EffectArchetype::kSlowTime },
		{ "Rally", RE::EffectArchetype::kRally },
		{ "EnhanceWeapon", RE::EffectArchetype::kEnhanceWeapon },
		{ "SpawnHazard", RE::EffectArchetype::kSpawnHazard },
		{ "Etherealize", RE::EffectArchetype::kEtherealize },
		{ "Banish", RE::EffectArchetype::kBanish },
		{ "Disguise", RE::EffectArchetype::kDisguise },
		{ "PeakValueModifier", RE::EffectArchetype::kPeakValueModifier },
	};

	class EffectMatcher
	{
	public:
		virtual ~EffectMatcher() = default;
		[[nodiscard]] virtual bool Match(RE::ActiveEffect* effect) const = 0;
	};

	template <class T>
	class EffectExactMatch final : public EffectMatcher
	{
	public:
		using Getter = T (*)(RE::ActiveEffect*);

		EffectExactMatch(T expected, Getter getter) :
			_expected(expected), _getter(getter)
		{}

		bool Match(RE::ActiveEffect* effect) const override
		{
			return _getter(effect) == _expected;
		}

	private:
		T _expected;
		Getter _getter;
	};

	class EffectRangeMatch final : public EffectMatcher
	{
	public:
		using Getter = float (*)(RE::ActiveEffect*);

		EffectRangeMatch(std::optional<float> min, std::optional<float> max, Getter getter) :
			_min(min), _max(max), _getter(getter)
		{}

		bool Match(RE::ActiveEffect* effect) const override
		{
			const float value = _getter(effect);
			if (_min.has_value() && value < _min.value()) {
				return false;
			}
			if (_max.has_value() && value > _max.value()) {
				return false;
			}
			return true;
		}

	private:
		std::optional<float> _min;
		std::optional<float> _max;
		Getter _getter;
	};

	class EffectFormMatch final : public EffectMatcher
	{
	public:
		using Getter = RE::TESForm* (*)(RE::ActiveEffect*);

		EffectFormMatch(std::vector<RE::FormID> ids, Getter getter) :
			_ids(std::move(ids)), _getter(getter)
		{}

		bool Match(RE::ActiveEffect* effect) const override
		{
			auto* form = _getter(effect);
			if (!form) {
				return false;
			}

			for (auto id : _ids) {
				auto* resolved = RE::TESForm::LookupByID(id);
				if (!resolved) {
					continue;
				}

				if (auto* list = resolved->As<RE::BGSListForm>()) {
					if (list->HasForm(form)) {
						return true;
					}
				} else if (resolved->GetFormID() == form->GetFormID()) {
					return true;
				}
			}

			return false;
		}

	private:
		std::vector<RE::FormID> _ids;
		Getter _getter;
	};

	class EffectKeywordMatch final : public EffectMatcher
	{
	public:
		explicit EffectKeywordMatch(std::vector<RE::BGSKeyword*> keywords) :
			_keywords(std::move(keywords))
		{}

		bool Match(RE::ActiveEffect* effect) const override
		{
			auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
			if (!baseEffect) {
				return false;
			}

			for (auto* keyword : _keywords) {
				if (!baseEffect->HasKeyword(keyword)) {
					return false;
				}
			}

			return true;
		}

	private:
		std::vector<RE::BGSKeyword*> _keywords;
	};

	class EffectBitfieldMatch final : public EffectMatcher
	{
	public:
		explicit EffectBitfieldMatch(std::uint32_t flags) :
			_flags(flags)
		{}

		bool Match(RE::ActiveEffect* effect) const override
		{
			auto* baseEffect = effect ? effect->GetBaseObject() : nullptr;
			if (!baseEffect) {
				return false;
			}

			const auto effectFlags = static_cast<std::uint32_t>(baseEffect->data.flags.underlying());
			return (effectFlags & _flags) != 0;
		}

	private:
		std::uint32_t _flags;
	};
}
