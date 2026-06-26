#pragma once
#include <mc2hook\mc2hook.h>

class vehEntity;

class mcPlayerFactory {
public:
	int dword_00;
	int dword_04;
	int dword_08;
	int dword_0c;

public:
	vehEntity* MakeEntity();
};
