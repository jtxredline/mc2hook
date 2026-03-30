#include <mc2hook\mc2hook.h>
#include "carmodel.h"

void vehModel::sub_4C4BC0(char a2)
{
	hook::Thunk<0x4C4BC0>::Call<void>(this, a2); // Call original
}
