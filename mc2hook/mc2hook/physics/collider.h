#pragma once
#include <mc2hook/mc2hook.h>
#include <memory/age_alloc_baseclass.h>
#include <age/physics/ph_some_inst_parent.h>

class phInertialCS;
class phInstOld;
class mcCarSim;
class phBound;
class vehEntity;

class phCollider : public AGEAllocatedClass
{
public:
	void* m_Vtable;
	ph_Some_Inst_Parent* m_SomeInstParent; // ?
	int dword_08;
	phBound* m_Bound;
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
	mcCarSim* m_CarSim;

public:
	phCollider()  { hook::Thunk<0x591420>::Call<void>(this); }
	~phCollider() { hook::Thunk<0x591440>::Call<void>(this); }

	static hook::Type<int> SomeVtable;

	void ApplyLeanImpulse(float impulse);
	void Init(vehEntity* entity, phInertialCS* ics, int a3);
};

static_assert(sizeof(phCollider) == 0x7C, "phCollider size mismatch");
