#pragma once
#include <age/vehicle/engine.h>
#include <age/vehicle/drivetrain.h>

class vehEngine;

class vehTransmission {
public:
    void* m_Vtable;
    vehEngine* m_Engine;
    vehDrivetrain* m_Drivetrain;
    int m_GearChangeFlag;
    float m_GearChangeTimer;
    int m_IsAutomatic;
    int m_CurrentGear;
    float m_ManualGearRatios[8];
    int m_ManualNumGears;
    int m_AutoNumGears;
    float m_AutoRatios[8];
    float m_UpshiftRPMs[8];
    float m_MaxDownshiftRPMs[8];
    float m_MinDownshiftRPMs[8];
    float m_GearChangeTime;
    float m_ReverseSpeed;
    float m_MaxSpeedLow;
    float m_MaxSpeedHigh;
    float m_UpshiftBias;
    float m_DownshiftBiasMin;
    float m_DownshiftBiasMax;
    int m_MaxDownshifts;
    float m_GearBias;
    bool m_Clutch;
    bool m_CallOriginal; // Debug flag
    char m_Padding[2];

public:
    void Update();
    void SetGearChangeFlag(int flag);
    void SetCurrentGear(int g);
    float GetRatio() const;
};
//ASSERT_SIZEOF(vehTransmission, 0xEC);
