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

void phInertialCS::AccumulateForce(const Vector3& force)
{
	m_Force.X += m_Mass * force.X;
	m_Force.Y += m_Mass * force.Y;
	m_Force.Z += m_Mass * force.Z;
}

void phInertialCS::ComputeForce(float impulseScale, Vector3& out) const
{
    out = m_Force;

    if (impulseScale > 0.0f)
    {
        out.X += m_Impulse.X * impulseScale;
        out.Y += m_Impulse.Y * impulseScale;
        out.Z += m_Impulse.Z * impulseScale;
    }
}

void phInertialCS::ApplyForwardTorque(float amount)
{
    // Scale by angular inertia (Z axis)
    float scaled = amount * m_AngInertia.Z;

    // Forward vector (Z axis)
    const Vector3& forward = m_WorldTransform.GetRow(2);

    // Accumulate torque
    m_Torque.X += forward.X * scaled;
    m_Torque.Y += forward.Y * scaled;
    m_Torque.Z += forward.Z * scaled;
}

void phInertialCS::ApplyUpTorque(float amount)
{
    // Scale by angular inertia (Y axis)
    float scaled = amount * m_AngInertia.Y;

    // Up vector (Y axis)
    const Vector3& up = m_WorldTransform.GetRow(1);

    // Accumulate torque
    m_Torque.X += up.X * scaled;
    m_Torque.Y += up.Y * scaled;
    m_Torque.Z += up.Z * scaled;
}

void phInertialCS::ApplyLateralTorque(float amount)
{
    // Scale by angular inertia (X axis)
    float scaled = amount * m_AngInertia.X;

    // Lat vector (X axis)
    const Vector3& lat = m_WorldTransform.GetRow(0);

    // Accumulate torque
    m_Torque.X += lat.X * scaled;
    m_Torque.Y += lat.Y * scaled;
    m_Torque.Z += lat.Z * scaled;
}
