#include <mc2hook\mc2hook.h>
#include "phinertia.h"
#include <age/data/timemgr.h>

#include <age/core/output.h> //

void phInertialCS::ApplyContactForce(Vector3* someForce, Vector3* somePos, Matrix34* a4, Vector3* a5)
{
	hook::Thunk<0x595120>::Call<void>(this, someForce, somePos, a4, a5); // Call original
}

void phInertialCS::AddForce(Vector3* a2, Vector3* a3)
{
    //m_Force = m_Force + f;
    hook::Thunk<0x595280>::Call<void>(this, a2, a3); // Call original
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

void phInertialCS::ApplyPush(Vector3* a2, Vector3* a3, int a4)
{
    hook::Thunk<0x5952C0>::Call<void>(this, a2, a3, a4); // Call original
}

void phInertialCS::GetLocalVelocity(Vector3* position, Vector3* velocity, int a4) // Get velocity at point
{
    Vector3 offset(position->X - m_WorldTransform.m30,
                   position->Y - m_WorldTransform.m31,
                   position->Z - m_WorldTransform.m32);

    *velocity = m_WorldVelocity + Vector3::Cross(m_AngularVelocity, offset);
}


void phInertialCS::GetLocalFilteredVelocity2(Vector3* out, Vector3* vel) // out might just be offset
{    
    hook::Thunk<0x595A60>::Call<void>(this, out, vel); // Call original
    return;

    /////////
    
    // Get local velocity (at point?)
    GetLocalVelocity(out, vel, 0);

    // Squared magnitude of last push vector
    float lastPushMagSq = m_LastPush.Mag2();

    // Ignore tiny pushes
    if (lastPushMagSq > 0.0001f)
    {
        // Projection of velocity onto last push direction
        float pushDotVelocity = m_LastPush.Dot(*vel);
        
        // Only counter velocity opposing the push
        if (pushDotVelocity < 0.0f)
        {
            float invSeconds = datTimeManager::InvSeconds;
            float scaledDot = pushDotVelocity * invSeconds;
            float maxCorrection = invSeconds * invSeconds * lastPushMagSq;

            // Clamp correction amount
            if (-scaledDot <= maxCorrection)
                invSeconds *= (-1.0 / maxCorrection * scaledDot);

            // Apply filtered push compensation
            vel->X = invSeconds * this->m_LastPush.X + vel->X;
            vel->Y = invSeconds * this->m_LastPush.Y + vel->Y;
            vel->Z = invSeconds * this->m_LastPush.Z + vel->Z;
        }
    }
}

float phInertialCS::CalcCollisionNoFriction(Vector3* a2, float a3, Vector3* a4)
{
    return hook::Thunk<0x5957D0>::Call<float>(this, a2, a3, a4); // Call original
}
