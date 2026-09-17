#pragma once
#include <mc2hook/mc2hook.h>
#include <veh_dyna/fluid.h>
#include <age/memory/age_alloc_baseclass.h>

class mcCarFluid : public vehFluid, public AGEAllocatedClass
{
public:
	float m_Buoyancy;
	float m_MinBuoyancy;
	float m_SinkRate;
	int dword_334;

public:
	mcCarFluid()  { hook::Thunk<0x4E55A0>::Call<void>(this); }
	~mcCarFluid() { hook::Thunk<0x4E55F0>::Call<void>(this); }
};

static_assert(sizeof(mcCarFluid) == 0x338, "mcCarFluid size mismatch");
