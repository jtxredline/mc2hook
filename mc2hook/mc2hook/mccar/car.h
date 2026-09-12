#pragma once
#include <mc2hook\mc2hook.h>

// Forward declarations
class mcCarSim;
class vehInput;
class vehModel;
class vehAudio;
class vehStuck;
class vehGyro;
class mcCarDamage;
class mcCarPickups;

class mcCar
{
public:
	void* m_Vtable;
	vehInput* m_Input;
	mcCarSim* m_CarSim;
	vehModel* m_Model;
	vehAudio* m_Audio;
	mcCarDamage* m_Damage;
	void* m_Feedback;
	void* m_Driver;
	vehStuck* m_Stuck;
	vehGyro* m_Gyro;
	int dword_28;
	int dword_2c;
	void* m_WheelPtx;
	int dword_34;
	mcCarPickups* m_Pickups; // Maybe
	int dword_3c;

public:
	void Update();
};
