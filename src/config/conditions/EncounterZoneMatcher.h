#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace Config
{
	class EncounterZoneMatcher
	{
	public:
		virtual ~EncounterZoneMatcher() = default;
		[[nodiscard]] virtual bool Match(RE::BGSEncounterZone* zone) const = 0;
	};

	template <class T>
	class EncounterZoneExactMatch final : public EncounterZoneMatcher
	{
	public:
		using Getter = T (*)(RE::BGSEncounterZone*);

		EncounterZoneExactMatch(T expected, Getter getter) :
			_expected(expected), _getter(getter)
		{}

		bool Match(RE::BGSEncounterZone* zone) const override
		{
			return _getter(zone) == _expected;
		}

	private:
		T _expected;
		Getter _getter;
	};

	class EncounterZoneFormMatch final : public EncounterZoneMatcher
	{
	public:
		using Getter = RE::TESForm* (*)(RE::BGSEncounterZone*);

		EncounterZoneFormMatch(std::vector<RE::FormID> ids, Getter getter) :
			_ids(std::move(ids)), _getter(getter)
		{}

		bool Match(RE::BGSEncounterZone* zone) const override
		{
			auto* form = _getter(zone);
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

	class EncounterZoneLevelIntersectionMatch final : public EncounterZoneMatcher
	{
	public:
		EncounterZoneLevelIntersectionMatch(std::optional<float> min, std::optional<float> max) :
			_min(min), _max(max)
		{}

		bool Match(RE::BGSEncounterZone* zone) const override
		{
			if (!zone) {
				return false;
			}

			const float zoneMin = static_cast<float>(zone->data.minLevel);
			const float zoneMax = zone->data.maxLevel > 0 ?
				static_cast<float>(zone->data.maxLevel) :
				std::numeric_limits<float>::max();

			const float queryMin = _min.value_or(std::numeric_limits<float>::lowest());
			const float queryMax = _max.value_or(std::numeric_limits<float>::max());

			return queryMax >= zoneMin && queryMin <= zoneMax;
		}

	private:
		std::optional<float> _min;
		std::optional<float> _max;
	};

	class EncounterZoneDeltaPlayerLevelMatch final : public EncounterZoneMatcher
	{
	public:
		EncounterZoneDeltaPlayerLevelMatch(std::optional<float> min, std::optional<float> max) :
			_min(min), _max(max)
		{}

		bool Match(RE::BGSEncounterZone* zone) const override
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!zone || !player) {
				return false;
			}

			const auto playerLevel = static_cast<std::int32_t>(player->GetLevel());
			std::int32_t encounterLevel = playerLevel;

			if (!zone->data.flags.all(RE::ENCOUNTER_ZONE_DATA::Flag::kMatchPCBelowMinimumLevel) &&
				encounterLevel < zone->data.minLevel) {
				encounterLevel = zone->data.minLevel;
			}

			if (zone->data.maxLevel > 0 && encounterLevel > zone->data.maxLevel) {
				encounterLevel = zone->data.maxLevel;
			}

			const float delta = static_cast<float>(encounterLevel - playerLevel);
			if (_min.has_value() && delta < _min.value()) {
				return false;
			}
			if (_max.has_value() && delta > _max.value()) {
				return false;
			}
			return true;
		}

	private:
		std::optional<float> _min;
		std::optional<float> _max;
	};
}
