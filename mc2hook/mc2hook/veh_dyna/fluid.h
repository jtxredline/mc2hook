#pragma once

class mcCarSim;

class vehFluid
{
public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	float m_Current;
	int dword_10;
	int dword_14;
	int dword_18;
	int dword_1c;
	float dword_20;
	float m_Damp;
	char m_Anim[768];

public:
	void Init(mcCarSim* sim, int* a2, int* a3);
};
