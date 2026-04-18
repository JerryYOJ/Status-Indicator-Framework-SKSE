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
	if (!baseObject) {
		return false;
	}

	switch (baseObject->GetFormType()) {
	case RE::FormType::Door:
		return ref->extraList.GetTeleportLinkedDoor() && ref->extraList.GetTeleportLinkedDoor().get();
	case RE::FormType::Container:
	case RE::FormType::Activator:
		return true;
	default:
		return false;
	}
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
