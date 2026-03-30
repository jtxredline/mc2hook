#pragma once
#include <mc2hook\mc2hook.h>

class vehCarSim;
class vehModel;
class vehAudio;
class vehDamage;

class vehEntity {
public:
	char pad_0000[68]; //0x0000
	void* m_Input; //0x0044
	vehCarSim* m_CarSim; //0x0048
	vehModel* m_Model; //0x004C
	vehAudio* m_Audio; //0x0050
	void* m_Damage2; //0x0054
	void* m_Feedback; //0x0058
	void* m_MaybePhysBehavior; //0x005C
	void* m_Stuck; //0x0060
	void* m_Gyro; //0x0064
	char pad_0068[8]; //0x0068
	void* m_WheelPtx; //0x0070
	int dword_74;
	vehDamage* m_Damage;
};
