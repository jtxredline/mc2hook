#pragma once
#include <age/vector/vector3.h>

class phSimulator {
public:
	void* m_Vtable;
	int dword_04;
	int m_ContactMgr;
	int dword_0c;
	int dword_10;
	int dword_14;
	float dword_18;
	int dword_1c;
	int dword_20;
	int dword_24;
	int dword_28;
	int dword_2c;

public:
	bool ApplyForce(int bodyId, Vector3* force, Vector3* position, float mass);
};

extern phSimulator* g_phSimulator;
