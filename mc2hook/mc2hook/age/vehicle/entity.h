#pragma once
#include <mc2hook\mc2hook.h>

class vehCarSim;
class vehModel;
class vehAudio;
class vehDamage;
class vehInput;
class phArchetype;

class vehEntity {
public:
	void* m_Vtable;
	phArchetype* m_Archetype; // phArchetypePhys?
	char pad_0000[60]; //0x0008
	vehInput* m_Input; //0x0044
	vehCarSim* m_CarSim; //0x0048
	vehModel* m_Model; //0x004C
	vehAudio* m_Audio; //0x0050
	void* m_Unk; //0x0054
	void* m_Feedback; //0x0058
	void* m_Driver; //0x005C
	void* m_Stuck; //0x0060
	void* m_Gyro; //0x0064
	char pad_0068[8]; //0x0068
	void* m_WheelPtx; //0x0070
	int dword_74;
	vehDamage* m_Damage;
};
