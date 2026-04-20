#pragma once

#include <memory>
#include <vector>

namespace Config
{
	class PackageMatcher
	{
	public:
		virtual ~PackageMatcher() = default;
		[[nodiscard]] virtual bool Match(RE::TESPackage* package) const = 0;
	};

	class PackageFormMatch final : public PackageMatcher
	{
	public:
		explicit PackageFormMatch(std::vector<RE::FormID> ids) :
			_ids(std::move(ids))
		{}

		bool Match(RE::TESPackage* package) const override
		{
			if (!package) {
				return false;
			}

			for (auto id : _ids) {
				auto* resolved = RE::TESForm::LookupByID(id);
				if (!resolved) {
					continue;
				}

				if (auto* list = resolved->As<RE::BGSListForm>()) {
					if (list->HasForm(package)) {
						return true;
					}
				} else if (resolved->GetFormID() == package->GetFormID()) {
					return true;
				}
			}

			return false;
		}

	private:
		std::vector<RE::FormID> _ids;
	};
}
