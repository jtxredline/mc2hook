#include "fluid.h"
#include <mc2hook/mc2hook.h>

void vehFluid::Init(mcCarSim* sim, int* a2, int* a3)
{
	hook::Thunk<0x56B6E0>::Call<void>(this, sim, a2, a3); // Call original
}
