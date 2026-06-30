#include "car.h"

//#include <age/core/output.h> //

void mcCar::Update()
{
	//Printf("mcCar: %p\n", this);
	return hook::Thunk<0x4D1710>::Call<void>(this);
}
