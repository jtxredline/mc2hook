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
#include <age/gfx/rstate.h>
#include <age/vehicle/vehinput.h>
#include <age/mcdata/mcdata.h>
#include <age/mcconfig/mcconfig.h>
#include <age/input/input.h>
#include <age/vehicle/gyro.h>
#include <age/hud/hudMap.h> //
#include <age/managers/layermgr.h>
#include <age/input/joystick.h>
#include <age/gfx/pipeline.h>

void REHandler::Install()
{
    // gfxPipeline
    InstallCallback("gfxPipeline::gfxWindowCreate()", "gfxPipeline::gfxWindowCreate()", &gfxPipeline::gfxWindowCreate, { cb::call(0x5F1338) });

    InstallCallback("gfxPipeline::InputWindowProc()", "gfxPipeline::InputWindowProc()",
        &gfxPipeline::InputWindowProc, {
            cb::jmp(0x5ED4E9),
            cb::jmp(0x5ED517),
            cb::jmp(0x5ED544) });
    
    // camTrackCS
    InstallCallback("RE Handler (1)", "camTrackCS::UpdateSS()",
        &camTrackCS::UpdateSS, {
            cb::jmp(0x46F140) });

    // vehTransmission
    InstallVTableHook("vehTransmission Update", &vehTransmission::Update, { 0x6507E8 });
    InstallCallback("vehTransmission::Upshift()", "vehTransmission::Upshift()",
        &vehTransmission::Upshift, {
            cb::call(0x46AC79),
            cb::call(0x46B03D),
            cb::call(0x46BC49),
            cb::call(0x5681CB),
            cb::call(0x56896E),
            cb::call(0x568BFA),
        });
    InstallCallback("vehTransmission::Downshift()", "vehTransmission::Downshift()",
        &vehTransmission::Downshift, {
            cb::call(0x46AC9D),
            cb::call(0x46B054),
            cb::call(0x46BC60),
            cb::call(0x5681F0),
            cb::call(0x568993),
            cb::call(0x568BE9),
        });

    InstallCallback("vehTransmission::SetCurrentGear()", "vehTransmission::SetCurrentGear()", &vehTransmission::SetCurrentGear, { cb::jmp(0x5677D0) });

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

    // gfxState

    // Mirror mode
    bool mirrorMode = HookConfig::GetBool("Graphics", "MirrorMode", false);
    if (mirrorMode == true)
    {
        InstallCallback("gfxState::SetCull()", "gfxState::SetCull()",
            &gfxState::SetCull, {
                //cb::call(0x4211B9),
                //cb::call(0x459038),
                //cb::call(0x45D2B4),
                //cb::call(0x45DC55),
                //cb::call(0x45FA57),
                //cb::call(0x49242A),
                cb::call(0x493245),   // Powerup icon related
                //cb::call(0x4944E3),
                //cb::call(0x4A522E),
                //cb::call(0x4A5254),
                //cb::call(0x4A53CF),
                //cb::call(0x4A6027),
                //cb::call(0x4CB3E1), // vehModel car normals 
                //cb::call(0x4CD6DA),
                //cb::call(0x4E0418),
                //cb::call(0x4E04F6),
                //cb::call(0x4E1459),
                //cb::call(0x4E2E24),
                //cb::call(0x4E2E2E),
                //cb::call(0x4E2E43),
                //cb::call(0x4E2E6B), // Car shadow related
                //cb::call(0x4E2EA8),
                //cb::call(0x4E2EB2),
                //cb::call(0x4E2F33),
                cb::call(0x4E2F3D),   // Skybox / car taillight related
                //cb::call(0x4E2F52),
                //cb::call(0x4E2F75),
                //cb::call(0x4E2F97),
                //cb::call(0x4E2FA1),
                //cb::call(0x4ED4C3),
                //cb::call(0x4ED598),
                //cb::call(0x4F3E99),
                //cb::call(0x4F3EE4),
                //cb::call(0x4F3F43),
                //cb::call(0x4F3F82),
                //cb::call(0x4F3F8C),
                //cb::call(0x4F4022),
                //cb::call(0x4F4041),
                //cb::call(0x4F4096),
                //cb::call(0x4F40B8),
                //cb::call(0x4F40C2),
                //cb::call(0x4F6E25),
                //cb::call(0x4F6E8A),
                //cb::call(0x4FD295),
                //cb::call(0x4FD2C7),
                //cb::call(0x50904B),
                //cb::call(0x509145),
                //cb::call(0x50B5F7),
                //cb::call(0x50B64E),
                //cb::call(0x50B66D),
                //cb::call(0x50B736),
                //cb::call(0x511AAA),
                //cb::call(0x511AFB),
                //cb::call(0x513045),
                //cb::call(0x5133CF),
                //cb::call(0x514B26),
                //cb::call(0x5154BD),
                //cb::call(0x5154E8),
                //cb::call(0x517181),
                //cb::call(0x517CCC),
                //cb::call(0x51AB30),
                //cb::call(0x51AF60),
                //cb::call(0x51E4A7), // FX related
                //cb::call(0x51EB5C),
                //cb::call(0x51EEC1),
                //cb::call(0x52262A),
                //cb::call(0x52283C),
                cb::call(0x525F26),   // World building normals
                cb::call(0x526025),   // Some prop normals // Should be 1
                cb::call(0x526CF8),   // World road normals
                //cb::call(0x526DF5),
                //cb::call(0x527F74),
                //cb::call(0x5280B4),
                //cb::call(0x559A45),
                //cb::call(0x559EA8),
                //cb::call(0x55B646),
                //cb::call(0x55B707),
                //cb::call(0x5790B4),
                //cb::call(0x5790D8),
                //cb::call(0x5EA6DD),
                //cb::call(0x5EA892),
                //cb::call(0x5EC8BB),
                //cb::call(0x5EC986),
                //cb::call(0x5EEA2C),
                //cb::call(0x5FB7C9), // Nitro flame normals
                //cb::call(0x5FBD34),

            });

        //InstallCallback("gfxState::SetCamera()", "gfxState::SetCamera()", &gfxState::SetCamera, { cb::jmp(0x5EE8D0) });
        InstallCallback("gfxState::SetCamera()", "gfxState::SetCamera()",
            &gfxState::SetCamera, {
                //cb::call(0x458C05),
                //cb::call(0x459175),
                //cb::call(0x49FDF7),
                //cb::call(0x4A1ACE),
                //cb::call(0x4A26E1), // 3D arrow glow related
                //cb::call(0x4A27F8), // 3D arrow related
                //cb::call(0x4A582D),
                //cb::call(0x4A5967),
                //cb::call(0x4A6034),
                cb::call(0x4A6238), // Map related
                //cb::call(0x4A712A),
                cb::call(0x4A8603), // Minimap CPs related
                cb::call(0x4A8C3C), // Battle mode minimap CPs related
                cb::call(0x4A8ED8), // Opponent arrows on minimap (online) related
                //cb::call(0x4A903E), // Map arrow related, makes it disappear, it is broken anyway
                cb::call(0x4A90E9), // Opponent arrows on minimap related
                //cb::call(0x4A999C),
                //cb::call(0x4A9E12),
                //cb::call(0x5E96B2), // World geo related
                //cb::call(0x5E99CB),
                //cb::call(0x5EE913), // N letter on the minimap gets affected through Matrix34 version of SetCamera
                //cb::call(0x5F2340),
            });

        InstallCallback("gfxState::SetCamera34()", "gfxState::SetCamera34()",
            &gfxState::SetCamera34, {
                //cb::call(0x49FBCF), // Flips world geo
                cb::call(0x4A6D7E), // Flips world geo and N letter
                //cb::call(0x517C3F),
                //cb::call(0x57C36A),
            });
    }

    InstallCallback("gfxState::SetFogEnd()", "gfxState::SetFogEnd()",
        &gfxState::SetFogEnd, {
            cb::call(0x517F68),
            cb::call(0x517FB3),
            cb::call(0x5EEA4E),
        });
    InstallCallback("gfxState::SetFogStart()", "gfxState::SetFogStart()",
        &gfxState::SetFogStart, {
            cb::call(0x517F42),
            cb::call(0x517F93),
            cb::call(0x5EEA44),
        });

    // hudMap
    //InstallCallback("hudMap::sub_4A93C0()", "hudMap::sub_4A93C0()",
    //    &hudMap::sub_4A93C0, {
    //        cb::call(0x4A990B),
    //        cb::call(0x4A9D8F),
    //    });

    //InstallVTableHook("hudMap::sub_4A93C0()", &hudMap::sub_4A93C0, { 0x642C38 });

    // vehInput
    InstallVTableHook("vehInput::Update()", &vehInput::Update, { 0x63D0C4 });
    InstallVTableHook("vehInput::ApplyReplayFrame()", &vehInput::ApplyReplayFrame, { 0x63D0D4 });
    
    // ioJoystick
    InstallCallback("ioJoystick::BeginAll()", "ioJoystick::BeginAll()", &ioJoystick::BeginAll, { cb::call(0x60481A) });
    InstallCallback("ioJoystick::Poll()", "ioJoystick::Poll()", &ioJoystick::Poll, {cb::call(0x605084)});
    //InstallCallback("ioJoystick::Update()", "ioJoystick::Update()", &ioJoystick::Update, { cb::call(0x6050B4) });
    InstallCallback("ioJoystick::UpdateAll()", "ioJoystick::UpdateAll()", &ioJoystick::UpdateAll, { cb::call(0x60488C) });
    //InstallCallback("ioJoystick::End()", "ioJoystick::End()", &ioJoystick::End, { cb::call(0x6050F3) });
    InstallCallback("ioJoystick::EndAll()", "ioJoystick::EndAll()", &ioJoystick::EndAll, { cb::call(0x60483E) });

    // mcData
    InstallCallback("mcData::SetTOD()", "mcData::SetTOD()", &mcData::SetTOD, { cb::jmp(0x53A960) });
    InstallCallback("mcData::SetWeather()", "mcData::SetWeather()", &mcData::SetWeather, { cb::jmp(0x53AA00) });

    // mcConfig
    InstallCallback("mcConfig::LookupTOD()", "mcConfig::LookupTOD()", &mcConfig::LookupTOD, { cb::jmp(0x53D0A0) });
    InstallCallback("mcConfig::LookupWeather()", "mcConfig::LookupWeather()", &mcConfig::LookupWeather, { cb::jmp(0x53D050) });

    // vehGyro
    InstallVTableHook("vehGyro::Update()", &vehGyro::Update, { 0x64556C });
    //InstallCallback("vehGyro::ApplyScaledTorqueAndForce()", "vehGyro::ApplyScaledTorqueAndForce()", &vehGyro::ApplyScaledTorqueAndForce, { cb::call (0x4DCB48) });

    // mcLayerMgr
    InstallCallback("mcLayerMgr::BeginLoadLayer()", "mcLayerMgr::BeginLoadLayer()",
        &mcLayerMgr::BeginLoadLayer, {
        //cb::call(0x402A69),
        //cb::call(0x402C1C),
        //cb::call(0x402C5E),
        //cb::call(0x402C96),
        //cb::call(0x402CB8),
        //cb::call(0x402CF0),
        cb::call(0x4038B8), // Begin loading movie
        //cb::call(0x403962),
        //cb::call(0x403984),
        //cb::call(0x403A6C),
        //cb::call(0x403AB6),
        //cb::call(0x403B0E),
        //cb::call(0x403B5E),
        //cb::call(0x40420C),
        //cb::call(0x40424F),
        //cb::call(0x404292),
        //cb::call(0x4042D5),
        //cb::call(0x404318),
    });

    InstallCallback("mcLayerMgr::AfterLoadLayer()", "mcLayerMgr::AfterLoadLayer()",
        &mcLayerMgr::AfterLoadLayer, {
            cb::call(0x4038D1), // After loading movie
        });
};
