#pragma once
#include <vector/matrix34.h>

class phArchetype;

class phInst
{
public:
	void* m_Vtable;
	phArchetype* m_Archetype;
	int dword_08;
	int dword_0c;
	Matrix34 m_Transform;

public:
	void SetArchetype(phArchetype* archetype);
};
