#pragma once

#include "conditions/EffectMatcher.h"
#include "conditions/EncounterZoneMatcher.h"
#include "conditions/FactionMatcher.h"
#include "conditions/PackageMatcher.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace Config
{
	using Condition = SIF::ICondition;

	template <class T>
	class ExactMatch final : public Condition
	{
	public:
		using Getter = T (*)(RE::TESObjectREFR*);

		ExactMatch(T expected, Getter getter) :
			_expected(expected),
			_getter(getter)
		{}

		bool Match(RE::TESObjectREFR* ref) const override
		{
			return _getter(ref) == _expected;
		}

	private:
		T _expected;
		Getter _getter;
	};

	class RangeCondition final : public Condition
	{
	public:
		using Getter = float (*)(RE::TESObjectREFR*);

		RangeCondition(std::optional<float> min, std::optional<float> max, Getter getter) :
			_min(min),
			_max(max),
			_getter(getter)
		{}

		bool Match(RE::TESObjectREFR* ref) const override
		{
			const auto value = _getter(ref);
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

	class BoolCondition final : public Condition
	{
	public:
		using Getter = bool (*)(RE::TESObjectREFR*);

		BoolCondition(bool expected, Getter getter) :
			_expected(expected),
			_getter(getter)
		{}

		bool Match(RE::TESObjectREFR* ref) const override
		{
			return _getter(ref) == _expected;
		}

	private:
		bool _expected;
		Getter _getter;
	};

	class BitfieldCondition final : public Condition
	{
	public:
		using Getter = std::uint32_t (*)(RE::TESObjectREFR*);

		BitfieldCondition(std::uint32_t flags, Getter getter) :
			_flags(flags),
			_getter(getter)
		{}

		bool Match(RE::TESObjectREFR* ref) const override
		{
			return (_getter(ref) & _flags) != 0;
		}

	private:
		std::uint32_t _flags;
		Getter _getter;
	};

	template <class T>
	class SetCondition final : public Condition
	{
	public:
		using Getter = T (*)(RE::TESObjectREFR*);

		SetCondition(std::vector<T> values, Getter getter) :
			_values(std::move(values)), _getter(getter) {}

		bool Match(RE::TESObjectREFR* ref) const override
		{
			const T val = _getter(ref);
			for (const auto& v : _values)
				if (v == val) return true;
			return false;
		}

	private:
		std::vector<T> _values;
		Getter _getter;
	};

	class FuncCondition final : public Condition
	{
	public:
		using Func = std::function<bool(RE::TESObjectREFR*)>;

		explicit FuncCondition(Func fn) : _fn(std::move(fn)) {}

		bool Match(RE::TESObjectREFR* ref) const override { return _fn(ref); }

	private:
		Func _fn;
	};

	class FormIDCondition final : public Condition
	{
	public:
		FormIDCondition(std::vector<RE::FormID> ids, bool useBase)
			: _ids(std::move(ids)), _useBase(useBase) {}

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		std::vector<RE::FormID> _ids;
		bool                    _useBase;
	};

	class KeywordsCondition final : public Condition
	{
	public:
		explicit KeywordsCondition(std::vector<RE::BGSKeyword*> keywords)
			: _keywords(std::move(keywords)) {}

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		std::vector<RE::BGSKeyword*> _keywords;
	};

	// Runs the perk's built-in conditions with player as subject and ref as target
	class PerkCondition final : public Condition
	{
	public:
		explicit PerkCondition(RE::BGSPerk* perk) : _perk(perk) {}

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		RE::BGSPerk* _perk;
	};

	class MagicEffectCondition final : public Condition
	{
	public:
		void AddMatcher(std::unique_ptr<EffectMatcher> matcher)
		{
			_matchers.push_back(std::move(matcher));
		}

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		std::vector<std::unique_ptr<EffectMatcher>> _matchers;
	};

	class EncounterZoneCondition final : public Condition
	{
	public:
		void AddMatcher(std::unique_ptr<EncounterZoneMatcher> matcher)
		{
			_matchers.push_back(std::move(matcher));
		}

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		std::vector<std::unique_ptr<EncounterZoneMatcher>> _matchers;
	};

	class FactionCondition final : public Condition
	{
	public:
		void AddMatcher(std::unique_ptr<FactionMatcher> matcher)
		{
			_matchers.push_back(std::move(matcher));
		}

		bool HasMatchers() const { return !_matchers.empty(); }

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		std::vector<std::unique_ptr<FactionMatcher>> _matchers;
	};

	class PackageCondition final : public Condition
	{
	public:
		void AddMatcher(std::unique_ptr<PackageMatcher> matcher)
		{
			_matchers.push_back(std::move(matcher));
		}

		bool HasMatchers() const { return !_matchers.empty(); }

		bool Match(RE::TESObjectREFR* ref) const override;

	private:
		std::vector<std::unique_ptr<PackageMatcher>> _matchers;
	};

	class NotCondition final : public Condition
	{
	public:
		explicit NotCondition(std::unique_ptr<Condition> inner) :
			_inner(std::move(inner))
		{}

		bool Match(RE::TESObjectREFR* ref) const override
		{
			return !_inner->Match(ref);
		}

	private:
		std::unique_ptr<Condition> _inner;
	};

	class AnyOfCondition final : public Condition
	{
	public:
		void Add(std::unique_ptr<Condition> cond) { _conditions.push_back(std::move(cond)); }
		bool HasConditions() const { return !_conditions.empty(); }

		bool Match(RE::TESObjectREFR* ref) const override
		{
			for (const auto& cond : _conditions)
				if (cond->Match(ref)) return true;
			return false;
		}

	private:
		std::vector<std::unique_ptr<Condition>> _conditions;
	};

	class AllOfCondition final : public Condition
	{
	public:
		void Add(std::unique_ptr<Condition> cond) { _conditions.push_back(std::move(cond)); }
		bool HasConditions() const { return !_conditions.empty(); }

		bool Match(RE::TESObjectREFR* ref) const override
		{
			for (const auto& cond : _conditions)
				if (!cond->Match(ref)) return false;
			return true;
		}

	private:
		std::vector<std::unique_ptr<Condition>> _conditions;
	};
}
