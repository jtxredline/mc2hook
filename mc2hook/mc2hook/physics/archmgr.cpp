#include "archmgr.h"

declfield(phArchetypeMgr::Instance)(0x6C8EB4);

bool phArchetypeMgr::CreateInstance(int a1)
{
	return hook::StaticThunk<0x580590>::Call<bool>(a1); // Call original
}

phArchetype* phArchetypeMgr::RegisterArchetype(phBound* bound, const char* s, bool a3, int a4, float a5)
{
	return hook::Thunk<0x5800C0>::Call<phArchetype*>(this, bound, s, a3, a4, a5); // Call original
}
