#pragma once
#include <age/vector/vector3.h>

class vehCarSim;
class vehInput;

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
	vehInput* m_BikeInput;
	float m_DragSlipstream;
	float m_Drag;
	float dword_48;

public:
	void Update();
	void Update2(); // Temp name
	void sub_4E5450(vehCarSim* sim, vehInput* input); // Assign bike input
};
