#pragma once
#include <age/vector/vector3.h>

class mcCarSim;

class vehAero
{
public:
	void* m_Vtable;
	float dword_04;
	int dword_08;
	mcCarSim* m_CarSim;
	Vector3 m_AngCDamp;
	Vector3 m_AngVelDamp;
	Vector3 m_AngVel2Damp;
	float dword_34;
	float m_Down;

public:
	void Update();
};
