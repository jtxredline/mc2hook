#pragma once
#include <mc2hook/mc2hook.h>

class phArchetype;
class phBound;

class phArchetypeMgr
{
public:
    int dword_00;
    int dword_04;
    int dword_08;
    int dword_0c;
    int dword_10;
    int dword_14;
    int dword_18;
    int dword_1c;
    int dword_20;
    int dword_24;
    int dword_28;

public:
	static hook::Type<phArchetypeMgr*> Instance;

	static bool CreateInstance(int a1);
    phArchetype* RegisterArchetype(phBound* bound, const char* s, bool a3, int a4, float a5);
};
