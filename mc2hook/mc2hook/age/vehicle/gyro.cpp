#include "gyro.h"
#include <age/vehicle/wheel.h>
#include <age/vehicle/carsim.h>
#include <age/data/timemgr.h>
#include <age/math/math.h>
#include <age/vehicle/vehinput.h>
#include <age/physics/archetype.h>

declfield(vehGyro::dword_6957C0)(0x6957C0);

void vehGyro::Update()
{
    // Wheel ptrs
    vehWheel* wheel_FL = m_CarSim->m_Wheels[0];
    vehWheel* wheel_RL = m_CarSim->m_Wheels[1];
    vehWheel* wheel_FR = m_CarSim->m_Wheels[2];
    vehWheel* wheel_RR = m_CarSim->m_Wheels[3];

    // Wheel contact positions
    Vector3 FLpos = wheel_FL->m_ContactMatrix.GetRow(3);
    Vector3 RLpos = wheel_RL->m_ContactMatrix.GetRow(3);
    Vector3 FRpos = wheel_FR->m_ContactMatrix.GetRow(3);
    Vector3 RRpos = wheel_RR->m_ContactMatrix.GetRow(3);

    // Cached vars
    float brake = m_CarSim->m_Brake;
    float handbrake = m_CarSim->m_Handbrake;
    float speed = m_CarSim->m_Speed;
    float steer = m_Input->m_Steer;
    float burnout = m_CarSim->m_BurnoutValue;
    float gasBrake = m_Input->m_GasBrake;

    phCollider* collider = m_CarSim->m_Collider;
    phInertialCS* ics = collider->m_ICS;
    Vector3 worldPos = ics->m_WorldTransform.GetRow(3);

    // Ground contact ratio (0.0f = all wheels airborne, 1.0f = all on ground)
    float wheelsOnGround = (float)m_CarSim->OnGround();
    float invWheelCount = 1.0f / (float)m_CarSim->m_NumWheels;
    float groundRatio = wheelsOnGround * invWheelCount;

    // Forward speed along the vehicle's local Z axis
    Matrix34& mat = collider->m_SomeInstParent->m_SomeInstParentTransform;
    Vector3& vel = collider->m_ICS->m_WorldVelocity;

    float forwardSpeed = -(mat.m20 * vel.X + mat.m21 * vel.Y + mat.m22 * vel.Z);
    float absForwardSpeed = fabs(forwardSpeed);

    // -------------------------------------------------------------------------
    // Handbrake torque / Burnout turn
    // -------------------------------------------------------------------------
    if ((dword_18 & 0x20000) != 0)
    {
        if (m_Drift <= 0.0f || handbrake <= 0.0f || burnout <= 0.0f)
        {
            if (m_Drift > 0.0f && handbrake > 0.0f && forwardSpeed > 10.0f)
            {
                float handbrakeTorqueScale = forwardSpeed;
                if (forwardSpeed < 25.0f)
                    handbrakeTorqueScale = forwardSpeed * forwardSpeed * 0.04f;

                handbrakeTorqueScale *= groundRatio * steer * handbrake;

                Vector3 handbrakeTorqueAxis;
                // Compute axis
                if (wheel_RR)
                {
                    handbrakeTorqueAxis.Midpoint(wheel_RL->m_ContactMatrix.GetRow(1), wheel_RR->m_ContactMatrix.GetRow(1));
                    handbrakeTorqueAxis.Normalize();
                }
                else
                {
                    handbrakeTorqueAxis = wheel_RL->m_ContactMatrix.GetRow(1);
                }

                // Force offset origin: midpoint of rear wheel contacts
                Vector3 handbrakeTorqueOrigin;
                if (wheel_RR)
                    handbrakeTorqueOrigin.Midpoint(wheel_RL->m_ContactMatrix.GetRow(3),
                                                   wheel_RR->m_ContactMatrix.GetRow(3));
                else
                    handbrakeTorqueOrigin = wheel_RL->m_ContactMatrix.GetRow(3);

                // Apply handbrake torque
                float handbrakeTorque = -(handbrakeTorqueScale * ics->m_AngInertia.Y * m_Drift);

                Vector3 handbrakeTorqueVec = handbrakeTorqueAxis * handbrakeTorque;
                Vector3 handbrakeTorqueOffset = handbrakeTorqueOrigin - worldPos;

                ApplyScaledTorqueAndForce(handbrakeTorqueVec, handbrakeTorqueOffset, ics->m_InvAngInertia.Y);
            }
        }
        else
        {
            // Burnout turning torque
            bool isRWD = m_CarSim->m_DrivetrainType == 1;

            float burnoutTurnTorqueScale =
                pow(m_CarSim->m_Engine->m_CurrentRPS / m_CarSim->m_Engine->m_OptRPS, 0.5f)
                * steer
                * m_BurnoutTurn
                * burnout;

            // Pick the driven axle wheels
            vehWheel* wA = isRWD ? m_CarSim->m_Wheels[1] : m_CarSim->m_Wheels[0];
            vehWheel* wB = isRWD ? m_CarSim->m_Wheels[3] : m_CarSim->m_Wheels[2];

            // FWD gets a larger scale
            if (!isRWD) burnoutTurnTorqueScale *= 2.0f;

            Vector3 burnoutTurnTorqueAxis;
            burnoutTurnTorqueAxis.Midpoint(wA->m_ContactMatrix.GetRow(1),
                                           wB->m_ContactMatrix.GetRow(1));

            Vector3 burnoutTurnTorqueOrigin;
            burnoutTurnTorqueOrigin.Midpoint(wA->m_ContactMatrix.GetRow(3),
                                             wB->m_ContactMatrix.GetRow(3));

            float burnoutTurnTorque = -(burnoutTurnTorqueScale * ics->m_AngInertia.Y * m_Drift);
            
            Vector3 burnoutTurnTorqueVec = burnoutTurnTorqueAxis * burnoutTurnTorque;
            Vector3 burnoutTurnOffset = burnoutTurnTorqueOrigin - worldPos;

            ApplyScaledTorqueAndForce(burnoutTurnTorqueVec, burnoutTurnOffset, ics->m_InvAngInertia.Y);
        }
    }
    // -------------------------------------------------------------------------
    // Gravity scale / airtime
    // -------------------------------------------------------------------------
    float gravityScale = collider->m_SomeInstParent->m_Archetype->sub_47B9D0();

    if (wheelsOnGround == 0.0f)
    {
        m_Airtime += datTimeManager::Seconds;
    }
    else if (m_Airtime > 0.0f)
    {
        m_Airtime = 0.0f;
        *(uint16_t*)&dword_4C = 30;
    }

    // -------------------------------------------------------------------------
    // Fishtail / drift force
    // -------------------------------------------------------------------------
    if ((dword_18 & 0x20000) != 0 && m_DriftThrust > 0.0f)
    {
        float driftForceScale = (handbrake > 0.0f && burnout <= 0.0f) ? handbrake : 0.0f;

        if (driftForceScale <= dword_44)
        {
            if (driftForceScale < dword_44)
                math::Approach(dword_44, driftForceScale, m_DriftDecay, datTimeManager::Seconds);
        }
        else
        {
            dword_44 = driftForceScale;
        }

        if (dword_44 > 0.0f)
        {
            // Alignment: how much velocity is sideways relative to vehicle forward
            Vector3 driftVelocityDir = ics->m_WorldVelocity;
            driftVelocityDir.Normalize();

            float driftAlignmentDot = fabs(driftVelocityDir.Dot(ics->m_WorldTransform.GetRow(0)));
            float driftAlignmentClamped = math::Clamp(driftAlignmentDot, 0.0f, 1.0f);

            float driftForceScale =
                -(math::EaseOutSine(driftAlignmentClamped) *
                    groundRatio *
                    *(float*)&m_CarSim->dword_154 *
                    m_DriftThrust *
                    dword_44 *
                    m_CarSim->m_Throttle *
                    m_CarSim->m_Drivetrain->m_WheelBL->m_SurfaceFriction);

            Vector3 driftForce = ics->m_WorldTransform.GetRow(2) * driftForceScale;

            ics->AccumulateForce(driftForce);
        }

        // Reduce driven wheel torque proportionally while drifting
        if (m_CarSim->m_NumWheels > 0)
        {
            float driftWheelTorqueScale = 1.0f - driftForceScale * 0.5f;
            for (int i = 0; i < m_CarSim->m_NumWheels; ++i)
                m_CarSim->m_Wheels[i]->m_WheelDriveTorque *= driftWheelTorqueScale;
        }
    }

    // -------------------------------------------------------------------------
    // 2-wheel lean
    // -------------------------------------------------------------------------
    if ((m_CarSim->field_184 & 2) == 0 && (dword_18 & 0x80000) != 0 && m_CarSim->m_NumWheels == 4)
    {
        Vector3 comPoint;
        // Initiate a lean if conditions are met (upright, on ground, no burnout, above min speed)
        if ((dword_4C & 0xFFFF) == 0 && wheelsOnGround != 0.0f && ics->m_WorldTransform.m11 > 0.8f)
        {
            if (burnout == 0.0f)
            {
                if ((m_Input->m_CurrentGearFlags & 0x800000) != 0 &&
                    !((int16_t)(dword_14 >> 16)) &&
                    speed > m_LeanSpeedMin)
                {
                    int16_t leanDir = 0;

                    if (m_Input->m_2WheelSteer > 0.0f) leanDir = 1;
                    else if (m_Input->m_2WheelSteer < 0.0f) leanDir = -1;

                    *(int16_t*)((char*)&dword_14 + 2) = leanDir;

                    // Shift center of mass and apply impulse in lean direction
                    comPoint = m_CarSim->m_CenterOfMass;
                    if (leanDir == -1)
                    {
                        comPoint.X = -(m_CarSim->m_Size.X * 0.5f * m_TwoWheelCOG);
                        m_CarSim->SetCenterOfMass(comPoint);
                        collider->ApplyLeanImpulse(m_LeanImpulseUp);
                    }
                    else if (leanDir == 1)
                    {
                        comPoint.X = m_CarSim->m_Size.X * 0.5f * m_TwoWheelCOG;
                        m_CarSim->SetCenterOfMass(comPoint);
                        collider->ApplyLeanImpulse(-m_LeanImpulseUp);
                    }

                    // Disable front steering while on 2 wheels
                    wheel_FL->m_SteeringLimit = 0.0f;
                    wheel_FR->m_SteeringLimit = 0.0f;
                    *(uint16_t*)&dword_4C = 30;
                }
            }
        }

    LABEL_65:
        uint16_t& leanTimer = *(uint16_t*)&dword_4C;
        if (leanTimer > 0) leanTimer--;

        int16_t currentLean = (int16_t)(dword_14 >> 16);
        if (!currentLean) goto LABEL_89;

        if ((m_Input->m_CurrentGearFlags & 0x10000) == 0 || currentLean == 0x63)
        {
            if (wheelsOnGround != 0.0f)
            {
                collider->ApplyLeanImpulse(currentLean == -1 ? -m_LeanImpulseDn : m_LeanImpulseDn);
            }
            *(int16_t*)((char*)&dword_14 + 2) = 0;
            leanTimer = 10;
        }

        currentLean = (int16_t)(dword_14 >> 16);

        if (currentLean)
        {
            if (speed < m_LeanSpeedMin) goto LABEL_82;

            if (leanTimer)              goto LABEL_87;

            if (collider->m_Inst->m_WorldTransform.m11 < 0.0f)
            {
            LABEL_82:
                if (wheelsOnGround != 0.0)
                {
                    collider->ApplyLeanImpulse(currentLean == -1 ? -m_LeanImpulseDn : m_LeanImpulseDn);
                }
                goto LABEL_86;
            }

            if (wheelsOnGround == 0.0f && m_Airtime > 1.0f)
            {
            LABEL_86:
                *(int16_t*)((char*)&dword_14 + 2) = 0;
                leanTimer = 0xA;
            }
        }

    LABEL_87:
        if (!((int16_t)(dword_14 >> 16)))
        {
            comPoint = m_CarSim->m_CenterOfMass;
            comPoint.X = 0.0f;
            m_CarSim->SetCenterOfMass(comPoint);
        }

    LABEL_89:
        currentLean = (int16_t)(dword_14 >> 16);

        if (currentLean && m_Lean > 0.0f)
        {
            float wheelZDiff = wheel_RL->m_LocalOffset.Z - wheel_FL->m_LocalOffset.Z;

            Vector3 leanTorqueAxis(0.0f, 0.0f, 0.0f);
            Vector3 leanTorqueOrigin(0.0f, 0.0f, 0.0f);

            if (currentLean >= 0)
            {
                leanTorqueOrigin.Midpoint(RRpos, FRpos);
                leanTorqueAxis.Subtract(RRpos, FRpos);
            }
            else
            {
                leanTorqueOrigin.Midpoint(RLpos, FLpos);
                leanTorqueAxis.Subtract(RLpos, FLpos);
            }

            leanTorqueAxis = leanTorqueAxis / wheelZDiff;

            float leanTargetAngle = asin(ics->m_WorldTransform.m01) -
                (float)(-(int16_t)(dword_14 >> 16)) *
                m_TwoWheelLeanAngle * 1.5705f;


            leanTargetAngle = math::Clamp(leanTargetAngle, -m_LeanLimit, m_LeanLimit);

            float leanTorqueScale = -((leanTargetAngle * m_Lean
                + leanTorqueAxis.Dot(ics->m_AngularVelocity)
                * m_LeanDamp * datTimeManager::InvSeconds
                * datTimeManager::PhysicsSecondsScale) // FPS dependency fix
                * ics->m_AngInertia.Z);

            Vector3 leanTorqueVec = leanTorqueAxis * leanTorqueScale;
            Vector3 leanTorqueOffset = leanTorqueOrigin - worldPos;

            ApplyScaledTorqueAndForce(leanTorqueVec, leanTorqueOffset, ics->m_InvAngInertia.Z);

            // Optional drag force to slow the vehicle while on 2 wheels
            if (m_TwoWheelDrag != 0.0f)
            {
                Vector3 velDir;
                velDir.Normalize(ics->m_WorldVelocity);
                ics->AccumulateForce(velDir * -m_TwoWheelDrag);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Wheelie
    // -------------------------------------------------------------------------
    if ((dword_18 & 0x100000) == 0)
        goto LABEL_118;

    if (burnout <= 0.0f || handbrake != 0.0f)
        goto LABEL_118;

    Vector3 wheelieTorqueAxis(0.0f, 0.0f, 0.0f);
    float wheelieGroundNormalY = 0.0f;

    if (!wheel_RL->m_OnGround)
    {
        if (!wheel_RR || !wheel_RR->m_OnGround)
            goto LABEL_112;

        wheelieTorqueAxis = wheel_RR->m_ContactMatrix.GetRow(0);
        wheelieGroundNormalY = wheel_RR->m_ContactMatrix.m21;
        goto LABEL_112;
    }

    if (!wheel_RR || !wheel_RR->m_OnGround)
    {
        wheelieTorqueAxis = wheel_RL->m_ContactMatrix.GetRow(0);
        wheelieGroundNormalY = wheel_RL->m_ContactMatrix.m21;
        goto LABEL_112;
    }

    wheelieTorqueAxis.Subtract(RRpos, RLpos);
    wheelieTorqueAxis.Normalize();
    wheelieGroundNormalY = (wheel_RR->m_ContactMatrix.m21 + wheel_RL->m_ContactMatrix.m21) * 0.5f;

LABEL_112:
    // Cross product of torque axis with vehicle up
    Vector3 wheelieCross;
    wheelieCross.X = wheelieTorqueAxis.Y * ics->m_WorldTransform.m12 - wheelieTorqueAxis.Z * ics->m_WorldTransform.m11;
    wheelieCross.Y = wheelieTorqueAxis.Z * ics->m_WorldTransform.m10 - wheelieTorqueAxis.X * ics->m_WorldTransform.m12;
    wheelieCross.Z = wheelieTorqueAxis.X * ics->m_WorldTransform.m11 - wheelieTorqueAxis.Y * ics->m_WorldTransform.m10;

    float wheelieGravity = -(gravityScale * -9.8f);

    float wheelieTargetAngle =
        atan2(speed * speed, wheelieGravity)
        * gasBrake
        * -0.79f
        * (m_Input->m_SteerRate + 1.0f) * 0.5f;

    Vector3 wheelieAccelDir;
    ics->ComputeForce(0.0f, wheelieAccelDir);
    wheelieAccelDir = wheelieAccelDir * ics->m_InvMass;

    float wheelieAccelDot = wheelieAccelDir.Dot(wheelieCross);
    float wheelieAngleCorrection = 0.0f;
    if (wheelieAccelDot < 0.0f)
        wheelieAngleCorrection = atan2(wheelieAccelDot, wheelieGravity) * gasBrake;

    // Force application point: midpoint between rear wheels
    Vector3 rlContact = wheel_RL->m_ContactMatrix.GetRow(3);
    Vector3 wheelieTorqueOffset;

    if (wheel_RR)
    {
        wheelieTorqueOffset.X = (wheel_RR->m_ContactMatrix.m30 + rlContact.X) * 0.5;
        wheelieTorqueOffset.Y = (wheel_RR->m_ContactMatrix.m31 + rlContact.Y) * 0.5;
        wheelieTorqueOffset.Z = (wheel_RR->m_ContactMatrix.m32 + rlContact.Z) * 0.5;
    }
    else wheelieTorqueOffset = rlContact;

    wheelieTorqueOffset = wheelieTorqueOffset - worldPos;

    float wheelieTorqueScale =
        (ics->m_WorldTransform.m21 - wheelieGroundNormalY - sin(wheelieAngleCorrection * 0.25f + wheelieTargetAngle * 0.75f))
        * m_Wheelie
        - (wheelieTorqueAxis.Dot(ics->m_AngularVelocity))
        * datTimeManager::InvSeconds * 0.1f
        * datTimeManager::PhysicsSecondsScale; // FPS dependency fix
    
    float wheelieTorque = wheelieTorqueScale * ics->m_AngInertia.X;
    Vector3 wheelieTorqueVec = wheelieTorqueAxis * wheelieTorque;

    ApplyScaledTorqueAndForce(wheelieTorqueVec, wheelieTorqueOffset, ics->m_InvAngInertia.X);

    // -------------------------------------------------------------------------
    // Turn / Spin / 180 Assist
    // -------------------------------------------------------------------------
LABEL_118:
    if (m_Turn <= 0.0f)
        goto LABEL_148;

    // Torque axis: average normal of rear wheels; origin: rear axle midpoint
    Vector3 turnSpinTorqueOrigin =
        (wheel_RL->m_ContactMatrix.GetRow(3) + wheel_RR->m_ContactMatrix.GetRow(3)) * 0.5f;

    Vector3 turnSpinTorqueAxis(
        (wheel_RR->m_ContactMatrix.m10 + wheel_RL->m_ContactMatrix.m10) * 0.5f,
        (wheel_RL->m_ContactMatrix.m11 + wheel_RR->m_ContactMatrix.m11) * 0.5f,
        (wheel_RL->m_ContactMatrix.m12 + wheel_RR->m_ContactMatrix.m12) * 0.5f);
    turnSpinTorqueAxis.Normalize();

    Vector3 turnSpinTorqueOffset = turnSpinTorqueOrigin - worldPos;

    float turnSpinTorqueScale = 0.0f;
    float turnSpinTorque = 0.0f;
    Vector3 turnSpinTorqueVec(0.0f, 0.0f, 0.0f);

    bool spinAssist = dword_18 & 0x10000;
    vehWheels* wheels = m_CarSim->m_WheelsStruct;

    if (spinAssist && brake > 0.0f)
    {
        float steerScale = m_CarSim->m_Steer;
        if (absForwardSpeed < 25.0f)
            steerScale *= absForwardSpeed * 0.04f;

        turnSpinTorqueScale =
            groundRatio
            * steerScale
            * (ics->m_AngInertia.Y
                * m_Spin180);
    }
    else if (spinAssist && handbrake > 0.0f)
    {
        // Reverse 180 spin assist (handbrake in reverse)
        if (m_CarSim->m_Transmission->m_CurrentGear || forwardSpeed >= -5.0f)
            goto LABEL_147;

        float reverseSpinScale = -steer;
        if (absForwardSpeed < 10.0f)
            reverseSpinScale *= absForwardSpeed * 0.1f;

        turnSpinTorqueScale =
            groundRatio
            * reverseSpinScale
            * (ics->m_AngInertia.Y
                * m_Reverse180);
    }
    else
    {
        if (m_TurnFactor > 0.0f)
        {
            float lowSpeedTurnScale = 1.0f;
            if (absForwardSpeed < 5.0f)
                lowSpeedTurnScale = absForwardSpeed * 0.2f;

            float angularVelDot = turnSpinTorqueAxis.Dot(ics->m_AngularVelocity);

            float turnParam = -1.0f
                / (wheels->m_Wheel_FR.m_LocalOffset.Z - wheels->m_Wheel_FL.m_LocalOffset.Z)
                * ((1.0f - wheels->m_Wheel_FL.m_OptimumSlipPercent)
                    * m_CarSim->m_Steer
                    * wheels->m_Wheel_FL.m_SteeringLimit
                    * forwardSpeed)
                * m_TurnFactor
                + (1.0f - m_TurnFactor) * angularVelDot
                - angularVelDot;

            turnSpinTorque =
                groundRatio
                * lowSpeedTurnScale
                * wheels->m_Wheel_FL.m_SurfaceFriction
                * ics->m_AngInertia.Y
                * turnParam
                * m_Turn;
            goto LABEL_146;
        }

        float directionSign = 0.0f;
        if (forwardSpeed > 0.0f) directionSign = 1.0f;
        else if (forwardSpeed < 0.0f) directionSign = -1.0f;

        float steerInput = directionSign * m_CarSim->m_Steer;

        if (absForwardSpeed > 0.0f)
        {
            if (absForwardSpeed < 5.0f)
                steerInput *= absForwardSpeed * 0.2f;
        }
        else
        {
            steerInput = 0.0f;
        }

        turnSpinTorqueScale =
            groundRatio
            * steerInput
            * wheels->m_Wheel_FL.m_SurfaceFriction
            * ics->m_AngInertia.Y
            * m_Turn;
    }

    turnSpinTorque = -turnSpinTorqueScale;

LABEL_146:
    turnSpinTorqueVec = turnSpinTorqueAxis * turnSpinTorque;

LABEL_147:
    ApplyScaledTorqueAndForce(turnSpinTorqueVec, turnSpinTorqueOffset, ics->m_InvAngInertia.Y);

    // -------------------------------------------------------------------------
    // Air control
    // -------------------------------------------------------------------------
LABEL_148:
    float airControlThreshold = ((dword_6957C0 & 0x1000) != 0) ? 0.0f : 0.25f;

    if ((m_CarSim->field_184 & 1) == 0 && (dword_18 & 0x40000) != 0)
    {
        if ((m_Input->m_CurrentGearFlags & 0x10000) != 0
            && m_CarSim->m_Airtime > airControlThreshold
            && (*(float*)&dword_2C > 0.0f || m_Pitch > 0.0f || m_RollTorque > 0.0f))
        {
            float airYawTorque = -(m_Input->m_2WheelSteer * *(float*)&dword_2C); // m_Yaw?

            ics->ApplyUpTorque(airYawTorque);

            float airPitchTorque = m_Input->m_SteerRate * m_Pitch;

            ics->ApplyLateralTorque(airPitchTorque);

            ph_Some_Inst_Parent* instParent = collider->m_SomeInstParent;

            float airCurrentRoll = asin(instParent->m_SomeInstParentTransform.m01);
            float airClampedRoll = math::Clamp(airCurrentRoll, -m_RollLimit, m_RollLimit);

            float airRollRate =
                instParent->m_SomeInstParentTransform.m22 * ics->m_AngularVelocity.Z
                + instParent->m_SomeInstParentTransform.m21 * ics->m_AngularVelocity.Y
                + instParent->m_SomeInstParentTransform.m20 * ics->m_AngularVelocity.X;

            airRollRate *= datTimeManager::PhysicsSecondsScale; // FPS dependency fix

            float airRollTorque =
                -(airClampedRoll * m_RollTorque
                    + airRollRate * m_RollDamp * datTimeManager::InvSeconds);

            ics->ApplyForwardTorque(airRollTorque);
        }
    }
}

void vehGyro::ApplyScaledTorqueAndForce(const Vector3& torque, const Vector3& offset, float scale)
{
    phInertialCS* ics = m_CarSim->m_Collider->m_ICS;

    // Generate force
    Vector3 force;
    force.X = offset.Z * torque.Y - offset.Y * torque.Z;
    force.Y = offset.X * torque.Z - offset.Z * torque.X;
    force.Z = offset.Y * torque.X - offset.X * torque.Y;

    float mul = -(ics->m_Mass * scale);

    ics->m_Force.X += force.X * mul;
    ics->m_Force.Y += force.Y * mul;
    ics->m_Force.Z += force.Z * mul;

    // Apply scaled torque
    ics->m_Torque.X += torque.X * m_CarSim->dword_48.X;
    ics->m_Torque.Y += torque.Y * m_CarSim->dword_48.Y;
    ics->m_Torque.Z += torque.Z * m_CarSim->dword_48.Z;
}
