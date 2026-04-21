#include "hudMap.h"

declfield(hudMap::dword_85B2A4)(0x85B2A4);
declfield(hudMap::dword_85B2B0)(0x85B2B0);
declfield(hudMap::dword_85B2AC)(0x85B2AC); // Some ptr
declfield(hudMap::dword_85B2C0)(0x85B2C0);

void hudMap::sub_4A93C0()
{
	hook::Thunk<0x4A93C0>::Call<void>(this);
}
