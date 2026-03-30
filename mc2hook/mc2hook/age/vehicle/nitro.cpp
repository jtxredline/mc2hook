#include <mc2hook\mc2hook.h>
#include "nitro.h"

void vehNitro::sub_4D1F80()
{
	hook::Thunk<0x4D1F80>::Call<void>(this); // Call original
}

bool vehNitro::sub_46A350()
{
	return hook::Thunk<0x46A350>::Call<bool>(this); // Call original
}
