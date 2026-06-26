#include "factory.h"
#include <age/vehicle/entity.h>

vehEntity* mcPlayerFactory::MakeEntity() // TODO: Rewrite this
{
	//vehEntity* ghost = hook::Thunk<0x46BE10>::Call<vehEntity*>(this);
	//int ghost = hook::Thunk<0x46BE10>::Call<int>(this); // This creates a second car already
	//ghost->m_Input = nullptr;

	return hook::Thunk<0x46BE10>::Call<vehEntity*>(this); // Call original
}
