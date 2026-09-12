#pragma once
#include <memory\age_alloc_baseclass.h>

class vehEntity;

class vehNitro : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int m_NitrosMax;
	float m_NitroBoostExp;
	float m_NitroBoostAmount;
	float m_NitroBoostTime;
	float m_NitroFOV;
	float m_FOVOutTime;
	float m_FOVInTime;
	int dword_28;
	float dword_2c;
	int dword_30;
	int dword_34;
	float m_FlameTime;
	float m_FlameSize;
	int dword_40;

public:
	vehNitro()  { hook::Thunk<0x4D2080>::Call<void>(this); }
	~vehNitro() { hook::Thunk<0x4D1EA0>::Call<void>(this); }

public:
	void sub_4D1F80();
	bool CanActivate();
	void sub_4D1EE0();

	void Init(int a2, vehEntity* entity, const char* carName);
};
