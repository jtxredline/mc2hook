#include "CityModelLimitHandler.h"

void CityModelLimitHandler::SetCPVSBufferLimit(uint32_t b)
{
    // CPV buffer size limit
    uint32_t  originalCpvSize = 0x482480; // 4.51mb

    double mb = HookConfig::GetFloat("Memory", "CPVBufferSize", 8.0f);
    uint32_t bytes = (uint32_t)(mb * 1024.0 * 1024.0);
    bytes = (bytes + 0xFFF) & ~0xFFFu; // Round up to a 4 KB page
    if (bytes < originalCpvSize) bytes = originalCpvSize; // Never go below the game's own size

    hook::StaticThunk<0x5F8B10>::Call<void>(bytes);
}

void CityModelLimitHandler::Install()
{
    bool enableNoPVSFix = HookConfig::GetBool("Experimental", "EnableNoPVSFix", false);

    if (enableNoPVSFix)
    {
        mem::nop(0x4691E0, 7); // -nopvs crash fix
    }

    InstallCallback("CityModelLimitHandler::SetCPVSBufferLimit()", "CityModelLimitHandler::SetCPVSBufferLimit()",
        &CityModelLimitHandler::SetCPVSBufferLimit, {
            cb::call(0x40ABBF),
        });
}
