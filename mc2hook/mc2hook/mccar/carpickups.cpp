#include "carpickups.h"
#include <mc2hook\mc2hook.h>
#include <core/output.h>

bool mcCarPickups::sub_4CEC50()
{
	return hook::Thunk<0x4CEC50>::Call<bool>(this); // Call original
}

void mcCarPickups::AttemptActivate(bool a2)
{
	hook::Thunk<0x4CF500>::Call<void>(this, a2); // Call original
}
