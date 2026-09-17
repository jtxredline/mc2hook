#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>
#include <veh_base/input.h>

class vehEntity;
class mcCarSim;

class aiInput : public vehInput, public AGEAllocatedClass
{
public:
	vehEntity* m_Entity;
	mcCarSim* m_CarSim;
	int dword_34;
	int m_InputTarget;
	int dword_3c;
	int dword_40;
	int dword_44;
	int dword_48;
	int dword_4c;
	int dword_50;
	int dword_54;
	int dword_58;
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
	int dword_bc;
	int dword_c0;
	int dword_c4;
	int dword_c8;
	int dword_cc;
	int dword_d0;
	int dword_d4;
	int dword_d8;
	int dword_dc;
	int dword_e0;
	int dword_e4;
	int dword_e8;
	int dword_ec;
	int dword_f0;
	int dword_f4;
	int dword_f8;
	int dword_fc;
	int dword_100;
	float dword_104;
	int dword_108;
	float dword_10c;
	int dword_110;
	int dword_114;
	float dword_118;

public:
	aiInput(vehEntity* entity)  { hook::Thunk<0x4BC1F0>::Call<void>(this, entity); }
	~aiInput() { hook::Thunk<0x575370>::Call<void>(this); } // ?
};

static_assert(sizeof(aiInput) == 0x11C, "aiInput size mismatch");
