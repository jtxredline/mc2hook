#pragma once
#include <mc2hook/mc2hook.h>
#include <age/memory/age_alloc_baseclass.h>
//#include <age/vector/matrix34.h>
//#include <veh_dyna/transmission.h>
//#include <veh_dyna/drivetrain.h>
//#include <age/physics/phinertia.h>
//#include <age/physics/phinst_old.h>

class Matrix34;
class vehDrivetrain;
class vehTransmission;
class phInertialCS;
class phInstOld;
class vehSim;

class vehEngine : public AGEAllocatedClass
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
    int dword_3c;
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
    phInstOld* m_Instance;

public:
    vehEngine()  { hook::Thunk<0x56FFF0>::Call<void>(this); }
    ~vehEngine() { hook::Thunk<0x56F810>::Call<void>(this); }

    void Init(vehSim* sim, const char* carName, const char* partName);
    void Update();
    float CalcTorqueAtFullThrottle(float angVel) const;
    float CalcTorqueAtZeroThrottle() const;
    float GetMaxHP() const;
    void StopBoost();
    void StartBoost(float boost);
};

static_assert(sizeof(vehEngine) == 0x90, "vehEngine size mismatch");
