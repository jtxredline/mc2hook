#include "car.h"

void mcCar::Update()
{
	return hook::Thunk<0x4D1710>::Call<void>(this);
}
