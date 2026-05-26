#include <mc2hook\mc2hook.h>
#include "carSSTurbo.h"

void mcCarSSTurbo::sub_4D4270()
{
	hook::Thunk<0x4D4270>::Call<void>(this); // Call original
}

bool mcCarSSTurbo::sub_4D4230()
{
	return hook::Thunk<0x4D4230>::Call<bool>(this); // Call original
}

void mcCarSSTurbo::sub_4D41F0()
{
	hook::Thunk<0x4D41F0>::Call<void>(this); // Call original
}
