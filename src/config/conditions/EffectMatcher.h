#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace Config
{
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
}
