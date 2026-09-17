#include "caraero.h"
#include <age/vehicle/vehinput.h>

void mcCarAero::Update()
{
    //hook::Thunk<0x4E5480>::Call<void>(this); // Call original

    if (m_BikeInput && (m_BikeInput->m_CurrentGearFlags & 0x10000) != 0 && m_BikeInput->m_SteerRate < -0.5f)
    {
        this->dword_34 = this->m_DragSlipstream;
        this->dword_34 += this->dword_48;
        vehAero::Update();
    }
    else
    {
        this->dword_34 = this->m_Drag;
        this->dword_34 += this->dword_48;
        vehAero::Update();
    }
}

void mcCarAero::Init(mcCarSim* sim, vehPlayerInput* input)
{
    hook::Thunk<0x4E5450>::Call<void>(this, sim, input); // Call original
}
