#include "drivetrain.h"

void vehDrivetrain::Detach()
{
	m_Engine = nullptr;
}

void vehDrivetrain::Attach()
{
	m_Engine = m_CarSim->m_Engine;
}
