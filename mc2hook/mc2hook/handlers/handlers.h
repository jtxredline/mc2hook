#pragma once
#include <handlers\InitHandler.h>
#include <handlers\zipIintHandler.h>
#include <handlers\CustomExceptionHandler.h>
#include <handlers\PrinterHandler.h>
#include <handlers\FontCrashFixHandler.h>
#include <handlers\RemoveForcedArgsHandler.h>
#include <handlers\OpenSpyHandler.h>
#include <handlers\ChatHandler.h>

#include <handlers\CustomVehicleHandler.h>
#include <handlers\SteeringSmootherHandler.h>
#include <handlers\PathHandler.h>
#include <handlers\zipEnumHandler.h>
#include <handlers\TestPanelHandler.h>
#include <handlers\InputHandler.h>
#include <handlers\BorderlessHandler.h>
#include <handlers\ReflectionFidelityHandler.h>
#include <handlers/OnlineBangersHandler.h>
#include <handlers/FreecamHandler.h>
#include <handlers/MaxVelocityHandler.h>
#include <handlers/AddressPrinterHandler.h>
#include <handlers/ControllerScrollingFixHandler.h>
#include <handlers/RDTSCFixHandler.h>
#include <handlers/CityModelLimitHandler.h>
#include <handlers/LevelExtentsHandler.h>
#include <handlers/CarShadowHandler.h>
#include <handlers\FPSCounterHandler.h>
#include <handlers/TimeWarpHandler.h>
#include <handlers/DeadZoneHandler.h>
#include <handlers/BurnoutHandler.h>
#include <handlers/GravityHandler.h>
#include <handlers/CPUPlayerVehiclesHandler.h>
#include <handlers/REHandler.h>
#include <handlers\KinematicSteeringHandler.h>
#include <handlers\TrafficDistanceHandler.h>
#include <handlers/RenderDistanceHandler.h>
#include <handlers\StateResearchHook.h>

// Installed before engine init
static void InstallHandlersPreEngineInit()
{
    InstallHandler<REHandler>("RE Handler");
    InstallHandler<InputHandler>("Input Handler");
    InstallHandler<BorderlessHandler>("Borderless Handler");
}

// Installed after engine init
static void InstallHandlersPostEngineInit()
{ 
    InstallHandler<CustomVehicleHandler>("Custom Vehicle Handler");
    InstallHandler<SteeringSmootherHandler>("Smooth Steering");
    InstallHandler<BurnoutHandler>("Burnout Handler");
    InstallHandler<PathHandler>("Path Handler");
    InstallHandler<MaxVelocityHandler>("Max Velocity Handler");
    InstallHandler<AddressPrinterHandler>("Address Printer Handler");
    InstallHandler<KinematicSteeringHandler>("Kinematic Steering Handler");
    InstallHandler<TrafficDistanceHandler>("Traffic Distance Handler");

    //InstallHandler<TestPanelHandler>("Panel Handler (Keyboard input test really)");
    //InstallHandler<OnlineBangersHandler>("Online Bangers Handler");
    //InstallHandler<VelocityPrinterHandler>("Velocity Printer Handler");
}

// Installed at game launch
static void InstallMainHandlers()
{
    InstallHandler<CustomExceptionHandler>("Custom Exception Handler");
    InstallHandler<PrinterHandler>("Printer Handler");
    InstallHandler<zipInitHandler>("ZIP Handler");
    InstallHandler<zipEnumHandler>("ZIP Enum Handler");
    InstallHandler<RemoveForcedArgsHandler>("Remove Forced Args Handler");
    InstallHandler<FontCrashFixHandler>("Font Crash Fix");
    InstallHandler<OpenSpyHandler>("OpenSpy Handler");
    InstallHandler<ChatHandler>("Chat Handler");
    InstallHandler<InitHandler>("Game Init Handler");
    InstallHandler<ReflectionFidelityHandler>("Reflection Fidelity Handller");
    InstallHandler<FreeCamHandler>("Free Cam Handler");
    InstallHandler<ControllerScrollingFixHandler>("Controller Scrolling Fix");
    InstallHandler<RDTSCFixHandler>("RDTSC Fix Handler");
    InstallHandler<CityModelLimitHandler>("City Model Limit Handler"); // Breaks headlights
    InstallHandler<LevelExtentsHandler>("Level Extents Handler");
    InstallHandler<CarShadowHandler>("Car Shadow Handler");
    InstallHandler<FPSCounterHandler>("FPS Counter Handler");
    InstallHandler<TimeWarpHandler>("Time Warp Handler");
    InstallHandler<DeadZoneHandler>("Dead Zone Handler");
    InstallHandler<GravityHandler>("Gravity Handler");
    InstallHandler<CPUPlayerVehiclesHandler>("CPU Player Vehicles Handler");
    InstallHandler<RenderDistanceHandler>("RenderDistance Handler");
    //InstallHandler<REHandler>("RE Handler");
    
    //InstallHandler<StateResearchHook>("SRH");
}

/*#pragma once
#include <handlers\InitHandler.h>
#include <handlers\OpenSpyHandler.h>
#include <handlers/MiniFPSCounterHandler.h>

// Installed before engine init
static void InstallHandlersPreEngineInit()
{
}

// Installed after engine init
static void InstallHandlersPostEngineInit()
{
}

// Installed at game launch
static void InstallMainHandlers()
{
    InstallHandler<OpenSpyHandler>("OpenSpy Handler");
    InstallHandler<MiniFPSCounterHandler>("Mini FPS Counter Handler");
}*/