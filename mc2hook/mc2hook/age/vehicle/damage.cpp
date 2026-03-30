#include <mc2hook\mc2hook.h>
#include "damage.h"

void vehDamage::sub_4CF500(int a2)
{
	hook::Thunk<0x4CF500>::Call<void>(this, a2); // Call original
}

bool vehDamage::sub_4CEC50()
{
	return hook::Thunk<0x4CEC50>::Call<bool>(this); // Call original
}
