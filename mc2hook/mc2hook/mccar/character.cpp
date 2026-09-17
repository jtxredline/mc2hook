#include "character.h"

void mcCharacter::Init(mcCarSim* carSim, vehInput* input, const char* carName, const char* suffix, int a6, bool a7)
{
	hook::Thunk<0x4D91F0>::Call<void>(this, carSim, input, carName, suffix, a6, a7); // Call original
}

void mcCharacter::sub_4D8F30(int a2)
{
	hook::Thunk<0x4D8F30>::Call<void>(this, a2); // Call original
}
