#include "cardamage.h"
#include <mc2hook/mc2hook.h>

void mcCarDamage::Init(vehEntity* entity, const char* carName)
{
	hook::Thunk<0x4CFC10>::Call<void>(this, entity, carName); // Call original
}
