#include "CellAttachDetach.h"

#include "../../core/IconManager.h"

void CellAttachDetach::Install()
{
	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESCellAttachDetachEvent>(getInstance());
}

static bool IsSuitableRef(RE::TESObjectREFR* ref)
{
	if (!ref || ref->IsDeleted() || ref->IsDisabled()) {
		return false;
	}

	const auto* baseObject = ref->GetBaseObject();
	return baseObject && baseObject->GetFormType() == RE::FormType::Door && ref->extraList.GetTeleportLinkedDoor();
}

RE::BSEventNotifyControl CellAttachDetach::ProcessEvent(
	const RE::TESCellAttachDetachEvent* event,
	RE::BSTEventSource<RE::TESCellAttachDetachEvent>*)
{
	if (!event || !event->reference) {
		return RE::BSEventNotifyControl::kContinue;
	}

	auto* ref = event->reference.get();
	if (!ref) {
		return RE::BSEventNotifyControl::kContinue;
	}

	if (event->attached) {
		if (IsSuitableRef(ref)) {
			IconManager::getInstance()->AddCachedRef(ref);
		}
	} else {
		IconManager::getInstance()->RemoveCachedRef(ref->GetHandle());
	}

	return RE::BSEventNotifyControl::kContinue;
}
