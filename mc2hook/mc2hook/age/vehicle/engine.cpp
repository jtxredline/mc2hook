#include "engine.h"

#include <mc2hook\mc2hook.h>
#include <age/data/timemgr.h>
#include <age/math/math.h>

void vehEngine::Update()
{
    //hook::Thunk<0x5FBD0>::Call<void>(this); // Call original
    
    float currentRatio = m_Transmission->GetRatio();

    // Gear change init
    if (m_Transmission->m_GearChangeFlag && !m_GearChangeActive)
    {
        if (!m_GearChangeForced)
        {
            m_GearChangeTimer = (m_Transmission->m_GearChangeFlag <= 0) ? m_GCL * 0.25f : m_GCL;
            m_GearChangeActive = true;
        }
    }

    // Gear change update
    if (m_GearChangeActive)
    {
        if (m_GearChangeTimer > 0.0f)
        {
            m_ThrottleValue = m_GearChangeThrottle;
            m_GearChangeTimer -= datTimeManager::GetSeconds();

            if (m_GearChangeTimer < 0.0f)
                m_GearChangeTimer = 0.0f;
        }

        if (m_GearChangeTimer == 0.0f)
        {
            m_GearChangeActive = false;
            m_Transmission->SetGearChangeFlag(0);
        }
        else if (m_Transmission->m_GearChangeFlag > 0 &&
            m_Transmission->m_CurrentGear > 2)
        {
            if (fabs(currentRatio * m_Drivetrain->m_SomeRPS) > m_CurrentRPS)
            {
                m_GearChangeTimer = 0.0f;
                m_GearChangeActive = false;
                m_Transmission->SetGearChangeFlag(0);
            }
        }
    }

    // Torque computation
    float minTorque = CalcTorqueAtZeroThrottle() * (1.0f - m_ThrottleValue);
    float maxTorque = CalcTorqueAtFullThrottle(m_CurrentRPS);
    m_Torque = maxTorque * m_ThrottleValue + minTorque; // Actual force (used in vehDrivetrain?)

    // Drivetrain attach / detach
    bool shouldDetach = currentRatio == 0.0f || m_GearChangeForced || m_Transmission->m_Clutch;

    if (!shouldDetach)
    {
        float r = currentRatio * m_Drivetrain->m_SomeRPS;

        // Engine RPM too far from idle while in low gear and low throttle
        bool idleMismatch = fabs(m_IdleRPS * 0.75f) > fabs(r);
        bool lowGear = m_Transmission->m_CurrentGear == 0 || m_Transmission->m_CurrentGear == 2;

        if (idleMismatch && lowGear && m_ThrottleValue < 0.1f)
            shouldDetach = true;
    }

    if (shouldDetach)
    {
        if (m_Drivetrain->m_Engine)
            m_Drivetrain->Detach();
    }
    else
    {
        if (currentRatio != 0.0f &&
            !m_Drivetrain->m_Engine &&
            (fabs(m_IdleRPS * 1.25f) < fabs(m_Transmission->GetRatio() * m_Drivetrain->m_SomeRPS) ||
            m_ThrottleValue > 0.0f))
        {
            m_Drivetrain->Attach();
        }
    }

    // Free-running RPM update
    if (!m_Drivetrain->m_Engine || m_Transmission->m_Clutch)
    {
        if (m_GearChangeForced)
        {
            m_CurrentRPS = 0.0f;
            m_Torque = 0.0f;
        }
        else
        {
            m_CurrentRPS += (m_Torque / m_AngInertia) * datTimeManager::GetSeconds();

            if (m_CurrentRPS < 0.0f)
                m_CurrentRPS = 0.0f;
            else if (m_CurrentRPS > m_MaxRPS)
                m_CurrentRPS = m_MaxRPS;
        }
    }

    // Derived outputs
    m_CurrentRPM = m_CurrentRPS * math::RPS_TO_RPM;
    unk_field_68 = m_Torque * m_CurrentRPS * math::NMS_TO_HP;

    float jiggleFactor = (m_TorqueAtOptRPS > 0.0f) ? (m_Torque / m_TorqueAtOptRPS) * m_AngInertia : 0.0f;

    // Visual jiggle
    if (m_PivotTransform && m_Instance)
    {
        float jiggleTarget = jiggleFactor * 0.05f;
        math::Approach(m_JiggleAngle, jiggleTarget, 0.5f, datTimeManager::GetSeconds());

        if (!m_GearChangeForced)
            m_JiggleAngle += math::frand() * (math::DEG_TO_RAD * 2.0f) - math::DEG_TO_RAD;

        Matrix34 m;
        m.Set(*m_PivotTransform);
        m.Rotate(m.GetRow(2), m_JiggleAngle);
        m_InstanceTransform->Dot(m, m_Instance->m_WorldTransform);
    }

    // Reaction torque
    if (m_Transmission->m_CurrentGear == 1 || m_Transmission->m_Clutch)
    {
        float t = -(jiggleFactor * m_AngInertia) * m_ICS->m_AngInertia.Z;

        m_ICS->m_Torque.X += t * m_ICS->m_WorldTransform.m20;
        m_ICS->m_Torque.Y += t * m_ICS->m_WorldTransform.m21;
        m_ICS->m_Torque.Z += t * m_ICS->m_WorldTransform.m22;
    }

    // Boost timer
    if (m_BoostTimer > 0.0f)
    {
        m_BoostTimer -= datTimeManager::GetSeconds();
        if (m_BoostTimer <= 0.0f)
            StopBoost();
    }
}

float vehEngine::CalcTorqueAtFullThrottle(float angVel) const
{
    float hp = GetMaxHP();

    // Below or at optimal RPM
    if (angVel <= m_OptRPS)
        return hp * ((m_InvFib * m_OptRPS + angVel) * (m_Fib * m_OptRPS - angVel) * m_UnkComputed_1 * m_HPScale);

    // Above redline
    if (angVel > m_MaxRPS)
        return 0.0f;

    // Between opt_rps and max_rps
    float torque =
        hp *
        ((angVel + m_MaxRPS - (m_OptRPS + m_OptRPS)) *
        (m_InvFib * m_OptRPS + angVel) *
        (m_Fib * m_OptRPS - angVel) *
        (m_MaxRPS - angVel) *
        m_UnkComputed_1 *
        m_InvOptRPSDiffSq *
        m_HPScale);

    if (torque < 0.0)
        return 0.0f;

    return torque;
}

float vehEngine::CalcTorqueAtZeroThrottle() const
{
    float idleDiff = m_IdleRPS - m_CurrentRPS;

    float hpMultiplier;
    if (idleDiff > 0.0)
        hpMultiplier = 1.0f;
    else if (idleDiff < 0.0)
        hpMultiplier = -1.0f;
    else
        hpMultiplier = 0.0f;

    float invOptRPS = 1.0 / m_OptRPS;
    float hp = GetMaxHP();

    return hp * m_HPScale * invOptRPS * 746.0* (invOptRPS * m_CurrentRPS) * hpMultiplier;
}

float vehEngine::GetMaxHP() const
{
    if (m_BoostTimer <= 0.0f)
        return m_MaxHP;

    return (m_BoostTimer / m_BoostDuration) * m_BoostHP + m_MaxHP;
}

void vehEngine::StopBoost()
{
    m_BoostTimer = 0.0f;
    m_Transmission->m_Clutch = 0;
}

void vehEngine::StartBoost(float boost)
{
    if (boost <= 0.0f)
        m_BoostTimer = m_BoostDuration;
    else
        m_BoostTimer = boost;
}
