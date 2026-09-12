#pragma once
#include <mc2hook/mc2hook.h>
#include <memory\age_alloc_baseclass.h>

class mcCarSim;
class vehDrivetrain;
class vehInput;
class Vector3;

class vehGyro : public AGEAllocatedClass
{
public:
	static hook::Type<int> dword_6957C0; // Not sure what this is yet

public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	mcCarSim* m_CarSim;
	vehDrivetrain* m_Drivetrain;
	int dword_14;
	int dword_18;
	float m_Turn;
	float m_Drift;
	float m_Spin180;
	float m_Reverse180;
	int dword_2C;
	float m_Pitch;
	float m_RollTorque;
	int dword_38;
	int dword_3C;
	vehInput* m_Input;
	float dword_44;
	float m_Airtime;
	int dword_4C;
	float m_RollLimit;
	float m_RollDamp;
	float m_Lean;
	float m_LeanLimit;
	float m_LeanDamp;
	float m_LeanImpulseUp;
	float m_LeanImpulseDn;
	float m_LeanSpeedMin;
	float m_Wheelie;
	float m_DriftThrust;
	float m_DriftDecay;
	float m_TurnFactor;
	float m_BurnoutTurn;
	float m_TwoWheelSteerLimit;
	float m_TwoWheelLeanAngle;
	float m_TwoWheelCOG;
	float m_TwoWheelDrag;

public:
	vehGyro()  { hook::Thunk<0x4DC010>::Call<void>(this); }
	~vehGyro() { hook::Thunk<0x4DC010>::Call<void>(this); }

	void Init(mcCarSim* sim, const char* carName);
	void Update();
	void ApplyScaledTorqueAndForce(const Vector3& torque, const Vector3& offset, float scale);
};

class vehBikeGyro : public vehGyro
{
public:
	float m_LeanBikeLimit;
	float m_LeanGasLimit;
	float m_LeanBrakeLimit;
	float m_LeanSkidLimit;

	vehBikeGyro()  { hook::Thunk<0x4DD620>::Call<void>(this); }
	~vehBikeGyro() { hook::Thunk<0x4DD6E0>::Call<void>(this); }

	void Init(mcCarSim* sim, const char* carName);
};

static_assert(sizeof(vehGyro) == 0x94, "vehGyro size mismatch");
static_assert(sizeof(vehBikeGyro) == 0xA4, "vehBikeGyro size mismatch");
