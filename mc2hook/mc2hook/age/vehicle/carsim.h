#pragma once
#include <age/vehicle/wheel.h>

class vehAero;
class vehTransmission;
class vehEngine;
class vehDrivetrain;
class phCollider;
class vehNitro;
class mcCarSSTurbo;
class vehDamage;
class vehWheel;

struct vehWheels
{
	vehWheel m_Wheel_FL;
	vehWheel m_Wheel_RL;
	vehWheel m_Wheel_FR;
	vehWheel m_Wheel_RR;
};

class vehCarSim {
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	vehAero* m_Aero;
	void* m_Fluid;
	vehEngine* m_Engine;
	vehTransmission* m_Transmission;
	int m_NumWheels;
	vehWheels* m_WheelsStruct; //vehWheel* m_WheelFront; // void*
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
	float field_B4;
	float m_Mass;
	Vector3 m_Size;
	Vector3 m_InertiaBox;
	float m_BoundFriction;
	float m_BoundElasticity;
	float m_BoundGravity;
	float m_AirGravity;
	int m_DrivetrainType;
	int* m_Freetrain;
	vehWheel* m_Wheels[4];
	//vehWheel* m_WheelFL;
	//vehWheel* m_WheelRL;
	//vehWheel* m_WheelFR;
	//vehWheel* m_WheelRR;
	int dword_fc;
	int dword_100;
	int dword_104;
	int dword_108;
	int dword_10c;
	int dword_110;
	int dword_114;
	int dword_118;
	int dword_11c;
	int dword_120;
	int dword_124;
	int dword_128;
	int dword_12c;
	int dword_130;
	int dword_134;
	int dword_138;
	int dword_13c;
	int dword_140;
	int dword_144;
	int dword_148;
	int dword_14c;
	int dword_150;
	float dword_154;
	float dword_158;
	float dword_15c;
	float dword_160;
	int dword_164;
	float dword_168;
	int dword_16c;
	int dword_170;
	float dword_174;
	float dword_178;
	int dword_17c;
	int dword_180;
	char field_184;
	bool m_BurnoutCharged;
	char field_186;
	char field_187;
	float dword_188;
	float m_BurnoutValue;
	float m_BurnoutIncreaseSpeed;
	float m_BurnoutDecreaseSpeed;
	float m_BurnoutDamageAmount;
	float m_BurnoutBoostSpeed;
	int m_BurnoutCharging;
	float dword_1a4;
	float dword_1a8;
	float m_SteeringLimit;
	float m_Airtime;
	float m_SomeBrake;
	int dword_1b8;
	int dword_1bc;
	int dword_1c0;
	int dword_1c4;
	int dword_1c8;
	int dword_1cc;
	vehNitro* m_Nitro;
	mcCarSSTurbo* m_SSTurbo;
	vehDamage* m_Damage;

public:
	void UpdateControls();
	void UpdateControlsComp();

	int OnGround(); // Number of wheels on ground
	int BottomedOut();
	void SetDrivable(int a2);
	void SetCenterOfMass(const Vector3& cg); // Set the simulation center of mass offset from instance origin
	void SetFrictionHandling(float friction);

	float sub_4D2860(float a2);
};

