#include "joystick.h"
#include <age/gfx/pipeline.h>

declfield(ioJoystick::MaybeActiveJoystick)(0x6627D0);
declfield(ioJoystick::sm_Sticks)(0x85FC00);
declfield(ioJoystick::dword_85FC90)(0x85FC90);

void ioJoystick::BeginAll()
{
	hook::StaticThunk<0x605020>::Call<void>();
}

void ioJoystick::Poll()
{
	hook::StaticThunk<0x604E80>::Call<void>();
}
void ioJoystick::Update()
{
	hook::StaticThunk<0x604EA0>::Call<void>();
}
void ioJoystick::UpdateAll()
{
	hook::StaticThunk<0x6050A0>::Call<void>();
}
void ioJoystick::End()
{
	hook::StaticThunk<0x604FF0>::Call<void>();
}
void ioJoystick::EndAll()
{
	hook::StaticThunk<0x6050D0>::Call<void>();
}
