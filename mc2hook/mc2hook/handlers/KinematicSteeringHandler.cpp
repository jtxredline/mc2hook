#include "KinematicSteeringHandler.h"
#include <age/data/timemgr.h>
#include <age/vector/vector3.h>
#include <age/vector/matrix34.h>
#include <cmath>
#include <age/vehicle/automgr.h>
#include <age/vehicle/carsim.h>
#include <age/physics/phcollider.h>
#include <age/physics/phinertia.h>
#include <age/managers/netmanager.h>
#include <age/input/keyboard.h>
#include <dinput.h>

float KinematicSteeringHandler::sm_SteeringRate = 0.0f;
float KinematicSteeringHandler::sm_SteeringLock = 0.0f;
float KinematicSteeringHandler::sm_SpeedEnd = 0.0f;
float KinematicSteeringHandler::sm_SCurveSharpness = 0.0f;
float KinematicSteeringHandler::sm_MaxSteeringFactor = 0.0f;
float KinematicSteeringHandler::sm_MinSteeringFactor = 0.0f;
float KinematicSteeringHandler::sm_GyroGain = 0.0f;
float KinematicSteeringHandler::sm_SlipAngleGain = 0.0f;
float KinematicSteeringHandler::sm_Range = 0.0f;
float KinematicSteeringHandler::sm_SpeedEndHalf = 0.0f;
float KinematicSteeringHandler::sm_InvSpeedEnd = 0.0f;
float KinematicSteeringHandler::sm_SteerValue = 0.0f;

void KinematicSteeringHandler::LoadConfig() {
    sm_SteeringRate = HookConfig::GetFloat("KinematicSteer", "SteeringRate", 8.0f);
    sm_SteeringLock = HookConfig::GetFloat("KinematicSteer", "SteeringLock", 1.0f);
    sm_SpeedEnd = HookConfig::GetFloat("KinematicSteer", "SpeedEnd", 100.0f);
    sm_SCurveSharpness = HookConfig::GetFloat("KinematicSteer", "SCurveSharpness", 0.1f);
    sm_MaxSteeringFactor = HookConfig::GetFloat("KinematicSteer", "MaxSteeringFactor", 1.0f);
    sm_MinSteeringFactor = HookConfig::GetFloat("KinematicSteer", "MinSteeringFactor", 0.89f);
    sm_GyroGain = HookConfig::GetFloat("KinematicSteer", "GyroGain", 1.0f);
    sm_SlipAngleGain = HookConfig::GetFloat("KinematicSteer", "SlipAngleGain", 2.0f);
}

void KinematicSteeringHandler::PreCalculateConstants() {
    sm_Range = sm_MaxSteeringFactor - sm_MinSteeringFactor;
    sm_SpeedEndHalf = sm_SpeedEnd * 0.5f;
    sm_InvSpeedEnd = 1.0f / sm_SpeedEnd;
}

bool KinematicSteeringHandler::IsLocalPlayerInput() {
    if (!mcNetManager::IsNetworkMode) return true;
    short thisInputPlayerIndex = *getPtr<short>(this, 0x0E);
    return thisInputPlayerIndex == mcNetManager::LocalPlayerID;
}

float KinematicSteeringHandler::ComputeNormalizedSpeed(float velocityMagnitude) {
    return fminf(velocityMagnitude * sm_InvSpeedEnd, 1.0f);
}

float KinematicSteeringHandler::ComputeGyroCorrection(float angularVelocityY, float deltaTime) {
    return (-angularVelocityY * deltaTime) * sm_GyroGain;
}

float KinematicSteeringHandler::ComputeSlipCorrection(float lateralVelocity, float longitudinalVelocity, float deltaTime, float normalizedSpeed) {
    float slipAngle = (fabsf(lateralVelocity) > 0.1f) ? atan2f(lateralVelocity, -longitudinalVelocity) : 0.0f;
    return -slipAngle * sm_SlipAngleGain * deltaTime * normalizedSpeed;
}

float KinematicSteeringHandler::ComputeSpeedFactor(float normalizedSpeed) {
    float expArg = -sm_SCurveSharpness * (normalizedSpeed * sm_SpeedEnd - sm_SpeedEndHalf);
    return fmaxf(sm_MinSteeringFactor, fminf(sm_MaxSteeringFactor - sm_Range / (1.0f + expf(expArg)), sm_MaxSteeringFactor));
}

float KinematicSteeringHandler::ClampSteering(float value) {
    return fminf(fmaxf(value, -sm_SteeringLock), sm_SteeringLock);
}

Vector3 KinematicSteeringHandler::ComputeAxleVelocity(const Vector3& worldVelocity, const Vector3& angularVelocity, const Matrix34& worldTransform, float axleOffsetX) {
    Vector3 xAxis = worldTransform.GetRow(0);
    Vector3 axleOffsetWorld = xAxis * axleOffsetX;
    Vector3 rotVel = Vector3::Cross(angularVelocity, axleOffsetWorld);
    return worldVelocity + rotVel;
}

void KinematicSteeringHandler::Update() {
    hook::Thunk<0x46B330>::Call<void>(this);
    if (!IsLocalPlayerInput()) return;

    float lastRequestedSteering = *getPtr<float>(this, 0x1C);
    float deltaTime = datTimeManager::GetSeconds();
    vehCarSim* vehicle = *getPtr<vehCarSim*>(this, 0x2C);
    if (!vehicle || !vehicle->collider || !vehicle->collider->ics) return;

    phInertialCS* ics = vehicle->collider->ics;
    Vector3 worldVel = ics->world_velocity;
    Matrix34 worldTrans = ics->world_transform;
    Vector3 angVel = ics->angular_velocity;

    float frontAxleX = ((Vector3*)((char*)vehicle->wheels[0] + 0x114))->X;
    Vector3 axleVel = ComputeAxleVelocity(worldVel, angVel, worldTrans, frontAxleX);

    float lateralVel = axleVel.Dot(worldTrans.GetRow(0));
    float longitudinalVel = axleVel.Dot(worldTrans.GetRow(2));

    float normalizedSpeed = ComputeNormalizedSpeed(worldVel.Mag());
    float gyroCorr = ComputeGyroCorrection(angVel.Y, deltaTime);
    float slipCorr = ComputeSlipCorrection(lateralVel, longitudinalVel, deltaTime, normalizedSpeed);

    float directionSign = (longitudinalVel >= 0.0f) ? -1.0f : 1.0f;
    float correctionDelta = (slipCorr + gyroCorr) * directionSign;
    float driverDelta = lastRequestedSteering * sm_SteeringRate * deltaTime;

    float speedFactor = ComputeSpeedFactor(normalizedSpeed);
    float fpsCorrectionPower = deltaTime * 144.0f;
    float normalizedDampening = powf(speedFactor, fpsCorrectionPower);

    // Apply the logic: (Current + Input) * Dampening
    sm_SteerValue = ClampSteering((sm_SteerValue + driverDelta - correctionDelta) * normalizedDampening);

    *getPtr<float>(this, 0x1C) = sm_SteerValue;
    vehicle->steer = sm_SteerValue;
}

void KinematicSteeringHandler::Install() {
    if (!HookConfig::GetBool("KinematicSteer", "KinematicSteerEnable", false)) return;
    LoadConfig();
    PreCalculateConstants();
    InstallVTableHook("Input Update", &Update, { 0x63D0C4 });
    mem::write(0x46B28E + 3, HookConfig::GetFloat("Input", "ControllerSteerSpeed", 10.0f));
}