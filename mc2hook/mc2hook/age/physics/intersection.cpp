#include "intersection.h"
#include <mc2hook\mc2hook.h>
#include <age/vector/vector3.h> //

Vector3* phIntersection::sub_58FDD0(Vector3* a1, Vector3* a2)
{
	return hook::Thunk<0x58FDD0>::Call<Vector3*>(this, a1, a2); // Call original
}
