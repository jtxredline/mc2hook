#pragma once
#include <mc2hook\mc2hook.h>

// Forward declarations
class vehCarSim;
class vehInput;
class vehModel;
class vehAudio;
class vehStuck;
class vehGyro;
class vehDamage;

class mcCar {
public:
	void* m_Vtable;
	vehInput* m_Input;
	vehCarSim* m_CarSim;
	vehModel* m_Model;
	vehAudio* m_Audio;
	void* m_Damage2;
	void* m_Feedback;
	void* m_MaybePhysBehavior;
	vehStuck* m_Stuck;
	vehGyro* m_Gyro;
	int dword_68;
	int dword_6c;
	void* m_WheelPtx;
	int dword_74;
	vehDamage* m_Damage;

public:
	void Update();
};
