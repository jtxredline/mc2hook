#pragma once
#include <mc2hook/mc2hook.h>
#include <mccar/car.h>
#include <physics/inst.h>
#include <memory/age_alloc_baseclass.h>

class vehEntity : public AGEAllocatedClass
{
public:
	phInst m_PhysInst;
	mcCar m_Car;

public:
	vehEntity()  { hook::Thunk<0x4D1580>::Call<void>(this); }
	~vehEntity() { hook::Thunk<0x4D1DA0>::Call<void>(this); }

	void Delete(bool a2);
	void SetPhysFlag(short flag, bool a2);
};

static_assert(sizeof(vehEntity) == 0x80, "vehEntity size mismatch");
