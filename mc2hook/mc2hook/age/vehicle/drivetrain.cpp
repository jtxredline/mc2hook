#include "drivetrain.h"

#include <age/vehicle/transmission.h>
#include <age/data/timemgr.h>>

#include <age/core/output.h> //

void vehDrivetrain::Update() // TODO: Cleanup
{
    double v1; // st7
    int num_wheels_1; // ebx
    vehWheel** m_Wheels; // eax
    int num_wheels_2; // ecx
    double m_BrakeValue; // st6
    int num_wheels_3; // ecx
    vehWheel** v8; // eax
    double v9; // st6
    double v11; // st6
    int v12; // edi
    vehTransmission* transmission_1; // eax
    int current_gear_1; // ecx
    double v15; // st5
    int num_wheels_4; // ecx
    vehWheel** v17; // eax
    double v18; // st7
    double m_SomeRPS; // st6
    double v20; // st6
    long double v21; // st7
    vehTransmission* transmission_2; // ecx
    int current_gear_2; // eax
    double v24; // st7
    int current_gear_3; // eax
    double v26; // st7
    int current_gear_4; // eax
    double v28; // st6
    double v30; // st7
    long double v32; // st6
    int v33; // edi
    int v34; // eax
    double v35; // st6
    double v36; // st5
    vehWheel** v37; // ecx
    double v38; // st4
    double v39; // st6
    int v40; // eax
    double v41; // st6
    int v42; // ecx
    int num_wheels_5; // ebx
    float m_AngInertia; // st6
    int i; // eax
    int v46; // ebp
    unsigned int v47; // edx
    int v48; // ecx
    double v49; // st5
    double v50; // st7
    vehEngine* v51; // edx
    vehTransmission* transmission_3; // ecx
    int current_gear_5; // eax
    double v54; // st6
    double m_MaxRPS; // st7
    int current_gear_6; // eax
    double v57; // st6
    double m_OptRPS; // st6
    double v59; // st6
    double v60; // st6
    int v61; // ecx
    vehWheel** v62; // eax
    vehWheel* v63; // edx
    int num_wheels_6; // eax
    float a2; // [esp+10h] [ebp-60h]
    float v66; // [esp+14h] [ebp-5Ch]
    float v67; // [esp+14h] [ebp-5Ch]
    float v68; // [esp+18h] [ebp-58h]
    float v69; // [esp+18h] [ebp-58h]
    float v70; // [esp+18h] [ebp-58h]
    float v71; // [esp+1Ch] [ebp-54h]
    float dword_30; // [esp+1Ch] [ebp-54h]
    float v73; // [esp+1Ch] [ebp-54h]
    int v74; // [esp+20h] [ebp-50h]

    float v75[19]; // [esp+24h] [ebp-4Ch] BYREF // TODO: Check this

    //v75
    /*float local_50[8];
    float local_30[6];
    float local_18[6];*/

    v1 = 50.0;
    num_wheels_1 = this->m_NumWheels;
    if (this->m_SomeRPS == 0.0)
    {
        if (num_wheels_1 > 0)
        {
            m_Wheels = this->m_Wheels;
            num_wheels_2 = this->m_NumWheels;
            do
            {
                m_BrakeValue = (*m_Wheels++)->m_BrakeValue;
                --num_wheels_2;
                v1 = v1 + m_BrakeValue * this->m_BrakeStaticCoef;
            } while (num_wheels_2);
        }
    }
    else if (num_wheels_1 > 0)
    {
        num_wheels_3 = this->m_NumWheels;
        v8 = this->m_Wheels;
        do
        {
            v9 = (*v8++)->m_BrakeValue;
            --num_wheels_3;
            v1 = v1 + v9 * this->m_BrakeDynamicCoef;
        } while (num_wheels_3);
    }
    //m_Engine = this->m_Engine;
    v11 = 0.0;
    v12 = 0;
    if (m_Engine)
    {
        transmission_1 = this->m_Transmission;
        if (transmission_1)
        {
            if (!transmission_1->m_Clutch)
            {
                current_gear_1 = transmission_1->m_CurrentGear;
                if (transmission_1->m_Mode)
                    v15 = transmission_1->m_AutoRatios[current_gear_1];
                else
                    v15 = transmission_1->m_ManualGearRatios[current_gear_1];
                v11 = m_Engine->m_Torque * v15;
            }
        }
    }
    v68 = v11;
    v74 = 0;
    if (num_wheels_1 > 0)
    {
        num_wheels_4 = this->m_NumWheels;
        v17 = this->m_Wheels;
        do
        {
            v11 = v11 - (*v17++)->m_WheelDriveTorque;
            --num_wheels_4;
        } while (num_wheels_4);
        v68 = v11;
    }
    if (this->m_SomeRPS == 0.0)
    {
        if (v11 < 0.0)
        {
            v18 = v1 + v11;
            if (v18 <= 0.0)
            {
            LABEL_23:
                a2 = v18;
                if (0.0 == a2)
                    v74 = 1;
                goto LABEL_35;
            }
        }
        else
        {
            v18 = v11 - v1;
            if (v18 >= 0.0)
                goto LABEL_23;
        }
        a2 = 0.0;
        v74 = 1;
        goto LABEL_35;
    }
    m_SomeRPS = this->m_SomeRPS;
    if (m_SomeRPS <= 0.0)
    {
        if (m_SomeRPS >= 0.0)
            v20 = 0.0;
        else
            v20 = -1.0;
    }
    else
    {
        v20 = 1.0;
    }
    v21 = v1 * v20;
    if (fabs(v68) <= fabs(v21))
        v74 = 1;
    a2 = v68 + v21;
LABEL_35:
    if (m_Engine
        && (transmission_2 = this->m_Transmission) != 0
        && ((current_gear_2 = transmission_2->m_CurrentGear, !transmission_2->m_Mode)
            ? (v24 = transmission_2->m_ManualGearRatios[current_gear_2])
            : (v24 = transmission_2->m_AutoRatios[current_gear_2]),
            v24 != 0.0))
    {
        current_gear_3 = transmission_2->m_CurrentGear;
        if (transmission_2->m_Mode)
            v26 = transmission_2->m_AutoRatios[current_gear_3];
        else
            v26 = transmission_2->m_ManualGearRatios[current_gear_3];
        current_gear_4 = transmission_2->m_CurrentGear;
        if (transmission_2->m_Mode)
            v28 = transmission_2->m_AutoRatios[current_gear_4];
        else
            v28 = transmission_2->m_ManualGearRatios[current_gear_4];
        v75[0] = v28 * m_Engine->m_AngInertia * v26 + 0.02;
    }
    else
    {
        v75[0] = 10.0;
    }
    if (num_wheels_1 > 0)
    {
        /*v29 = this->m_Wheels;
        do
        {
            vehWheel::ComputeDwtdw(*v29, SLODWORD(a2), &v75[v12 + 7], &v75[v12 + 1], &v75[v12 + 0xD]);
            ++v12;
            ++v29;
        } while (v12 < this->m_NumWheels);*/

        for (int i = 0; i < m_NumWheels; ++i)
        {
            m_Wheels[i]->ComputeDwtdw(
                a2,
                &v75[i + 7],
                &v75[i + 1],
                &v75[i + 0xD]);
        }
    }
    v30 = -a2;
    v69 = 1.0 / this->dword_30;
    if (diffRatioMax > 1.0)
    {
        m_NumWheels = this->m_NumWheels;
        if (m_NumWheels <= 1 || this->m_SomeRPS < 0.001 && this->m_SomeRPS > -0.001)
        {
            this->dword_30 = 1.0;
            v69 = 1.0;
        }
        else
        {
            v32 = fabs(this->m_SomeRPS);
            if (v32 < diffRatioHighSpeedLevel)
                v70 = ((diffRatioHighSpeedLevel - v32) * diffRatioMax
                    + *(float*)&diffRatioMaxHighSpeed * v32)
                * dRHSLinv;
            else
                v70 = *(float*)&diffRatioMaxHighSpeed;
            v33 = m_NumWheels - 1;
            v34 = 0;
            v71 = 1.0 / v70;
            v35 = 0.0;
            v36 = 0.0;
            v66 = 0.0;
            if (m_NumWheels - 1 > 0)
            {
                v37 = &this->m_Wheels[1];
                do
                {
                    v38 = v37[0xFFFFFFFF]->m_WheelDriveTorque - (*v37)->m_WheelDriveTorque;
                    v34 += 2;
                    v37 += 2;
                    v35 = v35 + v38;
                    v36 = v36 + v75[v34 + 6] + v75[v34 + 5];
                } while (v34 < this->m_NumWheels - 1);
                v66 = v36;
            }
            v39 = v35 / ((v36 + this->m_AngInertia) * this->m_SomeRPS) + this->dword_30;
            if (v39 <= v70)
            {
                if (v39 < v71)
                    v39 = v71;
            }
            else
            {
                v39 = v70;
            }
            dword_30 = this->dword_30;
            v40 = 0;
            v41 = (v39 + dword_30 * 9.0) * 0.1;
            this->dword_30 = v41;
            v69 = 1.0 / v41;
            v30 = v30 - (dword_30 - v41) * v66 * this->m_SomeRPS;
            if (v33 > 0)
            {
                v42 = this->m_NumWheels - 1;
                do
                {
                    v40 += 2;
                    v75[v40 + 5] = v41 * v75[v40 + 5];
                    v75[v40 + 0xB] = v41 * v75[v40 + 0xB];
                    v75[v40 - 1] = v69 * v75[v40 - 1];
                    v75[v40 + 6] = v69 * v75[v40 + 6];
                    v75[v40 + 0xC] = v69 * v75[v40 + 0xC];
                    v75[v40] = v41 * v75[v40];
                } while (v40 < v42);
            }
        }
    }
    num_wheels_5 = this->m_NumWheels;
    m_AngInertia = this->m_AngInertia;
    for (i = 0; i < num_wheels_5; ++i)
        m_AngInertia = m_AngInertia + v75[i + 7];
    v46 = 0;
    v47 = 0xFFFFFFFF;
    v67 = 0.0;
    v73 = this->m_SomeRPS;
    if (num_wheels_5 >= 0)
    {
        do
        {
            ++v46;
            v48 = 0;
            v67 = datTimeManager::Seconds / (datTimeManager::Seconds * m_AngInertia + v75[0]) * v30 + v73;
            if (a2 >= 0.0)
            {
                v49 = -1.0e10;
                if (num_wheels_5 > 0)
                {
                    do
                    {
                        if (v49 < v75[v48 + 1])
                        {
                            v47 = v48;
                            v49 = v75[v48 + 1];
                        }
                        ++v48;
                    } while (v48 < this->m_NumWheels);
                }
                if (v67 >= v49)
                    break;
                v75[v47 + 1] = -9.9999998e10;
            }
            else
            {
                v49 = 1000000000.0;
                if (num_wheels_5 > 0)
                {
                    do
                    {
                        if (v49 > v75[v48 + 1])
                        {
                            v47 = v48;
                            v49 = v75[v48 + 1];
                        }
                        ++v48;
                    } while (v48 < this->m_NumWheels);
                }
                if (v67 <= v49)
                    break;
                v75[v47 + 1] = 9.9999998e10;
            }
            v30 = v30 - (v49 - v73) * m_AngInertia;
            v73 = v49;
            m_AngInertia = m_AngInertia + v75[v47 + 0xD] - v75[v47 + 7];
        } while (v46 <= this->m_NumWheels);
    }
    v50 = v67;
    if (v74 && (v67 < 0.0 && this->m_SomeRPS > 0.0 || this->m_SomeRPS < 0.0 && v67 > 0.0))
        v50 = 0.0;
    v51 = this->m_Engine;
    this->m_SomeRPS = v50;
    if (v51)
    {
        transmission_3 = this->m_Transmission;
        current_gear_5 = transmission_3->m_CurrentGear;
        if (transmission_3->m_Mode)
            v54 = transmission_3->m_AutoRatios[current_gear_5];
        else
            v54 = transmission_3->m_ManualGearRatios[current_gear_5];
        m_MaxRPS = -(v50 * v54);
        v75[0] = v51->m_IdleRPS * 0.5;
        if (m_MaxRPS >= v75[0])
        {
            if (m_MaxRPS <= v51->m_MaxRPS)
                goto LABEL_106;
            m_MaxRPS = v51->m_MaxRPS;
        }
        else
        {
            m_MaxRPS = v75[0];
        }
        current_gear_6 = transmission_3->m_CurrentGear;
        if (transmission_3->m_Mode)
            v57 = transmission_3->m_AutoRatios[current_gear_6];
        else
            v57 = transmission_3->m_ManualGearRatios[current_gear_6];
        this->m_SomeRPS = -(m_MaxRPS / v57);
    LABEL_106:
        m_OptRPS = v51->m_OptRPS;
        if (v51->m_CurrentRPS >= m_MaxRPS)
        {
            if (v51->m_CurrentRPS > m_MaxRPS)
            {
                v60 = v51->m_CurrentRPS - m_OptRPS * datTimeManager::Seconds;
                v51->m_CurrentRPS = v60;
                if (v60 < m_MaxRPS)
                    v51->m_CurrentRPS = m_MaxRPS;
            }
        }
        else
        {
            v59 = m_OptRPS * datTimeManager::Seconds + v51->m_CurrentRPS;
            v51->m_CurrentRPS = v59;
            if (v59 > m_MaxRPS)
                v51->m_CurrentRPS = m_MaxRPS;
        }
    }
    v61 = 0;
    if (this->m_NumWheels - 1 > 0)
    {
        v62 = &this->m_Wheels[1];
        do
        {
            v63 = v62[0xFFFFFFFF];
            v61 += 2;
            v62 += 2;
            v63->m_SomeAngularVelocity = this->dword_30 * this->m_SomeRPS;
            v62[0xFFFFFFFE]->m_SomeAngularVelocity = v69 * this->m_SomeRPS;
        } while (v61 < this->m_NumWheels - 1);
    }
    num_wheels_6 = this->m_NumWheels;
    if ((num_wheels_6 & 1) != 0)
        *(float*)(*(&this->m_NumWheels + num_wheels_6) + 0x1A4) = this->m_SomeRPS;
}

void vehDrivetrain::Detach()
{
	m_Engine = nullptr;
}

void vehDrivetrain::Attach()
{
	m_Engine = m_CarSim->m_Engine;
}
