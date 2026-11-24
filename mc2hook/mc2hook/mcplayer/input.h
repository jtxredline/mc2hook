#pragma once
#include <age/input/input.h>
#include <veh_dyna/playerinput.h>
#include <age/memory/age_alloc_baseclass.h>

class vehEntity;
class mcCarSim;
class rmcCarModel;
class mcCarAudio;
class mcCarDamage;
class ioDeviceWrapper;

class mcPlayerInput : public vehPlayerInput, public AGEAllocatedClass
{
public:
    int dword_3c;
    int dword_40;
    int dword_44;
    int m_TransmissionMode;
    int dword_4c;
    vehEntity* m_Entity;
    int m_Drivable;
    float m_InAirSteer;
    int dword_5c;
    int dword_60;
    int dword_64;
    int dword_68;
    float m_AnalogSteerSpeed;
    float dword_70;                    // Steer lerp speed (likely float for high speed)
    float dword_74;                    // Steer lerp speed (likely float for low speed)
    int dword_78;
    int dword_7c;
    ioDeviceWrapper* m_Device;
    uint8_t net_84[4];
    uint8_t net_88[4];
    uint8_t net_8C[4];

public:
    mcPlayerInput(int a2, int a3, int a4)  { hook::Thunk<0x46B1D0>::Call<void>(this, a2, a3, a4); }
    ~mcPlayerInput()                       { hook::Thunk<0x46B2F0>::Call<void>(this); }

public:
    void Init(vehEntity* entity, const char* carName);
    void Update();

    void UpdateFFB();
    void UpdateNetworkInput();
    void PlaybackReplay();

    float sub_46AA90(float speed);
    void sub_46A760(int a2, int a3);
    void sub_46A7A0(float steer, float* gasbrake, float* brake, int* drivable);

    void SetIndex(int playerId);
    void SetStyle(int playerId);

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

static_assert(sizeof(mcPlayerInput) == 0x90, "mcPlayerInput size mismatch");
