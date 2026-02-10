#include "CityModelLimitHandler.h"

void CityModelLimitHandler::Install()
{
    bool enableNoPVSFix = HookConfig::GetBool("Experimental", "EnableNoPVSFix", false);

    if (enableNoPVSFix)
    {
        mem::nop(0x4691E0, 7); // -nopvs crash fix
    }
}
