#pragma once
#include <age/physics/ph_some_inst_parent.h>

class phInertialCS;
class phInstOld;

class phCollider {
public:
	void* m_Vtable;
	ph_Some_Inst_Parent* m_SomeInstParent; // ?
	int dword_08;
	int dword_0c;
	phInertialCS* m_ICS;
	phInstOld* m_Inst;
	Matrix34 m_MaybeWorldTransform;
	int dword_48;
	int dword_4c;
	int dword_50;
	int dword_54;
	int dword_58;
	int dword_5c;
	int dword_60;
	int dword_64;
	int dword_68;
	int dword_6c;
	int dword_70;
	int dword_74;
	int dword_78;

public:
	void ApplyLeanImpulse(float impulse);
};
