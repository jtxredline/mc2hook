#include "inst.h"
#include <mc2hook/mc2hook.h>

void phInst::SetArchetype(phArchetype* archetype)
{
	hook::Thunk<0x58FD50>::Call<void>(this, archetype); // Call original
}
