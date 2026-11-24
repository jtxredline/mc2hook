#pragma once
#include <mc2hook/mc2hook.h>
#include <veh_dyna/aero.h>
#include <age/memory/age_alloc_baseclass.h>

class mcPlayerInput;
class mcCarSim;
class vehInput;

class mcCarAero : public vehAero, public AGEAllocatedClass
{
public:
	mcPlayerInput* m_BikeInput; // vehInput* m_Input?
	float m_DragSlipstream;
	float m_Drag;
	float dword_48;

public:
	mcCarAero()  { hook::Thunk<0x4E5420>::Call<void>(this); }
	~mcCarAero() { hook::Thunk<0x4E5470>::Call<void>(this); }

	void Update();
	void Init(mcCarSim* sim, vehInput* input);
};

static_assert(sizeof(mcCarAero) == 0x4C, "mcCarAero size mismatch");
