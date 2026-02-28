#include "REHandler.h"
#include <age/camera/playercamera.h>
#include <age/vehicle/transmission.h>
#include <age/vehicle/engine.h>
#include <age/vehicle/wheel.h>
#include <age/state/gamestate.h>
#include <age/data/timemgr.h>
#include <age/managers/netmanager.h>
#include <age/mcfe/hub.h>
#include <age/mclobby/lobbyderived.h>

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

    // mcGameState
    InstallCallback("mcGameState::EnterState()", "mcGameState::EnterState()",
        &mcGameState::EnterState, {
            cb::call(0x404BAE),
        });

    InstallVTableHook("mcGameState EnterState", &mcGameState::EnterState, { 0x62D978 });

    // datTimeManager
    InstallCallback("datTimeManager::Update()", "datTimeManager::Update()",
        &datTimeManager::Update, {
            cb::call(0x401CE1),
            cb::call(0x402321),
            cb::call(0x402395),
            cb::call(0x4023F1),
            cb::call(0x42059D),
        });

    // mcNetManager
    InstallCallback("mcNetManager::NotifyJoin()", "mcNetManager::NotifyJoin()", &mcNetManager::NotifyJoin, { cb::call(0x535AA9) });
    InstallVTableHook("mcNetManager::NotifyJoin()", &mcNetManager::NotifyJoin, { 0x656D38 });

    InstallCallback("mcNetManager::NotifyQuit()", "mcNetManager::NotifyQuit()", &mcNetManager::NotifyQuit, { cb::call(0x535BB5) });
    InstallVTableHook("mcNetManager::NotifyQuit()", &mcNetManager::NotifyQuit, { 0x656D3C });

    InstallCallback("mcNetManager::DisplayConnectedState()", "mcNetManager::DisplayConnectedState()", &mcNetManager::DisplayConnectedState, { cb::call(0x4261F1) });
    
    // mcFeHub
    InstallCallback("mcFeHub::sub_419E60()", "mcFeHub::sub_419E60()", &mcFeHub::sub_419E60, { cb::call(0x41DC8C) }); // Exiting from LAN lobby to Frontend

    // mcLobbyDerived
    InstallCallback("mcLobbyDerived::sub_5395E0()", "mcLobbyDerived::sub_5395E0()", &mcLobbyDerived::sub_5395E0, { cb::call(0x418C96) }); // Exiting from Online lobby to Frontend
    
}
