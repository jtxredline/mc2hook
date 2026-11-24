#include "playerinput.h"

void vehPlayerInput::PlaybackReplay()
{
    hook::Thunk<0x5684C0>::Call<void>(this);
}

void vehPlayerInput::Update()
{
    hook::Thunk<0x568460>::Call<void>(this);
}
