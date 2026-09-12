#include "entity.h"

void vehEntity::Delete(bool a2)
{
	hook::Thunk<0x4D1E40>::Call<void>(this, a2); // Call original
}

void vehEntity::SetPhysFlag(short flag, bool a2)
{
	hook::Thunk<0x58FD80>::Call<void>(this, flag, a2); // Call original
}
