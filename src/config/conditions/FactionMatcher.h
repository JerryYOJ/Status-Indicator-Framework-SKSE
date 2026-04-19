#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace Config
{
	class FactionMatcher
	{
	public:
		virtual ~FactionMatcher() = default;
		[[nodiscard]] virtual bool Match(RE::Actor* actor) const = 0;
	};

	class FactionFormMatch final : public FactionMatcher
	{
	public:
		explicit FactionFormMatch(std::vector<RE::FormID> ids) :
			_ids(std::move(ids))
		{}

		bool Match(RE::Actor* actor) const override
		{
			if (!actor) {
				return false;
			}

			for (auto id : _ids) {
				auto* resolved = RE::TESForm::LookupByID(id);
				if (!resolved) {
					continue;
				}

				auto* faction = resolved->As<RE::TESFaction>();
				if (faction) {
					if (actor->GetFactionRank(faction, false) >= 0) {
						return true;
					}
					continue;
				}

				auto* list = resolved->As<RE::BGSListForm>();
				if (!list) {
					continue;
				}

				bool found = false;
				list->ForEachForm([&](RE::TESForm* form) {
					if (auto* f = form->As<RE::TESFaction>()) {
						if (actor->GetFactionRank(f, false) >= 0) {
							found = true;
							return RE::BSContainer::ForEachResult::kStop;
						}
					}
					return RE::BSContainer::ForEachResult::kContinue;
				});

				if (found) {
					return true;
				}
			}

			return false;
		}

	private:
		std::vector<RE::FormID> _ids;
	};

	class FactionRankMatch final : public FactionMatcher
	{
	public:
		FactionRankMatch(std::optional<float> min, std::optional<float> max) :
			_min(min), _max(max)
		{}

		bool Match(RE::Actor* actor) const override
		{
			if (!actor || !_targetFaction) {
				return false;
			}

			const auto rank = static_cast<float>(actor->GetFactionRank(_targetFaction, false));
			if (rank < 0) {
				return false;
			}

			if (_min.has_value() && rank < _min.value()) {
				return false;
			}
			if (_max.has_value() && rank > _max.value()) {
				return false;
			}
			return true;
		}

		void SetFaction(RE::TESFaction* faction) { _targetFaction = faction; }
		RE::TESFaction* GetFaction() const { return _targetFaction; }

	private:
		RE::TESFaction* _targetFaction{ nullptr };
		std::optional<float> _min;
		std::optional<float> _max;
	};
}
