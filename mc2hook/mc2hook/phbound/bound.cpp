#include "bound.h"
#include <mc2hook\mc2hook.h>
#include <phcore/surface.h>

phSurface* phBound::sub_5A5550(int a2, int a3)
{
	return hook::Thunk<0x5A5550>::Call<phSurface*>(this, a2, a3); // Call original
}

phBound* phBound::Load(const char* boundName)
{
	return hook::StaticThunk<0x56D020>::Call<phBound*>(boundName); // Call original
}
