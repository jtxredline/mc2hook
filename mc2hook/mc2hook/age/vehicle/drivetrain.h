#pragma once
#include <age/vehicle/carsim.h>

class vehCarSim;
class vehWheel;

class vehDrivetrain
{
public:
	void* m_Vtable;
	vehCarSim* m_CarSim;
	void* m_Engine;
	void* m_Transmission;
	int m_NumWheels;
	vehWheel* m_WheelBL;
	vehWheel* m_WheelBR;
	vehWheel* m_WheelCL;
	vehWheel* m_WheelCR;
	vehWheel* m_WheelFL;
	vehWheel* m_WheelFR;
	float m_SomeRPS;
	float dword_30;
	float m_AngInertia;
	float m_BrakeDynamicCoef;
	float m_BrakeStaticCoef;
	int dword_40;
	int dword_44;
	int dword_48;
	int dword_4c;
	int dword_50;
	void* m_WheelFront;
	void* m_WheelBack;
	int dword_5c;
	int dword_60;
	int dword_64;
	int dword_68;
	int dword_6c;
	int dword_70;
	int dword_74;
	int dword_78;
	int dword_7c;
	int dword_80;

public:
	void Detach();
	void Attach();
};
