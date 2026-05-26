#include "bound.h"
#include <mc2hook\mc2hook.h>
#include <age/physics/phsurface.h>

phSurface* phBound::sub_5A5550(int a2, int a3)
{
	return hook::Thunk<0x5A5550>::Call<phSurface*>(this, a2, a3); // Call original
}
