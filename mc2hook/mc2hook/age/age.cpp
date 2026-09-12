#include "age.h"
#include "dllmain.h"
#include <gfx/state.h>
#include <gfx/font.h>
#include <gfx/pipeline.h>
#include <data/timemgr.h>

declfield(Timer::s_HostTimer)(0x85835C);
declfield(Timer::s_HostTime)(0x858338);
declfield(Timer::s_TickToMilliseconds)(0x8600E0);
declfield(Timer::g_FrameTimeMode)(0x858330);
declfield(Timer::s_FrameTime)(0x858348);
declfield(Timer::s_CPUUpdateTime)(0x85834C);
declfield(Timer::flt_858344)(0x858344);
declfield(Timer::flt_85833C)(0x85833C);
declfield(Timer::s_LastFrameTick)(0x858354);

uint32_t Timer::QuickTicks()
{
    //return __rdtsc();
    
    // [FIX] Replaces RDTSC high frequency timer with QPC
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.LowPart; // +(rand() % 16384);
}

void ageEndFrame()
{
    int cpuTicks = Timer::Ticks() - Timer::s_HostTimer;
    Timer::s_HostTime = cpuTicks * Timer::s_TickToMilliseconds;
    gfxState::SetBlendSet(0, 128);

    // Speedrun.com mode
    if (datTimeManager::SpeedrunMode)
    {
        char buffer[128];
        float fps = 1000.0f / Timer::s_FrameTime; // 1010.0f?

        if (datTimeManager::EnableFPSFixes)
            sprintf(buffer, "%.0f %.0f %s", fps, datTimeManager::PhysicsBaselineFPS, GetMC2HookVersion());
        else
            sprintf(buffer, "%.0f %s", fps, GetMC2HookVersion());

        gfxFont::DrawFont(gfxPipeline::m_iWidth - (8 * strlen(buffer) + 40), 24, buffer, -1);
    }

    // Basic FPS counter
    static bool enableFPSCounter = HookConfig::GetBool("General", "EnableFPSCounter", false);
    if (enableFPSCounter) Timer::g_FrameTimeMode = 2; // Reimplements -miniframetime
    
    if (Timer::g_FrameTimeMode == 2 && !datTimeManager::SpeedrunMode)
    {
        char fpsString[64];
        float fps = 1000.0f / Timer::s_FrameTime; // 1010.0f?
        sprintf(fpsString, "%.0f", fps);
        gfxFont::DrawFont(gfxPipeline::m_iWidth - (8 * strlen(fpsString) + 40), 24, fpsString, -1);
    }

    // -frametime
    if (Timer::g_FrameTimeMode == 1 && !enableFPSCounter && !datTimeManager::SpeedrunMode)
    {
        float fps;
        if (Timer::s_FrameTime.get() == 0.0f) fps = 0.0f;
        else fps = 1010.0f / Timer::s_FrameTime;

        char frametimeString[64];
        sprintf(
            frametimeString,
            "CPU UP=%5.2f DRAW=%5.2f[%5.2f/%5.2f] FPS=%.0f",
            Timer::s_CPUUpdateTime.get(),
            Timer::s_HostTime.get(),
            Timer::flt_858344.get(),
            Timer::flt_85833C.get(),
            fps);

        gfxFont::DrawFont(gfxPipeline::m_iWidth - (8 * strlen(frametimeString) + 40), 24, frametimeString, -1);
    }

    Timer timer;
    gfxPipeline::EndFrame();
    uint32_t currentTicks = Timer::Ticks();
    Timer::flt_85833C = (currentTicks - timer.TickCount) * Timer::s_TickToMilliseconds;
    Timer::s_FrameTime = (currentTicks - Timer::s_LastFrameTick) * Timer::s_TickToMilliseconds;
    Timer::s_LastFrameTick = currentTicks;
}
