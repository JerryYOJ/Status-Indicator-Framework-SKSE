#include "AttachIcons.h"

#include <MinHook.h>

#include "../../core/IconManager.h"

void AttachIcons::Install()
{
	REL::Relocation<LPVOID> addr{ RELOCATION_ID(50782, 51677) };
	MH_CreateHook(addr.get(), InitializeMarkerData, (LPVOID*)&_InitializeMarkerData);
}

void AttachIcons::InitializeMarkerData(RE::FloatingQuestMarker* thiz)
{
	_InitializeMarkerData(thiz);
	return IconManager::getInstance()->InitializeAndAttachIcons(thiz);
}
