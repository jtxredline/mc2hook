#include <mc2hook\mc2hook.h>
#include "caraudio.h"

void vehAudio::sub_4D6800(uint8_t a2)
{
	hook::Thunk<0x4D6800>::Call<void>(this, a2); // Call original
}
