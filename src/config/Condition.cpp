#include "Condition.h"

namespace Config
{
	bool FormIDCondition::Match(RE::TESObjectREFR* ref) const
	{
		auto* form = _useBase ? ref->GetBaseObject() : static_cast<RE::TESForm*>(ref);
		if (!form) return false;

		for (auto id : _ids) {
			auto* resolved = RE::TESForm::LookupByID(id);
			if (!resolved) continue;

			if (auto* list = resolved->As<RE::BGSListForm>()) {
				if (list->HasForm(form)) return true;
			} else if (id == form->GetFormID()) {
				return true;
			}
		}
		return false;
	}

	bool KeywordsCondition::Match(RE::TESObjectREFR* ref) const
	{
		for (auto* kw : _keywords)
			if (!ref->HasKeyword(kw)) return false;
		return true;
	}

	bool PerkCondition::Match(RE::TESObjectREFR* ref) const
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !_perk) return false;
		return _perk->perkConditions.IsTrue(player, ref);
	}

	bool MagicEffectCondition::Match(RE::TESObjectREFR* ref) const
	{
		auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
		auto* magicTarget = actor ? actor->GetMagicTarget() : nullptr;
		auto* activeEffects = magicTarget ? magicTarget->GetActiveEffectList() : nullptr;
		if (!activeEffects) {
			return false;
		}

		for (auto* activeEffect : *activeEffects) {
			if (!activeEffect) {
				continue;
			}

			bool allMatch = true;
			for (const auto& matcher : _matchers) {
				if (!matcher->Match(activeEffect)) {
					allMatch = false;
					break;
				}
			}

			if (allMatch) {
				return true;
			}
		}

		return false;
	}

	bool EncounterZoneCondition::Match(RE::TESObjectREFR* ref) const
	{
		static REL::Relocation<RE::BGSEncounterZone*(*)(RE::TESObjectREFR*)> GetEncounterZone{RELOCATION_ID(19797, 20202)};
		
		if (!ref) {
			return false;
		}

		RE::BGSEncounterZone* zone = nullptr;

		if (const auto linkedDoor = ref->extraList.GetTeleportLinkedDoor(); linkedDoor) {
			if (linkedDoor && linkedDoor.get()) {
				zone = GetEncounterZone(linkedDoor.get().get());
			}
		}

		if (!zone) {
			return false;
		}

		for (const auto& matcher : _matchers) {
			if (!matcher->Match(zone)) {
				return false;
			}
		}

		return true;
	}

	bool FactionCondition::Match(RE::TESObjectREFR* ref) const
	{
		auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
		if (!actor) {
			return false;
		}

		for (const auto& matcher : _matchers) {
			if (!matcher->Match(actor)) {
				return false;
			}
		}

		return true;
	}
}
