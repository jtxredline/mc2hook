#include "model.h"

void vehModel::Init(const char* carName, Matrix34* mtx, mcCarSim* sim, bool a5, bool a6, bool isBike)
{
	hook::Thunk<0x4CDC80>::Call<void>(this, carName, mtx, sim, a5, a6, isBike);
}
