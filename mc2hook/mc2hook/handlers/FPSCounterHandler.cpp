#include "FPSCounterHandler.h"
#include <age/gfx/font.h>
#include <age/gfx/pipeline.h>

//#include <age/data/timemgr.h>
//#include <age/state/gamestate.h> //
//#include <age/managers/layermgr.h>
//#include <age/types.h>
//#include <age/input/keyboard.h>
//#include <dinput.h>

declfield(FPSCounterHandler::s_FrameTime) = 0x858348;

char fpsString[64];
float fps;

//char lrtString[64];
//float lrt; // In seconds
//int lrtHours;
//int lrtMinutes;
//int lrtSeconds;
//int lrtMilliseconds;

void FPSCounterHandler::DrawFontHook(int x, int y, void* a3, int a4)
{
    fps = 1000.0f / s_FrameTime; // Originally in the code it divides 1010.0f?
    sprintf(fpsString, "%.0f", fps);
    gfxFont::DrawFont(gfxPipeline::m_iWidth - (8 * strlen(fpsString) + 40), y, fpsString, a4);

    //// LRT
    //static bool isPaused = false;
    //int currentState = mcGameState::Instance->m_CurrentState;

    ////lrt += datTimeManager::ElapsedTime; // TODO: Test counting time during movies

    ////if (currentState == GameState::Game || currentState == GameState::Movie)
    ////if (isPaused == false)
    ////{
    ////    if (mcLayerMgr::Instance->m_LoadingMovie == true)
    ////    {
    ////        lrt += datTimeManager::Seconds;
    ////    }
    ////    else
    ////    {
    ////        lrt += datTimeManager::UnwarpedSeconds; // UnwarpedSeconds does not count during loading
    ////    }
    ////}

    ////TODO: Add remappable keys
    //// Reset timer on button press
    //if (ioKeyboard::GetKeyDown(DIK_1)) lrt = 0.0f;
    //// Toggle timer pause
    //if (ioKeyboard::GetKeyDown(DIK_TAB)) isPaused = !isPaused;

    //// Format string
    //lrtHours = (int)(lrt / 3600.0f);
    //lrtMinutes = (int)(lrt / 60.0f) % 60;
    //lrtSeconds = (int)lrt % 60;
    //lrtMilliseconds = (int)(lrt * 100.0f) % 100;
    //sprintf(lrtString, "%01d:%02d:%02d.%02d", lrtHours, lrtMinutes, lrtSeconds, lrtMilliseconds); // 0:00:00:00 format

    //gfxFont::DrawFont(gfxPipeline::m_iWidth - (8 * strlen(lrtString) + 40), y + 20, lrtString, a4);
}

void FPSCounterHandler::Install()
{
    bool enableFPSCounter = HookConfig::GetBool("Graphics", "EnableFPSCounter", false);

    if (enableFPSCounter)
    {
        mem::write(0x858330, static_cast<int>(1)); // Forces -frametime on
        InstallCallback("FPS Counter", "FPS Counter",
            &DrawFontHook, {
                cb::call(0x5ED1A1)
            });
    }
}
