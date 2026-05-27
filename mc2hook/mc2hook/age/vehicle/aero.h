#pragma once
#include <age/vector/vector3.h>

class vehCarSim;

class vehAero {
public:
	void* m_Vtable;
	float dword_04;
	int dword_08;
	vehCarSim* m_CarSim;
	Vector3 m_AngCDamp;
	Vector3 m_AngVelDamp;
	Vector3 m_AngVel2Damp;
	float dword_34;
	float m_Down;
	int dword_3c;
	float m_DragSlipstream;
	float m_Drag;
	float dword_48;

public:
	void Update();
	void Update2(); // Temp name
};
