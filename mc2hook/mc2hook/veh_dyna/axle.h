#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>

class mcCarSim;
class vehWheel;

class vehAxle : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int dword_0c;
	int dword_10;
	int dword_14;
	int dword_18;
	int dword_1c;
	int dword_20;
	int dword_24;
	int dword_28;
	int dword_2c;
	int dword_30;
	int dword_34;
	float m_TorqueCoef;
	float m_DampCoef;
	mcCarSim* m_CarSim;
	vehWheel* m_WheelLeft;
	vehWheel* m_WheelRight;
	int dword_4c;
	int dword_50;
	float m_TorqueCoefCopy;
	float m_TorqueAndDampComputed;
	int dword_5c;
	int dword_60;
	int dword_64;
	int dword_68;
	int dword_6c;
	int dword_70;
	int dword_74;
	int dword_78;
	int dword_7c;
	int dword_80;
	int dword_84;
	int dword_88;
	int dword_8c;
	int dword_90;
	int dword_94;
	int dword_98;
	int dword_9c;
	int dword_a0;
	int dword_a4;
	int dword_a8;
	int dword_ac;
	int dword_b0;
	int dword_b4;
	int dword_b8;

public:
	vehAxle()  { hook::Thunk<0x5747B0>::Call<void>(this); }
	~vehAxle() { hook::Thunk<0x5743C0>::Call<void>(this); }
};

static_assert(sizeof(vehAxle) == 0xBC, "vehAxle size mismatch");
