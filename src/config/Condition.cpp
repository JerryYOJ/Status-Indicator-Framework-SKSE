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
}
