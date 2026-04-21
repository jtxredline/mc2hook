#include <mc2hook\mc2hook.h>
#include "carsim.h"

int vehCarSim::OnGround()
{
    //return hook::Thunk<0x56A000>::Call<int>(this); // Call original

    int numWheelsOnGround = 0;

    for (int i = 0; i < m_NumWheels; i++)
    {
        if (m_Wheels[i]->m_OnGround)
            numWheelsOnGround++;
    }
    return numWheelsOnGround;
}

void vehCarSim::SetDrivable(int a2)
{
    hook::Thunk<0x4D2A50>::Call<void>(this, a2); // Call original
}

void vehCarSim::SetCenterOfMass(const Vector3& cg)
{
    this->m_CenterOfMass = cg;
}
