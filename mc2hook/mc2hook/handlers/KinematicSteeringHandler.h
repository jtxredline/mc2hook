#pragma once
#include <mc2hook\mc2hook.h>

class Vector3;
class Matrix34;

class KinematicSteeringHandler {
public:
    void Update();
    static void Install();

private:
    static float sm_SteeringRate;
    static float sm_SteeringLock;
    static float sm_SpeedEnd;
    static float sm_SCurveSharpness;
    static float sm_MaxSteeringFactor;
    static float sm_MinSteeringFactor;
    static float sm_GyroGain;
    static float sm_SlipAngleGain;

    static float sm_Range;
    static float sm_SpeedEndHalf;
    static float sm_InvSpeedEnd;

    static float sm_SteerValue;

    static void LoadConfig();
    static void PreCalculateConstants();
    bool IsLocalPlayerInput();

    static float ComputeNormalizedSpeed(float velocityMagnitude);
    static float ComputeGyroCorrection(float angularVelocityY, float deltaTime);
    static float ComputeSlipCorrection(float lateralVelocity, float longitudinalVelocity, float deltaTime, float normalizedSpeed);
    static float ComputeSpeedFactor(float normalizedSpeed);
    static Vector3 ComputeAxleVelocity(const Vector3& worldVelocity, const Vector3& angularVelocity, const Matrix34& worldTransform, float axleOffsetX);
    static float ClampSteering(float value);
};