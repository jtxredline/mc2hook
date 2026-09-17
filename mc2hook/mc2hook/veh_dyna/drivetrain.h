#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>

static constexpr float diffRatioMax = 1.25f;
static constexpr float diffRatioMaxHighSpeed = 1.03f;
static constexpr float diffRatioHighSpeedLevel = 50.0f;
static constexpr float dRHSLinv = 0.02f;

class vehChassis;
class mcCarSim;
class vehEngine;
class vehTransmission;
class vehWheel;

class vehDrivetrain : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	mcCarSim* m_CarSim;
	vehEngine* m_Engine;
	vehTransmission* m_Transmission;
	int m_NumWheels;
	vehWheel* m_Wheels[6];
	/*
	vehWheel* m_WheelRL;
	vehWheel* m_WheelRR;
	vehWheel* m_WheelCL;
	vehWheel* m_WheelCR;
	vehWheel* m_WheelFL;
	vehWheel* m_WheelFR;
	*/
	float m_SomeRPS;
	float dword_30;
	float m_AngInertia;
	float m_BrakeDynamicCoef;
	float m_BrakeStaticCoef;
	int dword_40; // Freetrain?
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
	vehDrivetrain()  { hook::Thunk<0x574380>::Call<void>(this); }
	~vehDrivetrain() { hook::Thunk<0x573C20>::Call<void>(this); }

	void Init(vehChassis* chassis, vehTransmission* transmission, vehEngine* engine);
	void Detach();
	void Attach();
	void Update();
};
