#include <MinHook.h>

#include "AttachIcons/AttachIcons.h"
#include "RenderIcons/RenderIcons.h"

namespace Hooks {
    void Install() { 
        MH_Initialize();

        AttachIcons::Install();
        RenderIcons::Install();

        MH_EnableHook(MH_ALL_HOOKS);
        return;
    }
}
