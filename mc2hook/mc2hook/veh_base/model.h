#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>

class Matrix34;
class vehDriver;
class mcCarSim;

class vehModel : public AGEAllocatedClass
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
	int dword_38;
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
	vehDriver* m_Driver;
	int dword_70;
	mcCarSim* m_CarSim;
	char m_Padding[41246];

	vehModel()  { hook::Thunk<0x4CE460>::Call<void>(this); }
	~vehModel() { hook::Thunk<0x4C95D0>::Call<void>(this); }

	void sub_4C4BC0(char a2) { hook::Thunk<0x4C4BC0>::Call<void>(this, a2); }
	void Init(const char* carName, Matrix34* mtx, mcCarSim* sim, bool a5, bool a6, bool isBike);
};

static_assert(sizeof(vehModel) == 0xA198, "vehModel size mismatch");
