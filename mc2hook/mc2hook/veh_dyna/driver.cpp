#include "driver.h"
#include <mc2hook/mc2hook.h>

void vehDriver::Init(mcCarSim* carSim, const char* carName)
{
	hook::Thunk<0x56AF50>::Call<void>(this, carSim, carName); // Call original
}
