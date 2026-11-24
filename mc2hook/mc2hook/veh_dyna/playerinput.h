#pragma once
#include <mc2hook/mc2hook.h>
#include <veh_base/input.h>

class mcCarSim;

class vehPlayerInput : public vehInput
{
public:
	mcCarSim* m_CarSim;
	int dword_30;
	int dword_34;
	int dword_38;

public:
	//vehPlayerInput()  { hook::Thunk<0x567F70>::Call<void>(this); }
	//~vehPlayerInput() { hook::Thunk<0x567FB0>::Call<void>(this); }

	void PlaybackReplay();
	void Update();
};

static_assert(sizeof(vehPlayerInput) == 0x3C, "vehPlayerInput size mismatch");
