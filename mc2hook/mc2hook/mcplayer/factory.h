#pragma once
#include <mc2hook\mc2hook.h>

class vehEntity;
class mcCar;

class mcPlayerFactory
{
public:
	void* m_Vtable;
	char m_CarName[64];
	mcCar* m_Car;
	int dword_48;
	int dword_4c;
	int m_PlayerId;

public:
	void Build(const char* carName, int idx, void* owner);
	vehEntity* Create();
	vehEntity* GetEntity() const;

	void MakeEntity();
	void MakeSim();
	void MakePlayerInput();
	void MaybeMakeModel()    { hook::Thunk<0x46C150>::Call<void>(this); }
	void MakeDamage1()       { hook::Thunk<0x46C280>::Call<void>(this); } // TODO: Figure out what the Damage1 and Damage2 are
	void MakeStuck()         { hook::Thunk<0x46C2D0>::Call<void>(this); }
	void MakeGyro()          { hook::Thunk<0x46C320>::Call<void>(this); }
	void MakeDriver()        { hook::Thunk<0x46C3D0>::Call<void>(this); }
	void MakeFeedback()      { hook::Thunk<0x46C4A0>::Call<void>(this); }
	void MaybeMakeCamera()   { hook::Thunk<0x46C7C0>::Call<void>(this); } // ?
	void MaybeMakeWheelPtx() { hook::Thunk<0x46C670>::Call<void>(this); }
	void MakeDamage2()       { hook::Thunk<0x46C750>::Call<void>(this); }
	void MaybeMakeAudio()    { hook::Thunk<0x46C5A0>::Call<void>(this); }
};
