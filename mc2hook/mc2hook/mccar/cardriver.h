#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>
#include <veh_dyna/driver.h>

class mcCarDriver : public vehDriver, public AGEAllocatedClass
{
public:
	mcCarDriver()  { hook::Thunk<0x4DEF30>::Call<void>(this); }
	~mcCarDriver() { hook::Thunk<0x4DEF50>::Call<void>(this); }
};

static_assert(sizeof(mcCarDriver) == 0xD8, "mcCarDriver size mismatch");
