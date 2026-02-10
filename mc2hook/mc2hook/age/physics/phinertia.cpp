#include <mc2hook\mc2hook.h>
#include "phinertia.h"

void phInertialCS::ApplyContactForce(Vector3* someForce, Vector3* somePos, Matrix34* a4, Vector3* a5)
{
	hook::Thunk<0x595120>::Call<void>(this, someForce, somePos, a4, a5); // Call original
}

void phInertialCS::AddForce(const Vector3& f)
{
	m_Force += f;
}
