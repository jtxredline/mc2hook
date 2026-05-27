#include <mc2hook\mc2hook.h>
#include <age/data/timemgr.h>
#include "transmission.h"

#include <age/input/keyboard.h> //
#include <dinput.h> //

void vehTransmission::Update()
{
    //hook::Thunk<0x567E00>::Call<void>(this); // Call original

    if (m_Mode == TransmissionMode::Auto)
    {
        int gear = m_CurrentGear;

        if (gear > 1 && m_GearChangeTimer > m_GearChangeTime && m_GearChangeFlag == 0)
        {
            bool anyWheelSlipping = false;
            bool anyWheelOnGround = false;

            for (int i = 0; i < m_Drivetrain->m_NumWheels; i++)
            {
                vehWheel* wheel = m_Drivetrain->m_Wheels[i]; //&m_Drivetrain->m_WheelRL[i]; // m_WheelRL
                if (wheel->m_SlidingStrength < 0.9f)
                    anyWheelSlipping = true;
                if (wheel->m_OnGround)
                    anyWheelOnGround = true;
            }

            // Upshift
            if (gear < m_AutoNumGears - 1 && m_Engine->m_CurrentRPM > m_UpshiftRPMs[gear] && anyWheelOnGround && anyWheelSlipping && !m_Clutch)
            {
                SetCurrentGear(gear + 1);
                m_GearChangeTimer += datTimeManager::GetSeconds();
                return;
            }

            int remainingDownshifts = m_MaxDownshifts;
            if (m_CurrentGear > 2)
            {
                do
                {
                    if (remainingDownshifts-- <= 0)
                        break;

                    float rpmLimit = (m_Engine->m_ThrottleValue == 0.0f) ? m_MinDownshiftRPMs[m_CurrentGear] : m_MaxDownshiftRPMs[m_CurrentGear];

                    if (m_Engine->m_CurrentRPM >= rpmLimit)
                        break;
                    if (!anyWheelOnGround)
                        break;

                    SetCurrentGear(m_CurrentGear - 1);
                } while (m_CurrentGear > 2);
            }
        }
    }

    m_GearChangeTimer += datTimeManager::GetSeconds();

    // Debug flag
    if (ioKeyboard::GetKeyDown(DIK_LALT)) m_CallOriginal = !m_CallOriginal;
}

void vehTransmission::SetGearChangeFlag(int flag)
{    
    m_GearChangeFlag = flag;

    if (flag != 0)
        m_GearChangeTimer = 0.0f;
}

int vehTransmission::SetCurrentGear(int g)
{
    int prevGear = m_CurrentGear;
    if (g != prevGear && (m_Mode != TransmissionMode::Auto || g < m_AutoNumGears))
    {
        if (g > 2)
        {
            m_GearChangeTimer = 0.0;
            if (g > prevGear)
            {
                SetGearChangeFlag(1);
                m_CurrentGear = g;
                return g;
            }
            if (g < prevGear)
                SetGearChangeFlag(-1);
        }
        m_CurrentGear = g;
    }
    return m_CurrentGear;
}

void vehTransmission::SetNeutral()
{
    SetCurrentGear(1);
}

void vehTransmission::SetForward()
{
    if (m_CurrentGear <= 1)
        SetCurrentGear(2);
}

float vehTransmission::GetRatio() const
{   
    if (m_Mode == TransmissionMode::Auto)
        return m_AutoRatios[m_CurrentGear];
    else
        return m_ManualGearRatios[m_CurrentGear];
}

void vehTransmission::Upshift()
{
    //hook::Thunk<0x567D80>::Call<void>(this); // Call original
    
    bool canShift;

    if (m_Mode)
    {
        if (m_Mode != TransmissionMode::Auto)
            return;
        canShift = m_CurrentGear < 2;
    }
    else
    {
        canShift = m_CurrentGear < m_ManualNumGears - 1;
    }
    if (canShift)
        SetCurrentGear(m_CurrentGear + 1);
}

void vehTransmission::Downshift()
{
    //hook::Thunk<0x567DB0>::Call<void>(this); // Call original

    if (m_Mode)
    {
        if (m_Mode == TransmissionMode::Auto)
        {
            if (m_CurrentGear < 2)
            {
                if (m_CurrentGear >= 1)
                    SetCurrentGear(0);
            }
            else
            {
                SetCurrentGear(1);
            }
        }
    }
    else
    {
        if (m_CurrentGear > 0) // > 2 // No neutral/reverse gear fix
            SetCurrentGear(m_CurrentGear - 1);
    }
}
