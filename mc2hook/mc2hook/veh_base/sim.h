#pragma once
#include <vector/vector3.h>
#include <veh_dyna/wheel.h>

class vehAero;
class vehFluid;
class vehEngine;
class vehTransmission;
class vehDrivetrain;
class vehAxle;
class vehSuspension;
class phCollider;
class vehWheel;

struct vehWheels
{
	vehWheel m_Wheel_FL;
	vehWheel m_Wheel_RL;
	vehWheel m_Wheel_FR;
	vehWheel m_Wheel_RR;
};

class vehSim
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	vehAero* m_Aero;
	void* m_Fluid;
	vehEngine* m_Engine;
	vehTransmission* m_Transmission;
	int m_NumWheels;
	vehWheels* m_WheelsStruct; //vehWheel* m_WheelFL; // TODO: Unify how wheels are retrieved, f.e using m_Wheels in vehGyro::Update crashes
	int m_NumDrivetrains;
	vehDrivetrain* m_Drivetrain;
	int m_NumAxles;
	void* m_Axle;
	int m_NumSuspensions;
	void* m_Suspension;
	int dword_3c;
	int dword_40;
	int dword_44;
	Vector3 m_InertiaScale;
	Vector3 m_ModelOffset;
	Vector3 m_CenterOfMass;
	phCollider* m_Collider;
	float m_Steer;
	float m_Throttle;
	float m_Brake;
	float m_Handbrake;
	float field_80;
	float field_84;
	float field_88;
	float field_8C;
	float field_90;
	float field_94;
	float field_98;
	float field_9C;
	float field_A0;
	float field_A4;
	float field_A8;
	float field_AC;
	float m_Speed;
};

static_assert(sizeof(vehSim) == 0xB4, "vehSim size mismatch");
