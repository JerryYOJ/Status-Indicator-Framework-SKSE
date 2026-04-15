#include "RenderIcons.h"

#include <MinHook.h>

#include "../../core/IconManager.h"

void RenderIcons::Install()
{
	REL::Relocation<LPVOID> addr{ RELOCATION_ID(51676, 51676) };
	MH_CreateHook(addr.get(), Update, (LPVOID*)&_Update);
}

void RenderIcons::Update(RE::FloatingQuestMarker* thiz)
{
	_Update(thiz);
	return IconManager::getInstance()->RenderIcons(thiz);
}
