#pragma once
#include <mc2hook/mc2hook.h>

class vehInput
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int m_FlagsPlayerID;
	float m_GasBrake;
	float m_Brake;
	float m_Handbrake;
	float m_Steer;
	float m_SteerRate;
	float m_2WheelSteer;
	int m_CurrentGearFlags;

public:
	//vehInput()  { hook::Thunk<0x575300>::Call<void>(this); }
	//~vehInput() { hook::Thunk<0x575340>::Call<void>(this); }
};

static_assert(sizeof(vehInput) == 0x2C, "vehInput size mismatch");
