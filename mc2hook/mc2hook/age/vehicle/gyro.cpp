#include "gyro.h"
#include <age/vehicle/wheel.h>
#include <age/vehicle/carsim.h>
#include <age/data/timemgr.h>
#include <age/math/math.h>
#include <age/vehicle/vehinput.h>
#include <age/physics/archetype.h>

#include <age/core/output.h> //

declfield(vehGyro::dword_6957C0)(0x6957C0);

// WIP
void vehGyro::Update()
{
    __int16 leanTimer;
    __int16 tempLean;
    __int16 currentLean;
    double wheelZDiff;
    long double targetLean;
    long double clampedLean;
    bool rearWheelOnGround;
    float contactM01;
    float contactM02;
    float contactM21;
    float gasBrakeInput;
    double wheelieGravityRef;
    long double wheelieAngleCorrection;
    long double wheelieBaseTorque;
    long double finalWheelieTorque;
    int spinAssistFlag;
    long double spinTorque;
    double finalTurnTorque;
    double directionSign;
    long double lowSpeedFactor;
    double airControlThreshold;
    float rollLimitNeg;
    float currentRollAngle;
    float negLeanUpB;
    float negLeanUpC;
    float negLeanUpD;
    float wheelHeightDelta;
    float minLeanClamp;
    float wheelieDot;
    float reverseSpinScale;
    float lowSpeedTurnScale;
    float lowSpeedAbs;
    Vector3 torqueVec;
    Vector3 forceOffset;
    Vector3 comPoint;
    float driftScale;
    float burnoutScale;
    Vector3 axis;
    Vector3 applyOffset;

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

    float onGroundFactorFinal = brake;

    phCollider* collider = m_CarSim->m_Collider;
    phInertialCS* ics = collider->m_ICS;
    Vector3 worldPos = ics->m_WorldTransform.GetRow(3);

    const float wheelsOnGround = (float)m_CarSim->OnGround();
    const float invWheelCount = 1.0f / (float)m_CarSim->m_NumWheels;
    const float groundRatio = wheelsOnGround * invWheelCount;

    // Compute forward velocity
    const Matrix34& mat = collider->m_SomeInstParent->m_SomeInstParentTransform;
    const Vector3& vel = collider->m_ICS->m_WorldVelocity;

    float forwardSpeed = -(mat.m20 * vel.X + mat.m21 * vel.Y + mat.m22 * vel.Z);
    const float absForwardSpeed = fabs(forwardSpeed);

    //Vector3 torqueVec, forceOffset, comPoint, axis, applyOffset;
    //float driftScale = 0.0f;
    //float burnoutScale = 0.0f;

    // Drift / handbrake torque
    if ((dword_18 & 0x20000) != 0)
    {
        if (m_Drift <= 0.0f || handbrake <= 0.0f || burnout <= 0.0f)
        {
            // Normal drift torque
            if (m_Drift > 0.0f && handbrake > 0.0f && forwardSpeed > 10.0f)
            {
                driftScale = forwardSpeed;
                if (forwardSpeed < 25.0f)
                    driftScale = forwardSpeed * forwardSpeed * 0.04f;

                driftScale *= groundRatio * steer * handbrake;

                // Compute axis
                if (wheel_RR)
                {
                    axis.Midpoint(wheel_RL->m_ContactMatrix.GetRow(1), wheel_RR->m_ContactMatrix.GetRow(1));
                    axis.Normalize();
                }
                else
                {
                    axis = wheel_RL->m_ContactMatrix.GetRow(1);
                }

                // Compute contact point
                if (wheel_RR)
                     comPoint.Midpoint(wheel_RL->m_ContactMatrix.GetRow(3), wheel_RR->m_ContactMatrix.GetRow(3));
                else comPoint = wheel_RL->m_ContactMatrix.GetRow(3);

                // Apply torque
                float torque = -(driftScale * ics->m_AngInertia.Y * m_Drift);
                torqueVec = axis * torque;

                forceOffset = comPoint - worldPos;

                ApplyScaledTorqueAndForce(torqueVec, forceOffset, ics->m_InvAngInertia.Y);
            }
        }
        else
        {
            // Burnout turning torque
            bool isRWD = m_CarSim->m_DrivetrainType == 1;

            burnoutScale = pow(m_CarSim->m_Engine->m_CurrentRPS / m_CarSim->m_Engine->m_OptRPS, 0.5f)
                * steer
                * m_BurnoutTurn
                * burnout;

            vehWheel* wA = isRWD ? m_CarSim->m_Wheels[1] : m_CarSim->m_Wheels[0];
            vehWheel* wB = isRWD ? m_CarSim->m_Wheels[3] : m_CarSim->m_Wheels[2];

            if (!isRWD) burnoutScale *= 2.0f;

            comPoint.Midpoint(wA->m_ContactMatrix.GetRow(1),
                wB->m_ContactMatrix.GetRow(1));

            axis.Midpoint(wA->m_ContactMatrix.GetRow(3),
                wB->m_ContactMatrix.GetRow(3));

            float burnoutTorque = -(burnoutScale * ics->m_AngInertia.Y * m_Drift);

            torqueVec = comPoint * burnoutTorque;

            forceOffset = axis - worldPos;

            ApplyScaledTorqueAndForce(torqueVec, forceOffset, ics->m_InvAngInertia.Y);
        }
    }

    // Gravity scale / Airtime
    float gravityScale = collider->m_SomeInstParent->m_Archetype->sub_47B9D0();
    float wheelieParam = gravityScale * -9.8f;

    if (wheelsOnGround == 0.0f)
    {
        m_Airtime += datTimeManager::Seconds;
    }
    else if (m_Airtime > 0.0f)
    {
        m_Airtime = 0.0f;
        *(uint16_t*)&dword_4C = 30;
    }

    // Drift thrust (boost / force)
    if ((dword_18 & 0x20000) != 0 && m_DriftThrust > 0.0f)
    {
        driftScale = (handbrake > 0.0f && burnout <= 0.0f) ? handbrake : 0.0f;

        if (driftScale <= dword_44)
        {
            if (driftScale < dword_44)
                math::Approach(dword_44, driftScale, m_DriftDecay, datTimeManager::Seconds);
        }
        else
        {
            dword_44 = driftScale;
        }

        if (dword_44 > 0.0f)
        {
            Vector3 velocityDir = ics->m_WorldVelocity;
            velocityDir.Normalize();

            float driftThrustScale = *(float*)&m_CarSim->dword_154 * m_DriftThrust * dword_44;
            float driftforceScale = groundRatio * driftThrustScale;

            float alignmentDot = fabs(velocityDir.Dot(ics->m_WorldTransform.GetRow(0)));

            float alignmentClamped = math::Clamp(alignmentDot, 0.0f, 1.0f);

            float driftThrustForce =
                -(math::EaseOutSine(alignmentClamped) *
                driftforceScale *
                m_CarSim->m_Throttle *
                m_CarSim->m_Drivetrain->m_WheelBL->m_SurfaceFriction);

            Vector3 driftForce = ics->m_WorldTransform.GetRow(2) * driftThrustForce;

            ics->AccumulateForce(driftForce);
        }

        if (m_CarSim->m_NumWheels > 0)
        {
            float scale = 1.0f - driftScale * 0.5f;
            for (int i = 0; i < m_CarSim->m_NumWheels; ++i)
                m_CarSim->m_Wheels[i]->m_WheelDriveTorque *= scale;
        }
    }

    // 2-wheel mode / Leaning
    Matrix34 worldTransform;

    if ((m_CarSim->field_184 & 2) == 0 && (dword_18 & 0x80000) != 0 && m_CarSim->m_NumWheels == 4)
    {
        worldTransform.Set(ics->m_WorldTransform);

        if ((dword_4C & 0xFFFF) == 0 && wheelsOnGround != 0.0f && worldTransform.m11 > 0.8f)
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

                    if (leanDir == -1)
                    {
                        comPoint = m_CarSim->m_CenterOfMass;
                        comPoint.X = -(m_CarSim->m_Size.X * 0.5f * m_TwoWheelCOG);
                        m_CarSim->SetCenterOfMass(comPoint);
                        collider->ApplyLeanImpulse(m_LeanImpulseUp);
                    }
                    else if (leanDir == 1)
                    {
                        comPoint = m_CarSim->m_CenterOfMass;
                        comPoint.X = m_CarSim->m_Size.X * 0.5f * m_TwoWheelCOG;
                        m_CarSim->SetCenterOfMass(comPoint);
                        collider->ApplyLeanImpulse(-m_LeanImpulseUp);
                    }

                    m_CarSim->m_Wheels[0]->m_SteeringLimit = 0.0f;
                    m_CarSim->m_Wheels[2]->m_SteeringLimit = 0.0f;
                    *(uint16_t*)&dword_4C = 0x1E;
                }
            }
        }

    ////////////////////////////////////////////////////

    LABEL_65:
        leanTimer = this->dword_4C;
        if (leanTimer > 0)
            *(uint16_t*)&this->dword_4C = (uint16_t)(leanTimer - 1);

        tempLean = (int16_t)(this->dword_14 >> 16);
        if (!tempLean)
            goto LABEL_89;

        if ((this->m_Input->m_CurrentGearFlags & 0x10000) == 0 || tempLean == 0x63)
        {
            if (wheelsOnGround != 0.0)
            {
                if (tempLean == -1)
                    collider->ApplyLeanImpulse(-m_LeanImpulseDn);
                else
                    collider->ApplyLeanImpulse(m_LeanImpulseDn);
            }
            *(int16_t*)((char*)&this->dword_14 + 2) = 0;
            *(uint16_t*)&this->dword_4C = 0xA;
        }

        tempLean = (int16_t)(this->dword_14 >> 16);
        if (tempLean)
        {
            if (m_CarSim->m_Speed < (double)this->m_LeanSpeedMin)
                goto LABEL_82;

            if (*(uint16_t*)&this->dword_4C)
                goto LABEL_87;

            if (collider->m_Inst->m_WorldTransform.m11 < 0.0)
            {
            LABEL_82:
                if (wheelsOnGround != 0.0)
                {
                    if (tempLean == (__int16)0xFFFF)
                        collider->ApplyLeanImpulse(-m_LeanImpulseDn);
                    else
                        collider->ApplyLeanImpulse(m_LeanImpulseDn);
                }
                goto LABEL_86;
            }

            if (wheelsOnGround == 0.0 && this->m_Airtime > 1.0)
            {
            LABEL_86:
                *(int16_t*)((char*)&this->dword_14 + 2) = 0;
                *(uint16_t*)&this->dword_4C = 0xA;
            }
        }

    LABEL_87:
        if (!((int16_t)(this->dword_14 >> 16)))
        {
            comPoint = m_CarSim->m_CenterOfMass;
            comPoint.X = 0.0;
            m_CarSim->SetCenterOfMass(comPoint);
        }

    LABEL_89:
        currentLean = (int16_t)(this->dword_14 >> 16);
        if (currentLean && this->m_Lean > 0.0)
        {
            wheelZDiff = wheel_RL->m_LocalOffset.Z - wheel_FL->m_LocalOffset.Z;

            forceOffset = Vector3(0.0f, 0.0f, 0.0f);
            axis = Vector3(0.0f, 0.0f, 0.0f);

            if (currentLean >= 0)
            {
                axis.Midpoint(RRpos, FRpos);
                forceOffset.Subtract(RRpos, FRpos);
            }
            else
            {
                axis.Midpoint(RLpos, FLpos);
                forceOffset.Subtract(RLpos, FLpos);
            }

            wheelHeightDelta = (float)wheelZDiff;
            forceOffset = forceOffset / wheelHeightDelta;

            targetLean = asin(ics->m_WorldTransform.m01) -
                (double)(-(int16_t)(this->dword_14 >> 16)) *
                this->m_TwoWheelLeanAngle * 1.5704999;

            if (targetLean <= this->m_LeanLimit)
            {
                minLeanClamp = -this->m_LeanLimit;
                if (targetLean < minLeanClamp)
                    targetLean = minLeanClamp;
            }
            else
            {
                targetLean = this->m_LeanLimit;
            }

            clampedLean = -((targetLean * this->m_Lean
                - -(forceOffset.Z * ics->m_AngularVelocity.Z +
                    forceOffset.Y * ics->m_AngularVelocity.Y +
                    forceOffset.X * ics->m_AngularVelocity.X)
                * this->m_LeanDamp * datTimeManager::InvSeconds)
                * ics->m_AngInertia.Z);

            torqueVec.X = forceOffset.X * clampedLean;
            torqueVec.Y = forceOffset.Y * clampedLean;
            torqueVec.Z = clampedLean * forceOffset.Z;

            applyOffset.X = axis.X - ics->m_WorldTransform.m30;
            applyOffset.Y = axis.Y - ics->m_WorldTransform.m31;
            applyOffset.Z = axis.Z - ics->m_WorldTransform.m32;

            ApplyScaledTorqueAndForce(torqueVec, applyOffset, ics->m_InvAngInertia.Z);

            if (this->m_TwoWheelDrag != 0.0)
            {
                Vector3 velDir;
                velDir.Normalize(ics->m_WorldVelocity);
                ics->AccumulateForce(velDir * -m_TwoWheelDrag);
            }
        }
    }

    // Wheelie system
    if ((this->dword_18 & 0x100000) == 0)
        goto LABEL_118;

    if (m_CarSim->m_BurnoutValue <= 0.0 || this->m_Input->m_Handbrake != 0.0)
        goto LABEL_118;

    rearWheelOnGround = wheel_RL->m_OnGround;
    memset(&torqueVec, 0, sizeof(torqueVec));
    burnoutScale = 0.0;

    if (!rearWheelOnGround)
    {
        if (!wheel_RR || !wheel_RR->m_OnGround)
            goto LABEL_112;

        contactM01 = wheel_RR->m_ContactMatrix.m01;
        torqueVec.X = wheel_RR->m_ContactMatrix.m00;
        contactM02 = wheel_RR->m_ContactMatrix.m02;
        torqueVec.Y = contactM01;
        contactM21 = wheel_RR->m_ContactMatrix.m21;
        goto LABEL_111;
    }

    if (!wheel_RR || !wheel_RR->m_OnGround)
    {
        contactM01 = wheel_RL->m_ContactMatrix.m01;
        contactM02 = wheel_RL->m_ContactMatrix.m02;
        torqueVec.X = wheel_RL->m_ContactMatrix.m00;
        contactM21 = wheel_RL->m_ContactMatrix.m21;
        torqueVec.Y = contactM01;
    LABEL_111:
        torqueVec.Z = contactM02;
        burnoutScale = contactM21;
        goto LABEL_112;
    }

    torqueVec.Subtract(RRpos, RLpos);
    torqueVec.Normalize();
    burnoutScale = (m_CarSim->m_Wheels[3]->m_ContactMatrix.m21 + wheel_RL->m_ContactMatrix.m21) * 0.5f;

LABEL_112:
    gasBrakeInput = m_Input->m_GasBrake;
    comPoint.X = torqueVec.Y * ics->m_WorldTransform.m12 - torqueVec.Z * ics->m_WorldTransform.m11;
    comPoint.Y = torqueVec.Z * ics->m_WorldTransform.m10 - torqueVec.X * ics->m_WorldTransform.m12;
    comPoint.Z = torqueVec.X * ics->m_WorldTransform.m11 - torqueVec.Y * ics->m_WorldTransform.m10;

    wheelieGravityRef = -wheelieParam;
    wheelieParam = atan2(speed * speed, wheelieGravityRef) * gasBrakeInput * -0.79000002 * (m_Input->m_SteerRate + 1.0) * 0.5;

    ics->ComputeForce(0.0f, axis);
    double invMass = ics->m_InvMass;
    axis.X *= invMass;
    axis.Y *= invMass;
    axis.Z *= invMass;

    wheelieDot = axis.X * comPoint.X + axis.Z * comPoint.Z + axis.Y * comPoint.Y;
    wheelieAngleCorrection = 0.0;
    if (wheelieDot < 0.0)
        wheelieAngleCorrection = atan2(wheelieDot, wheelieGravityRef) * gasBrakeInput;

    Vector3 rlContact = m_CarSim->m_Wheels[1]->m_ContactMatrix.GetRow(3);

    wheelieBaseTorque = (ics->m_WorldTransform.m21 - burnoutScale - sin(wheelieAngleCorrection * 0.25 + wheelieParam * 0.75))
        * this->m_Wheelie
        - (torqueVec.Z * ics->m_AngularVelocity.Z +
            torqueVec.Y * ics->m_AngularVelocity.Y +
            torqueVec.X * ics->m_AngularVelocity.X)
        * datTimeManager::InvSeconds * 0.1;

    if (wheel_RR)
    {
        forceOffset.X = (wheel_RR->m_ContactMatrix.m30 + rlContact.X) * 0.5;
        forceOffset.Y = (wheel_RR->m_ContactMatrix.m31 + rlContact.Y) * 0.5;
        forceOffset.Z = (wheel_RR->m_ContactMatrix.m32 + rlContact.Z) * 0.5;
    }
    else
    {
        forceOffset = rlContact;
    }

    forceOffset.X -= ics->m_WorldTransform.m30;
    forceOffset.Y -= ics->m_WorldTransform.m31;
    forceOffset.Z -= ics->m_WorldTransform.m32;

    finalWheelieTorque = wheelieBaseTorque * ics->m_AngInertia.X;
    applyOffset.X = torqueVec.X * finalWheelieTorque;
    applyOffset.Y = torqueVec.Y * finalWheelieTorque;
    applyOffset.Z = torqueVec.Z * finalWheelieTorque;

    ApplyScaledTorqueAndForce(applyOffset, forceOffset, ics->m_InvAngInertia.X);

    // Turn / Spin / 180 Assist
LABEL_118:
    if (this->m_Turn <= 0.0)
        goto LABEL_148;

    comPoint.X = (wheel_RL->m_ContactMatrix.m30 + wheel_RR->m_ContactMatrix.m30) * 0.5;
    comPoint.Y = (wheel_RL->m_ContactMatrix.m31 + wheel_RR->m_ContactMatrix.m31) * 0.5;
    comPoint.Z = (wheel_RL->m_ContactMatrix.m32 + wheel_RR->m_ContactMatrix.m32) * 0.5;

    torqueVec.X = (wheel_RR->m_ContactMatrix.m10 + wheel_RL->m_ContactMatrix.m10) * 0.5;
    torqueVec.Y = (wheel_RL->m_ContactMatrix.m11 + wheel_RR->m_ContactMatrix.m11) * 0.5;
    torqueVec.Z = (wheel_RL->m_ContactMatrix.m12 + wheel_RR->m_ContactMatrix.m12) * 0.5;
    torqueVec.Normalize();

    spinAssistFlag = this->dword_18 & 0x10000;
    applyOffset.X = comPoint.X - ics->m_WorldTransform.m30;
    applyOffset.Y = comPoint.Y - ics->m_WorldTransform.m31;
    applyOffset.Z = comPoint.Z - ics->m_WorldTransform.m32;

    forceOffset = Vector3(0.0f, 0.0f, 0.0f);

    if (spinAssistFlag && onGroundFactorFinal > 0.0)
    {
        float someSteer = m_CarSim->m_Steer;
        if (absForwardSpeed < 25.0)
            someSteer *= absForwardSpeed * 0.04f;

        onGroundFactorFinal = (float)m_CarSim->OnGround();
        spinTorque = groundRatio * someSteer * (ics->m_AngInertia.Y * this->m_Spin180);
    }
    else if (spinAssistFlag && handbrake > 0.0)
    {
        if (m_CarSim->m_Transmission->m_CurrentGear || forwardSpeed >= -5.0)
            goto LABEL_147;

        reverseSpinScale = -m_CarSim->m_Steer;
        if (absForwardSpeed < 10.0)
            reverseSpinScale = absForwardSpeed * reverseSpinScale * 0.1;

        onGroundFactorFinal = (float)m_CarSim->OnGround();
        spinTorque = groundRatio * reverseSpinScale * (ics->m_AngInertia.Y * this->m_Reverse180);
    }
    else
    {
        if (this->m_TurnFactor > 0.0)
        {
            lowSpeedTurnScale = 1.0;
            if (absForwardSpeed < 5.0)
                lowSpeedTurnScale = absForwardSpeed * 0.2;

            double angularComponent = torqueVec.X * ics->m_AngularVelocity.X +
                torqueVec.Y * ics->m_AngularVelocity.Y +
                torqueVec.Z * ics->m_AngularVelocity.Z;

            vehWheels* wheelsStruct = m_CarSim->m_WheelsStruct;
            wheelieParam = -1.0
                / (wheelsStruct->m_Wheel_FR.m_LocalOffset.Z - wheelsStruct->m_Wheel_FL.m_LocalOffset.Z)
                * ((1.0 - wheelsStruct->m_Wheel_FL.m_OptimumSlipPercent)
                    * m_CarSim->m_Steer
                    * wheelsStruct->m_Wheel_FL.m_SteeringLimit
                    * forwardSpeed)
                * this->m_TurnFactor
                + (1.0 - this->m_TurnFactor) * angularComponent
                - angularComponent;

            onGroundFactorFinal = (float)m_CarSim->OnGround();
            finalTurnTorque = (double)(int32_t)onGroundFactorFinal
                / (double)m_CarSim->m_NumWheels
                * lowSpeedTurnScale
                * this->m_CarSim->m_WheelsStruct->m_Wheel_FL.m_SurfaceFriction
                * ics->m_AngInertia.Y
                * wheelieParam
                * this->m_Turn;
            goto LABEL_146;
        }

        if (forwardSpeed <= 0.0)
        {
            directionSign = (forwardSpeed >= 0.0) ? 0.0 : -1.0;
        }
        else
        {
            directionSign = 1.0;
        }

        handbrake = directionSign * m_CarSim->m_Steer;
        lowSpeedFactor = fabs(forwardSpeed);
        lowSpeedAbs = (float)lowSpeedFactor;

        if (lowSpeedFactor > 0.0)
        {
            if (lowSpeedAbs < 5.0)
                handbrake = lowSpeedAbs * handbrake * 0.2;
        }
        else
        {
            handbrake = 0.0;
        }

        onGroundFactorFinal = (float)m_CarSim->OnGround();
        spinTorque = (double)(int32_t)onGroundFactorFinal
            / (double)m_CarSim->m_NumWheels
            * handbrake
            * this->m_CarSim->m_WheelsStruct->m_Wheel_FL.m_SurfaceFriction
            * ics->m_AngInertia.Y
            * this->m_Turn;
    }

    finalTurnTorque = -spinTorque;

LABEL_146:
    forceOffset.X = torqueVec.X * finalTurnTorque;
    forceOffset.Y = torqueVec.Y * finalTurnTorque;
    forceOffset.Z = torqueVec.Z * finalTurnTorque;

LABEL_147:
    ApplyScaledTorqueAndForce(forceOffset, applyOffset, ics->m_InvAngInertia.Y);

LABEL_148:
    if ((dword_6957C0 & 0x1000) != 0)
        airControlThreshold = 0.0;
    else
        airControlThreshold = 0.25;

    if ((m_CarSim->field_184 & 1) == 0 && (this->dword_18 & 0x40000) != 0)
    {
        if ((m_Input->m_CurrentGearFlags & 0x10000) != 0
            && m_CarSim->m_Airtime > airControlThreshold
            && (*(float*)&this->dword_2C > 0.0 || this->m_Pitch > 0.0 || this->m_RollTorque > 0.0))
        {
            negLeanUpB = -(m_Input->m_2WheelSteer * *(float*)&this->dword_2C);
            ics->ApplyUpTorque(negLeanUpB);

            negLeanUpC = this->m_Input->m_SteerRate * this->m_Pitch;
            ics->ApplyLateralTorque(negLeanUpC);

            ph_Some_Inst_Parent* instParent = this->m_CarSim->m_Collider->m_SomeInstParent;
            rollLimitNeg = -this->m_RollLimit;
            currentRollAngle = asin(instParent->m_SomeInstParentTransform.m01);

            negLeanUpD = -(math::Clamp(currentRollAngle, rollLimitNeg, this->m_RollLimit) * this->m_RollTorque
                - -(instParent->m_SomeInstParentTransform.m22 * ics->m_AngularVelocity.Z
                    + instParent->m_SomeInstParentTransform.m21 * ics->m_AngularVelocity.Y
                    + instParent->m_SomeInstParentTransform.m20 * ics->m_AngularVelocity.X)
                * this->m_RollDamp
                * datTimeManager::InvSeconds);

            ics->ApplyForwardTorque(negLeanUpD);
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
