#pragma once
#include <age/vector/matrix34.h>

class phArchetype;

class ph_Some_Inst_Parent { // Confirm this later
public:
	int dword_00;
	phArchetype* m_Archetype;
	int dword_08;
	int dword_0c;
	Matrix34 m_SomeInstParentTransform;
};
