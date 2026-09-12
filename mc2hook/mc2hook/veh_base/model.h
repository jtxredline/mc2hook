#pragma once
#include <mc2hook/mc2hook.h>
#include <memory/age_alloc_baseclass.h>

class Matrix34;
class mcCarSim;

class vehModel : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int dword_0c;
	char m_Padding[41350];

	vehModel()  { hook::Thunk<0x4CE460>::Call<void>(this); }
	~vehModel() { hook::Thunk<0x4C95D0>::Call<void>(this); }

	void sub_4C4BC0(char a2) { hook::Thunk<0x4C4BC0>::Call<void>(this, a2); }
	void Init(const char* carName, Matrix34* mtx, mcCarSim* sim, bool a5, bool a6, bool isBike);
};

static_assert(sizeof(vehModel) == 0xA198, "vehModel size mismatch");
