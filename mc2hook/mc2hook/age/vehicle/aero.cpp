#include "aero.h"
#include <age/vehicle/carsim.h>
#include <age/physics/phcollider.h>
#include <age/physics/phinertia.h>
#include <age/math/math.h>
#include <age/data/timemgr.h>
#include <age/vehicle/vehinput.h>

void vehAero::Update()
{
    if (this->m_BikeInput && (*(unsigned char*)(this->m_BikeInput + 0x2A) & 1) != 0 && *(float*)(this->m_BikeInput + 0x20) < -0.5f)
    {
        this->dword_34 = this->m_DragSlipstream;
        this->dword_34 += this->dword_48;
        Update2();
    }
    else
    {
        this->dword_34 = this->m_Drag;
        this->dword_34 += this->dword_48;
        Update2();
    }
}

void vehAero::Update2()
{
    float localAngularVelX;
    float localAngularVelY;
    float localAngularVelZ;

    float absAngularVel;
    float dampingTorque;

    float speed = m_CarSim->m_Speed;

    phInertialCS* ics = m_CarSim->m_Collider->m_ICS;

    // Aero disabled
    if ((this->dword_08 & 1) == 0)
        return;

    // Angular damping / stabilization
    if (this->dword_04 > 0.0f)
    {
        localAngularVelX  = ics->m_WorldTransform.m02 * ics->m_AngularVelocity.Z
            + ics->m_WorldTransform.m01 * ics->m_AngularVelocity.Y
            + ics->m_WorldTransform.m00 * ics->m_AngularVelocity.X;

        localAngularVelY = ics->m_WorldTransform.m12 * ics->m_AngularVelocity.Z
            + ics->m_WorldTransform.m11 * ics->m_AngularVelocity.Y
            + ics->m_AngularVelocity.X * ics->m_WorldTransform.m10;

        localAngularVelZ = ics->m_WorldTransform.m22 * ics->m_AngularVelocity.Z
            + ics->m_WorldTransform.m21 * ics->m_AngularVelocity.Y
            + ics->m_AngularVelocity.X * ics->m_WorldTransform.m20;

        // X axis damping
        absAngularVel  = fabs(localAngularVelX);
        if (absAngularVel > 0.1f)
        {
            dampingTorque = -(math::Sign(localAngularVelX) * this->m_AngCDamp.X)
                - localAngularVelX * this->m_AngVelDamp.X
                - fabs(localAngularVelX) * localAngularVelX * this->m_AngVel2Damp.X;

            if (fabs(dampingTorque) * datTimeManager::Seconds > absAngularVel)
                dampingTorque = -(datTimeManager::InvSeconds * localAngularVelX);

            if (absAngularVel < 1.0)
                dampingTorque *= absAngularVel;

            if (this->dword_04 != 1.0f)
                dampingTorque *= this->dword_04;

            ics->ApplyLateralTorque(dampingTorque);
        }

        // Y axis damping
        absAngularVel = fabs(localAngularVelY);
        if (absAngularVel > 0.1f)
        {
            dampingTorque = -(math::Sign(localAngularVelY) * this->m_AngCDamp.Y)
                - localAngularVelY * this->m_AngVelDamp.Y
                - fabs(localAngularVelY) * localAngularVelY * this->m_AngVel2Damp.Y;

            if (fabs(dampingTorque) * datTimeManager::Seconds > absAngularVel)
                dampingTorque = -(datTimeManager::InvSeconds * localAngularVelY);

            if (absAngularVel < 1.0f)
                dampingTorque *= absAngularVel;

            if (this->dword_04 != 1.0f)
                dampingTorque *= this->dword_04;

            ics->ApplyUpTorque(dampingTorque);
        }

        // Z axis damping
        absAngularVel = fabs(localAngularVelZ);
        if (absAngularVel > 0.1f)
        {
            dampingTorque = -(math::Sign(localAngularVelZ) * this->m_AngCDamp.Z)
                - localAngularVelZ * this->m_AngVelDamp.Z
                - fabs(localAngularVelZ) * localAngularVelZ * this->m_AngVel2Damp.Z;

            if (fabs(dampingTorque) * datTimeManager::Seconds > absAngularVel)
                dampingTorque = -(datTimeManager::InvSeconds * localAngularVelZ);

            if (absAngularVel < 1.0f)
                dampingTorque *= absAngularVel;

            if (this->dword_04 != 1.0f)
                dampingTorque *= this->dword_04;

            ics->ApplyForwardTorque(dampingTorque);
        }
    }

    // Downforce
    if (speed > 15.0f)
    {
        // Force magnitude scales with speed squared
        float downforceScale = speed * speed * ics->m_Mass * m_Down * -0.001f * datTimeManager::PhysicsSecondsScale; // Downforce FPS dependency fix

        float downforceX = downforceScale * ics->m_WorldTransform.m10;
        float downforceY = downforceScale * ics->m_WorldTransform.m11;
        float downforceZ = downforceScale * ics->m_WorldTransform.m12;

        int bottomedWheelCount = m_CarSim->BottomedOut();

        if (bottomedWheelCount)
        {
            float bottomOutScale = 1.0f - ((float)bottomedWheelCount / (float)m_CarSim->m_NumWheels);

            downforceX *= bottomOutScale;
            downforceY *= bottomOutScale;
            downforceZ *= bottomOutScale;
        }

        if (this->dword_04 < 1.0f)
        {
            downforceX *= this->dword_04;
            downforceY *= this->dword_04;
            downforceZ *= this->dword_04;
        }

        // Apply downforce
        ics->m_Force.X += downforceX;
        ics->m_Force.Y += downforceY;
        ics->m_Force.Z += downforceZ;
    }

    // Drag
    if (speed > 10.0f)
    {
        float dragScale = ics->m_Mass * this->dword_34 * speed * -0.001f * datTimeManager::PhysicsSecondsScale; // Drag force FPS dependency fix

        ics->m_Force.X += dragScale * ics->m_WorldVelocity.X;
        ics->m_Force.Y += dragScale * ics->m_WorldVelocity.Y;
        ics->m_Force.Z += dragScale * ics->m_WorldVelocity.Z;
    }
}

void vehAero::sub_4E5450(vehCarSim* sim, vehInput* input)
{
    hook::Thunk<0x4E5450>::Call<void>(this, sim, input); // Call original
}
