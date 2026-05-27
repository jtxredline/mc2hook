#pragma once
#include <mc2hook/mc2hook.h>

class vehCarSim;
class vehDrivetrain;
class vehInput;
class Vector3;

class vehGyro {
public:
	static hook::Type<int> dword_6957C0; // Not sure what this is yet

public:
	void* vtable;
	int dword_04;
	int dword_08;
	vehCarSim* m_CarSim;
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
	void Update();
	void ApplyScaledTorqueAndForce(const Vector3& torque, const Vector3& offset, float scale);
};
