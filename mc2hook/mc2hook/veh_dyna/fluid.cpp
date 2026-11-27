#include "fluid.h"
#include <mc2hook/mc2hook.h>

void vehFluid::Init(mcCarSim* sim, const Vector3* boxMin, const Vector3* boxMax)
{
	hook::Thunk<0x56B6E0>::Call<void>(this, sim, boxMin, boxMax); // Call original
}
