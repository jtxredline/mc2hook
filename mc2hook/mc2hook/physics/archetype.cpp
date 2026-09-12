#include "archetype.h"

declfield(phArchetype::dword_674060)(0x674060);

float phArchetype::sub_47B9D0()
{
	return this->dword_3C; // Gravity factor?
}

void phArchetype::SetTypeFlag(uint mask, bool value)
{
	hook::Thunk<0x58E800>::Call<void>(this, mask, value); // Call original
}

void phArchetype::SetMass(float mass)
{
	hook::Thunk<0x47B960>::Call<void>(this, mass); // Call original
}

void phArchetype::SetAngInertia(const Vector3& angInertia)
{
	hook::Thunk<0x58EA10>::Call<void>(this, &angInertia); // Call original
}
