#pragma once
#include <mc2hook\mc2hook.h>

class phSurface // TODO: Needs checking
{
public:
	static hook::Type<phSurface*> dword_6CE214; // Not sure if phSurface
	static hook::Type<phSurface*> DefaultMaterial;

public:
	void* m_Vtable;
	int dword_04;
	int dword_08;
	int dword_0C;
	int dword_10;
	int dword_14;
	int dword_18;
	int dword_1C;
	int dword_20;
	int dword_24;
	float dword_28;
	float m_Friction;
	int dword_30;
	float m_Drag;
	float m_Width;
	float m_Height;
	float m_Depth;
	int dword_44;
	float dword_48;
	float dword_4C;
	int dword_50;
};
