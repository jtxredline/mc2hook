#include "REHandler.h"
#include <age/camera/playercamera.h>
#include <age/vehicle/transmission.h>
#include <age/vehicle/engine.h>
#include <age/vehicle/wheel.h>

void REHandler::Install()
{
    // camTrackCS
    InstallCallback("RE Handler (1)", "camTrackCS::UpdateSS()",
        &camTrackCS::UpdateSS, {
            cb::jmp(0x46F140)});

    // vehTransmission
    InstallVTableHook("vehTransmission Update", &vehTransmission::Update, { 0x6507E8 });

    // vehEngine
    InstallVTableHook("vehEngine Update", &vehEngine::Update, { 0x650F9C });

    // vehWheel
    InstallVTableHook("vehWheel Update", &vehWheel::UpdateComp, { 0x650E30 });
    InstallCallback("RE Handler (2)", "vehWheel::UpdateSuspensionRay()",
        &vehWheel::UpdateSuspensionRay, {
            cb::call(0x56A615),
            cb::call(0x56C656),
        });
    InstallCallback("RE Handler (3)", "vehWheel::CalcDispAndDamp()",
        &vehWheel::CalcDispAndDamp, {
            cb::call(0x56E11F),
            cb::call(0x56E142),
        });
    InstallCallback("RE Handler (4)", "vehWheel::ComputeSlipPercent()",
        &vehWheel::ComputeSlipPercent, {
            cb::call(0x56DE0E),
            cb::call(0x56DE2A),
        });
    InstallCallback("RE Handler (5)", "vehWheel::ComputeFriction()",
        &vehWheel::ComputeFriction, {
            cb::call(0x56DF6B),
            cb::call(0x56DFE2),
        });
}
