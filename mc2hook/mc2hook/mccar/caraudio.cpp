#include "caraudio.h"

void mcCarAudio::sub_4D6800(uint8_t a2)
{
	hook::Thunk<0x4D6800>::Call<void>(this, a2); // Call original
}

void mcCarAudio::Init(vehEntity* a2, const char* carName, sndControlVolumeGroup* volumeGroup)
{
	hook::Thunk<0x4D6F40>::Call<void>(this, a2, carName, volumeGroup); // Call original
}
