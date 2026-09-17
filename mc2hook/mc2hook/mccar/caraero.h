#pragma once
#include <mc2hook/mc2hook.h>
#include <veh_dyna/aero.h>
#include <age/memory/age_alloc_baseclass.h>

class vehPlayerInput;
class mcCarSim;
class vehPlayerInput;

class mcCarAero : public vehAero, public AGEAllocatedClass
{
public:
	vehPlayerInput* m_BikeInput;
	float m_DragSlipstream;
	float m_Drag;
	float dword_48;

public:
	mcCarAero()  { hook::Thunk<0x4E5420>::Call<void>(this); }
	~mcCarAero() { hook::Thunk<0x4E5470>::Call<void>(this); }

	void Update();
	void Init(mcCarSim* sim, vehPlayerInput* input);
};

static_assert(sizeof(mcCarAero) == 0x4C, "mcCarAero size mismatch");
