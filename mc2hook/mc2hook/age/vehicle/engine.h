#pragma once
#include <age/vector/matrix34.h>
#include <age/vehicle/transmission.h>
#include <age/vehicle/drivetrain.h>
#include <age/physics/phinertia.h>
#include <age/physics/phinst.h>

class vehTransmission;
class vehDrivetrain;

class vehEngine
{
public:
    void* m_Vtable;
    float m_MaxHP;
    float m_IdleRPM;
    float m_OptRPM;
    float m_MaxRPM;
    float m_GCL;
    float m_HPScale;
    float dword_1c;
    float m_BoostDuration;
    float m_BoostHP;
    float m_GearChangeThrottle;
    float m_AngInertia;
    float m_MaxRPS;
    float m_OptRPS;
    float m_IdleRPS;
    char field_3C;
    char field_3D;
    char field_3E;
    char field_3F;
    float m_InvOptRPSDiffSq;
    float m_UnkComputed_1;
    float m_Fib;
    float m_InvFib;
    float m_GearChangeTimer;

    bool m_GearChangeActive = false;
    bool m_GearChangeForced = false;
    char m_GearChangePadding[2];

    float m_ThrottleValue;
    float m_BoostTimer;
    float m_CurrentRPS;
    float m_CurrentRPM;
    float unk_field_68;
    float m_Torque;
    float m_TorqueAtOptRPS;
    Matrix34* m_PivotTransform;
    Matrix34* m_InstanceTransform;
    float m_JiggleAngle;
    vehDrivetrain* m_Drivetrain;
    vehTransmission* m_Transmission;
    phInertialCS* m_ICS;
    phInst* m_Instance;

public:
    void Update();
    float CalcTorqueAtFullThrottle(float angVel) const;
    float CalcTorqueAtZeroThrottle() const;
    float GetMaxHP() const;
    void StopBoost();
};
//ASSERT_SIZEOF(vehEngine, 0x90);
