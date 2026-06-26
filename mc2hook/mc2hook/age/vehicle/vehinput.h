#pragma once
#include <age/input/input.h>

class vehEntity;
class vehCarSim;
class vehModel;
class vehAudio;
class vehDamage;
class ioDeviceWrapper;

class vehInput {
public:
    void* m_Vtable;
    int dword_04;
    int dword_08;

    uint16_t m_Flags;
    uint16_t m_PlayerID;

    float m_GasBrake;
    float m_Brake;
    float m_Handbrake;
    float m_Steer;
    float m_SteerRate;                 // ?
    float m_2WheelSteer;
    int m_CurrentGearFlags;            // Bitfield containing:
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
    vehCarSim* m_CarSim;               // Pointer to the vehicle car simulation
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3c;
    int dword_40;                      // Camera view index (0-3)
    int dword_44;                      // Some gear-related value
    int dword_48;
    int dword_4c;
    vehEntity* m_Entity;
    int m_Drivable;                 // int8_t?
    //char pad_55[3];                    //
    float m_InAirSteer;
    int dword_5c;
    int dword_60;
    int dword_64;
    int dword_68;
    float m_AnalogSteerSpeed;          // Analog steering smoothing speed (keyboard)
    float dword_70;                    // Steer lerp speed (likely float for high speed)
    float dword_74;                    // Steer lerp speed (likely float for low speed)
    int dword_78;
    int dword_7c;
    ioDeviceWrapper* m_Device;         // Input device/controller pointer
    uint8_t net_84[4];
    uint8_t net_88[4];
    uint8_t net_8C[4];

public:
    void Update();
    void UpdateFFB();
    void UpdateReplay();
    void UpdateNetworkInput();
    void ApplyReplayFrame();
    float sub_46AA90(float speed);
    void sub_46A760(int a2, int a3);
    void sub_46A7A0(float steer, float* gasbrake, float* brake, int* drivable);
    void SomethingReplay();

    static float PackSignedFloat(float value, int8_t* out)
    {
        int8_t packed = (int8_t)(value * 127.0f);
        *out = packed;

        return packed / 127.0f;
    }

    static float PackUnsignedFloat(float value, uint8_t* out)
    {
        uint8_t packed = (uint8_t)(value * 255.0f);
        *out = packed;

        return packed / 255.0f;
    }
};
