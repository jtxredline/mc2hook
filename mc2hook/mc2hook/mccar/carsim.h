#pragma once
#include <mc2hook/mc2hook.h>
#include <memory/age_alloc_baseclass.h>
#include <veh_dyna/chassis.h>

class carAIInfo;
class vehNitro;
class mcCarSSTurbo;
class mcCarDamage;
class vehEntity;

class mcCarSim : public vehChassis//, public AGEAllocatedClass
{
public:
	int dword_144;
	int dword_148;
	int dword_14C;
	int dword_150;
	float dword_154;
	float m_HillHorsepowerFactor;
	float m_CarFrictionHandlingWipeout;
	float dword_160;
	float m_LandingDampFactor;
	float m_AeroDampFactor;
	float m_MediumLod;
	float m_LowLod;
	float m_VeryLowLod;
	float m_SSSValue;
	int m_SSSThreshold;
	int dword_180;
	char field_184;
	bool m_BurnoutCharged;
	char field_186;
	char field_187;
	float m_BurnoutThresholdSpeed;
	float m_BurnoutValue;
	float m_BurnoutIncreaseSpeed;
	float m_BurnoutDecreaseSpeed;
	float m_BurnoutDamageAmount;
	float m_BurnoutBoostSpeed;
	int m_BurnoutCharging;
	float dword_1a4;
	float m_CenterOfMassY;
	float m_SteeringLimit;
	float m_Airtime;
	float m_SomeBrake;
	int dword_1b8;
	int dword_1bc;
	int dword_1c0;
	int dword_1c4;
	int dword_1c8;
	carAIInfo* m_AIInfo;
	vehNitro* m_Nitro;
	mcCarSSTurbo* m_SSTurbo;
	mcCarDamage* m_Damage;

public:
	mcCarSim()  { hook::Thunk<0x4D22E0>::Call<void>(this); }
	~mcCarSim() { hook::Thunk<0x4D23F0>::Call<void>(this); }

	void UpdateControls();
	void UpdateControlsComp();

	void SetTransDirection(int a2);
	void SetCenterOfMass(const Vector3& cg); // Set the simulation center of mass offset from instance origin
	void SetFrictionHandling(float friction);

	void sub_4D2F60(); // ComputeConstants?
	float sub_4D2860(float a2);
	void sub_569A80(const char* carName); // Some Load
	void sub_575060(void* a2);

	void MakeCollider(const char* carName, vehEntity* entity);
	void MakeColliderChassis(const char* carName, vehEntity* entity);

	void MakeAero(const char* carName)                        { hook::Thunk<0x4D2490>::Call<void>(this, carName); }
	void MakeFluid(const char* carName)                       { hook::Thunk<0x4D24E0>::Call<void>(this, carName); }
	void MakeTransmission(const char* carName)                { hook::Thunk<0x569370>::Call<void>(this, carName); }
	void MakeEngine(const char* carName)                      { hook::Thunk<0x569320>::Call<void>(this, carName); }
	void MakeWheels(const char* carName)                      { hook::Thunk<0x56AAB0>::Call<void>(this, carName); }
	void MakeDrivetrains(const char* carName)                 { hook::Thunk<0x5693C0>::Call<void>(this, carName); }
	void MakeAxles(const char* carName)                       { hook::Thunk<0x5694B0>::Call<void>(this, carName); }
	void MakeSuspensions(const char* carName)                 { hook::Thunk<0x5695E0>::Call<void>(this, carName); }
};

static_assert(sizeof(mcCarSim) == 0x1DC, "mcCarSim size mismatch");
