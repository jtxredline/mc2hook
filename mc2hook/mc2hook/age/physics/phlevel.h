#pragma once
#include <mc2hook\mc2hook.h>

class phCollider;
class phSurface;
class phIntersection;

class phLevel // TODO: Needs checking
{
public:
	int dword_00;
	int dword_04;
	int dword_08;
	int dword_0C;
	int dword_10;
	int dword_14;
	int dword_18;
	int dword_1C;
	int dword_20;
	int dword_24;
	int dword_28;
	int dword_2C;
	int dword_30;
	int dword_34;
	int dword_38;
	int dword_3C;
	int dword_40;
	int dword_44;
	int dword_48;
	int dword_4C;
	int dword_50;

public:
	static hook::Type<phLevel*> Instance;

public:
	phCollider* GetCollider(int levelIndex);
	static phSurface* GetMaterialFromIsect(phIntersection* isect);
};
