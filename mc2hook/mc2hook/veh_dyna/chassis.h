#pragma once
#include <veh_base/sim.h>

class vehWheel;
class vehEntity;

class vehChassis : public vehSim
{
public:
	int dword_B4;
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
	int dword_10C;
	int dword_110;
	int dword_114;
	int dword_118;
	int dword_11C;
	int dword_120;
	int dword_124;
	int dword_128;
	int dword_12C;
	int dword_130;
	int dword_134;
	int dword_138;
	int dword_13C;
	int dword_140;

public:
	int OnGround(); // Number of wheels on ground
	int BottomedOut();
	void MakeCollider(const char* carName, vehEntity* entity);

	void MakeAero(const char* carName);
	void MakeFluid(const char* carName);
	void MakeTransmission(const char* carName);
	void MakeEngine(const char* carName);
	void MakeWheels(const char* carName);
	void MakeDrivetrains(const char* carName);
	void MakeAxles(const char* carName);
	void MakeSuspensions(const char* carName);
};

static_assert(sizeof(vehChassis) == 0x144, "vehChassis size mismatch");
