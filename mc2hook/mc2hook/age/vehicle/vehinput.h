#pragma once
#include <age/vehicle/carsim.h>

class vehCarSim;

class vehInput {
public:
    void* m_Vtable;
    int dword_04;
    int dword_08;
    int dword_0c;                      // Contains network player ID at offset 0x0E (short)
    float m_GasBrake;                  // Gas/brake combined axis (-1 to 1)
    int dword_14;                      // Brake value
    int dword_18;                      // Handbrake (lerped value)
    float m_LastRequestedSteer;        // Steering input from player (-1 to 1)
    int dword_20;                      // Camera controls (likely float for up/down)
    int dword_24;                      // Camera controls (likely float for left/right)
    int m_CurrentGear;                 // Bitfield containing:
                                       //   Bits 0-15: Current gear number
                                       //   Bit 16 (0x10000): Camera mode flag
                                       //   Bit 17 (0x20000): Headlights on
                                       //   Bit 18 (0x40000): Nitro button pressed
                                       //   Bit 19 (0x80000): Special turbo
                                       //   Bit 20 (0x100000): Camera shake
                                       //   Bit 21 (0x200000): ???
                                       //   Bit 22 (0x400000): Horn pressed
                                       //   Bit 23 (0x800000): Analog input mode
                                       //   Bit 24 (0x1000000): Upshift request
                                       //   Bit 25 (0x2000000): Downshift request
                                       //   Bit 31 (0x80000000): Counter-steer detect
    vehCarSim* m_VehCarSim;            // Pointer to the vehicle car simulation
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3c;
    int dword_40;                      // Camera view index (0-3)
    int dword_44;                      // Some gear-related value
    int dword_48;
    int dword_4c;
    int dword_50;                      // Pointer to another structure (vehicle instance?)
    int dword_54;                      // Input flags/state
    int m_SteerCopy;                   // Copy of steering value for change detection (likely float)
    int dword_5c;
    int dword_60;
    int dword_64;
    int dword_68;
    float m_AnalogSteerSpeed;          // Analog steering smoothing speed (keyboard)
    int dword_70;                      // Steer lerp speed (likely float for high speed)
    int dword_74;                      // Steer lerp speed (likely float for low speed)
    int dword_78;
    int dword_7c;
    int dword_80;                      // Input device/controller pointer
    int dword_84;
    int dword_88;
    int dword_8c;

public:
    // Virtual function at vtable offset 0
    virtual void Update();              // The main update function (0x46B330)
    
};
