#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>
#include <age/vector/matrix34.h>

class mcCarSim;
class vehWheel;

class vehSuspension : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	mcCarSim* m_CarSim;
	int dword_08;
	Matrix34 m_GlobalMatrix;
	vehWheel* m_Wheel;
	float m_SomeLocalScaleZ;
	int m_Mode;

public:
	vehSuspension()  { hook::Thunk<0x574AA0>::Call<void>(this); }
	~vehSuspension() { hook::Thunk<0x574810>::Call<void>(this); }
};

static_assert(sizeof(vehSuspension) == 0x48, "vehSuspension size mismatch");
