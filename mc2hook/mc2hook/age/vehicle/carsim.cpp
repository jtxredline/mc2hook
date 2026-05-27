#include <mc2hook\mc2hook.h>
#include "carsim.h"
#include <age/math/math.h>

#include <age/vehicle/transmission.h>
#include <age/vehicle/aero.h>
#include <age/vehicle/wheel.h>
#include <age/physics/phcollider.h>
#include <age/vehicle/nitro.h>
#include <age/vehicle/carSSTurbo.h>
#include <age/vehicle/damage.h>
#include <age/physics/archetype.h>

#include <age/core/output.h> //

// WIP
/*
void vehCarSim::UpdateControls()
{
    vehDrivetrain* m_Drivetrain; // eax
    int dword_1b8; // eax
    phCollider* m_Collider; // eax
    vehNitro* m_Nitro; // ecx
    mcCarSSTurbo* m_SSTurbo; // ecx
    int dword_1c8; // eax
    phCollider* v9; // ecx
    double v10; // st7
    float m_Brake; // edi
    vehWheel* m_WheelFR; // ecx
    vehWheel* m_WheelRR; // ecx
    vehWheel* dword_fc; // ecx
    vehWheel* dword_100; // ecx
    vehTransmission* m_Transmission; // eax
    int m_CurrentGear; // ecx
    double v18; // st7
    int v19; // edi
    int v20; // ebp
    float v21; // ebx
    __int16 v22; // ax
    double v23; // st7
    float Y; // eax
    phArchetype* m_Archetype; // ecx
    int v26; // edi
    double v27; // st7
    int v28; // ebp
    float v29; // ecx
    double v30; // st7
    phArchetype* v31; // ecx
    int v32; // edi
    double v33; // st7
    int v34; // ebp
    float v35; // edx
    phArchetype* v36; // ecx
    int v37; // edi
    double v38; // st7
    int v39; // ebp
    float v40; // ecx
    float dword_1a4; // edx
    int v42; // edi
    int v43; // ebp
    vehTransmission* v44; // ecx
    int v46; // edi
    int v47; // ebp
    int v48; // edi
    int v49; // ebp
    int v50; // eax
    double v51; // st7
    int v52; // edi
    int v53; // ebp
    double m_BurnoutIncreaseSpeed; // st7
    double v55; // st7
    int m_DrivetrainType; // eax
    double Z; // st7
    int v58; // edi
    int v59; // ebp
    vehDamage* m_Damage; // ecx
    double v61; // st7
    int v62; // edi
    double m_BurnoutValue; // st7
    float v64; // edx
    float v65; // eax
    int v66; // ebp
    double v67; // st7
    vehDrivetrain* v68; // ecx
    double v69; // st6
    int m_NumWheels; // eax
    double v71; // st5
    int v72; // edx
    vehWheel** p_m_WheelRL; // ecx
    vehWheel* v74; // eax
    long double v75; // st5
    phInertialCS* m_ICS; // eax
    double v77; // st5
    double v78; // st7
    double m_Mass; // st6
    double v80; // st7
    int v81; // eax
    int v82; // eax
    float m_Steer; // [esp+0h] [ebp-38h]
    float v84; // [esp+0h] [ebp-38h]
    float v85; // [esp+0h] [ebp-38h]
    float v86; // [esp+0h] [ebp-38h]
    float steering; // [esp+14h] [ebp-24h]
    float brake; // [esp+18h] [ebp-20h]
    float brakea; // [esp+18h] [ebp-20h]
    float brakeb; // [esp+18h] [ebp-20h]
    float v91; // [esp+1Ch] [ebp-1Ch]
    float v92; // [esp+1Ch] [ebp-1Ch]
    float m_Throttle; // [esp+20h] [ebp-18h]
    int v94; // [esp+24h] [ebp-14h]
    float load_in_kilograms; // [esp+28h] [ebp-10h]
    float load_in_kilogramsa; // [esp+28h] [ebp-10h]
    float load_in_kilogramsb; // [esp+28h] [ebp-10h]
    float load_in_kilogramsc; // [esp+28h] [ebp-10h]
    float load_in_kilogramsd; // [esp+28h] [ebp-10h]
    float load_in_kilogramse; // [esp+28h] [ebp-10h]
    Vector3 a2a; // [esp+2Ch] [ebp-Ch] BYREF

    vehWheel* wheelFL = m_Wheels[0];
    vehWheel* wheelRL = m_Wheels[1];
    vehWheel* wheelFR = m_Wheels[2];
    vehWheel* wheelRR = m_Wheels[3];

    v94 = OnGround(); //((int(__usercall*)@<eax>(vehCarSim * @<ecx>, double@<st0>))this->vtable->vehCarSim_OnGround)(this, a2);                                  // int vehCarSim_OnGround;
    m_Drivetrain = this->m_Drivetrain;
    if (m_Drivetrain->m_NumWheels)
        this->dword_154 = -(m_Drivetrain->m_WheelRL->m_Radius * m_Drivetrain->m_SomeRPS);
    dword_1b8 = this->dword_1b8;
    if (dword_1b8)
    {
        switch (dword_1b8)
        {
        case 1:
            if ((this->field_184 & 4) != 0 || this->m_Throttle <= 0.0 || this->m_Handbrake <= 0.0)
            {
                this->m_Transmission->SetNeutral();
                this->m_Brake = 1.0;
            }
            else
            {
                this->m_Transmission->SetForward();
                this->m_Handbrake = 1.0;
            }
            if (this->m_NumWheels != 2)
                this->m_Steer = 0.0;
            break;
        case 2:
            this->m_Brake = 1.0;
            this->m_Throttle = 0.0;
            this->m_Steer = 0.0;
            goto LABEL_15;
        case 3:
            m_Steer = this->m_Steer;
            this->m_Brake = 1.0;
            this->m_Throttle = 0.0;
            this->m_Steer = math::Sign(m_Steer);
            this->m_Handbrake = 1.0;
            break;
        case 4:
            m_Collider = this->m_Collider;
            this->m_Brake = 0.5;
            this->m_Throttle = 0.0;
            this->m_Steer = math::Clamp(m_Collider->m_ICS->m_AngularVelocity.Z, -1.0, 1.0);
        LABEL_15:
            this->m_Handbrake = 0.0;
            break;
        default:
            break;
        }
        m_Nitro = this->m_Nitro;
        if (m_Nitro)
            m_Nitro->sub_4D1EE0();
        m_SSTurbo = this->m_SSTurbo;
        if (m_SSTurbo)
            m_SSTurbo->sub_4D41F0();
        if (this->dword_1b8 != 1)
        {
            this->m_BurnoutValue = 0.0;
            this->m_BurnoutCharged = 0;
            this->m_BurnoutCharging = 0;
        }
    }
    else
    {
        dword_1c8 = this->dword_1c8;
        if ((dword_1c8 & 1) != 0)
        {
            this->m_Brake = 0.0;
            this->m_Handbrake = 0.0;
        }
        else if ((dword_1c8 & 2) != 0)
        {
            this->m_Brake = 1.0;
            this->m_Handbrake = 1.0;
        }
        if ((dword_1c8 & 8) != 0)
        {
            this->m_Throttle = 1.0;
        }
        else if ((dword_1c8 & 4) != 0)
        {
            this->m_Throttle = 0.0;
        }
        if ((dword_1c8 & 0x10) != 0)
        {
            this->m_Steer = -1.0;
        }
        else if ((dword_1c8 & 0x20) != 0)
        {
            this->m_Steer = 1.0;
        }
        else if ((dword_1c8 & 0x40) != 0)
        {
            this->m_Steer = -this->m_Steer;
        }
    }
    if (LOBYTE(this->m_Nitro->dword_40))
    {
        this->m_Throttle = 1.0;
    }
    else if (this->m_Transmission->m_CurrentGear <= 1u
        && (this->m_Engine->m_MaxRPM + this->m_Engine->m_OptRPM) * 0.5 < this->m_Engine->m_CurrentRPM)
    {
        this->m_Throttle = 0.1;
    }
    v9 = this->m_Collider;
    v84 = fabs(
        v9->m_SomeInstParent->m_SomeInstParentTransform.m22 * v9->m_ICS->m_WorldVelocity.Z
        + v9->m_SomeInstParent->m_SomeInstParentTransform.m21 * v9->m_ICS->m_WorldVelocity.Y
        + v9->m_ICS->m_WorldVelocity.X * v9->m_SomeInstParent->m_SomeInstParentTransform.m20);
    v10 = sub_4D2860(v84);
    m_Throttle = this->m_Throttle;
    steering = v10 * this->m_Steer;
    brake = this->m_Brake;
    if (this->m_NumWheels == 2 && this->m_BurnoutValue > 0.0 && this->m_Handbrake > 0.0)
        steering = steering * 0.1;
    m_Brake = this->m_Brake;
    wheelFL->SetInputs(steering, brake, 0.0);
    m_WheelFR = wheelFR;
    if (m_WheelFR)
        m_WheelFR->SetInputs(steering, brake, 0.0);
    brakea = this->m_Handbrake;
    if (this->m_NumWheels != 2)
    {
        v91 = v10;
        brakea = brakea * v91;
    }
    v92 = -steering;
    wheelRL->SetInputs(v92, m_Brake, brakea);
    m_WheelRR = wheelRR;
    if (m_WheelRR)
        m_WheelRR->SetInputs(v92, m_Brake, brakea);

    dword_fc = (vehWheel*)this->dword_fc;   // ?
    if (dword_fc)
        dword_fc->SetInputs(v92, m_Brake, brakea);
    dword_100 = (vehWheel*)this->dword_100;
    if (dword_100)
        dword_100->SetInputs(v92, m_Brake, brakea);

    m_Transmission = this->m_Transmission;
    if (m_Transmission->m_GearChangeFlag)
    {
        this->m_Engine->StopBoost();
        this->m_Transmission->m_Clutch = 1;
    }
    else
    {
        m_CurrentGear = m_Transmission->m_CurrentGear;
        if (m_CurrentGear == 1)
        {
            m_Transmission->m_Clutch = 1;
        }
        else if (this->m_NumWheels != 2 || m_CurrentGear)
        {
            if (m_Transmission->m_Clutch)
                this->m_Engine->StartBoost(0.0);
            this->m_Transmission->m_Clutch = 0;
        }
        else
        {
            m_Transmission->m_Clutch = 1;
        }
    }
    if (this->m_Throttle > 0.0099999998 || this->m_Speed >= 6.7049999)
    {
        if (this->m_Throttle > 0.0099999998)
            this->m_SomeBrake = 0.0;
    }
    else
    {
        if (this->m_SomeBrake == 0.0)
            this->m_SomeBrake = 0.25;
        v18 = this->m_SomeBrake + 0.050000001;
        this->m_SomeBrake = v18;
        if (v18 > 1.0)
            this->m_SomeBrake = 1.0;
    }
    if (this->m_SomeBrake > 0.0)
    {
        for (int i = 0; i < m_NumWheels; i++)
            m_Wheels[i]->SetBrake(this->m_SomeBrake);

        this->m_Transmission->m_Clutch = 1;
    }
    v21 = *(float*)(this->m_Damage->dword_10 + 0x64);
    v22 = *(uint16_t*)(LODWORD(v21) + 0x14);
    if (v22 == 1)
    {
        if (this->m_CenterOfMass.Z == this->dword_1a4)
        {
            v23 = wheelRL->m_LocalOffset.Z * 0.5;
            Y = this->m_CenterOfMass.Y;
            a2a.X = this->m_CenterOfMass.X;
            a2a.Z = v23;
            a2a.Y = Y;
            SetCenterOfMass(a2a);
            m_Archetype = this->m_Collider->m_SomeInstParent->m_Archetype;
            load_in_kilograms = 2.0 / (double)this->m_NumWheels;
            v26 = 0;
            v27 = ((double(__thiscall*)(phArchetype*)) * (uint32_t*)(m_Archetype->dword_00 + 0x2C))(m_Archetype)
                * this->m_Mass
                * 9.8000002;
            if (this->m_NumWheels > 0)
            {
                load_in_kilogramsa = v27 * load_in_kilograms;

                for (int i = 0; i < m_NumWheels; i++)
                    m_Wheels[i]->SetNormalLoad(load_in_kilogramsa);
            }
        }
    }
    else if (v22 == (__int16)0xFFFF && this->m_CenterOfMass.Z == this->dword_1a4)
    {
        v29 = this->m_CenterOfMass.Y;
        v30 = wheelFL->m_LocalOffset.Z * 0.5;
        a2a.X = this->m_CenterOfMass.X;
        a2a.Y = v29;
        a2a.Z = v30;
        SetCenterOfMass(a2a);
        v31 = this->m_Collider->m_SomeInstParent->m_Archetype;
        load_in_kilogramsb = 2.0 / (double)this->m_NumWheels;
        v32 = 0;
        v33 = ((double(__thiscall*)(phArchetype*)) * (uint32_t*)(v31->dword_00 + 0x2C))(v31) * this->m_Mass * 9.8000002;
        if (this->m_NumWheels > 0)
        {
            load_in_kilogramsc = v33 * load_in_kilogramsb;

            for (int i = 0; i < m_NumWheels; i++)
                m_Wheels[i]->SetNormalLoad(load_in_kilogramsc);
        }
    }
    if (*(uint16_t*)(LODWORD(v21) + 0x16) && this->m_CenterOfMass.Z == this->dword_1a4)
    {
        v35 = this->m_CenterOfMass.Y;
        a2a.X = this->m_CenterOfMass.X;
        a2a.Y = v35;
        a2a.Z = 0.0;
        SetCenterOfMass(a2a);
        v36 = this->m_Collider->m_SomeInstParent->m_Archetype;
        load_in_kilogramsd = 2.0 / (double)this->m_NumWheels;
        v37 = 0;
        v38 = ((double(__thiscall*)(phArchetype*)) * (uint32_t*)(v36->dword_00 + 0x2C))(v36) * this->m_Mass * 9.8000002;
        if (this->m_NumWheels > 0)
        {
            load_in_kilogramse = v38 * load_in_kilogramsd;

            for (int i = 0; i < m_NumWheels; i++)
                m_Wheels[i]->SetNormalLoad(load_in_kilogramse);
        }
        LODWORD(this->m_Aero->dword_04) = this->dword_164;
    }
    if (!*(uint16_t*)(LODWORD(v21) + 0x14)
        && !*(uint16_t*)(LODWORD(v21) + 0x16)
        && this->m_CenterOfMass.Z != this->dword_1a4)
    {
        v40 = this->m_CenterOfMass.Y;
        dword_1a4 = this->dword_1a4;
        a2a.X = this->m_CenterOfMass.X;
        a2a.Y = v40;
        a2a.Z = dword_1a4;
        SetCenterOfMass(a2a);
        v42 = 0;
        if (this->m_NumWheels > 0)
        {
            for (int i = 0; i < m_NumWheels; i++)
                m_Wheels[i]->ComputeConstants();
        }
    }
    if (this->m_NumWheels == 4)
    {
        if (*(uint16_t*)(LODWORD(v21) + 0x16))
        {
            wheelFL->m_SteeringLimit = *(float*)(LODWORD(v21) + 0x84);
            wheelFR->m_SteeringLimit = *(float*)(LODWORD(v21) + 0x84);
        }
        else
        {
            wheelFL->m_SteeringLimit = this->m_SteeringLimit;
            wheelFR->m_SteeringLimit = this->m_SteeringLimit;
        }
    }
    if ((this->field_184 & 4) != 0
        || *(uint16_t*)(LODWORD(v21) + 0x14)
        || *(uint16_t*)(LODWORD(v21) + 0x16)
        || m_Throttle <= 0.0
        || brakea <= 0.0
        || (v44 = this->m_Transmission, v44->m_CurrentGear <= 0)
        || this->m_Speed >= (double)this->dword_188
        || v94 <= 0
        || this->m_BurnoutCharged == 2)
    {
        v62 = 0;
        if (!this->m_Transmission->m_CurrentGear)
            this->m_BurnoutValue = 0.0;
        m_BurnoutValue = this->m_BurnoutValue; // burnout timer
        this->m_BurnoutCharging = 0;
        this->m_BurnoutCharged = 0;
        if (m_BurnoutValue > 0.0)
        {
            v64 = this->m_CenterOfMass.Y;
            v65 = this->dword_1a4;
            a2a.X = this->m_CenterOfMass.X;
            a2a.Y = v64;
            a2a.Z = v65;
            SetCenterOfMass(a2a);
            if (this->m_NumWheels > 0)
            {
                for (int i = 0; i < m_NumWheels; i++)
                    m_Wheels[i]->ComputeConstants();
            }

            if (v94 > 0 && !this->dword_1b8)
            {
                v67 = 0.0;
                v68 = this->m_Drivetrain;
                v69 = 0.0;
                m_NumWheels = v68->m_NumWheels;
                v71 = 0.0;
                a2a.Z = 0.0;
                if (m_NumWheels > 0)
                {
                    v72 = this->m_Drivetrain->m_NumWheels;
                    p_m_WheelRL = &v68->m_WheelRL;
                    do
                    {
                        v74 = *p_m_WheelRL;
                        if ((*p_m_WheelRL)->m_OnGround)
                        {
                            v67 = v67 - v74->m_ContactMatrix.m20;
                            v69 = v69 - v74->m_ContactMatrix.m21;
                            v71 = v71 - v74->m_ContactMatrix.m22;
                        }
                        ++p_m_WheelRL;
                        --v72;
                    } while (v72);
                    a2a.Z = v71;
                }
                v75 = v71 * v71 + v69 * v69 + v67 * v67;
                if (v75 == 0.0)
                    brakeb = 0.0;
                else
                    brakeb = 1.0 / sqrt(v75);
                m_ICS = this->m_Collider->m_ICS;
                a2a.X = v67 * brakeb;
                v77 = m_Throttle * this->m_BurnoutBoostSpeed;
                a2a.X = a2a.X * v77;
                a2a.Y = v69 * brakeb * v77;
                v78 = v77 * (a2a.Z * brakeb);
                if (m_ICS)
                {
                    m_Mass = m_ICS->m_Mass;
                    m_ICS->m_Force.X = a2a.X * m_Mass + m_ICS->m_Force.X;
                    m_ICS->m_Force.Y = a2a.Y * m_Mass + m_ICS->m_Force.Y;
                    m_ICS->m_Force.Z = v78 * m_Mass + m_ICS->m_Force.Z;
                }
            }
            v80 = this->m_BurnoutValue - this->m_BurnoutDecreaseSpeed;
            this->m_BurnoutValue = v80;
            if (v80 < 0.0)
                this->m_BurnoutValue = 0.0;
        }
    }
    else
    {
        v44->SetCurrentGear(2);
        if (this->m_Drivetrain->m_NumWheels == this->m_NumWheels)
        {
            this->m_Transmission->m_Clutch = 1;
            this->m_Transmission->SetGearChangeFlag(1);
        }
        else
        {
            //m_Freetrain = this->m_Freetrain;
            if (m_Freetrain)
            {
                v46 = 0;
                if (*(int*)(m_Freetrain + 0x10) > 0)
                {
                    v47 = 0x14;
                    do
                    {
                        *(vehWheel**)(this->m_Freetrain + v47)->SetBrake(2.0); // TODO: Map freetrain at least a bit
                        ++v46;
                        v47 += 4;
                    } while (v46 < *(uint32_t*)(this->m_Freetrain + 0x10));
                }
            }
            v48 = 0;
            if (this->m_Drivetrain->m_NumWheels > 0)
            {
                v49 = 0x14;
                do
                {
                    vehWheel::SetBrake(*(vehWheel**)((char*)&this->m_Drivetrain->vtable + v49), 0.0); // Drivetrain as well
                    ++v48;
                    v49 += 4;
                } while (v48 < this->m_Drivetrain->m_NumWheels);
            }
        }
        if (this->m_BurnoutValue >= 1.0 || this->m_BurnoutCharged)
        {
            m_DrivetrainType = this->m_DrivetrainType;
            a2a.X = this->m_CenterOfMass.X;
            if (!m_DrivetrainType || m_DrivetrainType == 2)
                Z = wheelFL->m_LocalOffset.Z;
            else
                Z = wheelRL->m_LocalOffset.Z;
            a2a.Z = Z * 0.5;
            a2a.Y = 0.0;
            SetCenterOfMass(a2a);
            v58 = 0;
            if (this->m_NumWheels > 0)
            {
                for (int i = 0; i < m_NumWheels; i++)
                    m_Wheels[i]->ComputeConstants();
            }
            m_Damage = this->m_Damage;
            v61 = this->m_BurnoutDamageAmount * this->m_BurnoutValue;
            this->m_BurnoutCharging = 1;
            this->m_BurnoutCharged = 1;

            v86 = v61 * 100.0;
            (*(void(__stdcall**)(uint32_t))(m_Damage->m_Vtable + 0x2C))(LODWORD(v86)); // replace
        }
        else
        {
            v50 = this->m_DrivetrainType;
            a2a.X = this->m_CenterOfMass.X;
            if (!v50 || v50 == 2)
                v51 = wheelFL->m_LocalOffset.Z;
            else
                v51 = wheelRL->m_LocalOffset.Z;
            a2a.Z = v51 * 0.75;
            a2a.Y = 0.0;
            vehCarSim::SetCenterOfMass(this, &a2a);
            v52 = 0;
            if (this->m_NumWheels > 0)
            {
                v53 = 0;
                do
                {
                    vehWheel::ComputeConstants((vehWheel*)((char*)&this->m_WheelsStruct->m_Wheel_FL + v53));
                    ++v52;
                    v53 += 0x1D4;
                } while (v52 < this->m_NumWheels);
            }

            m_BurnoutIncreaseSpeed = this->m_BurnoutIncreaseSpeed;
            this->m_BurnoutCharging = 1;
            v55 = m_BurnoutIncreaseSpeed + this->m_BurnoutValue;
            this->m_BurnoutValue = v55;
            if (v55 > 1.0)
                this->m_BurnoutValue = 1.0;

            if ((this->m_Collider->m_SomeInstParent->m_Archetype->dword_0C & 0x800) != 0)
            {
                v85 = this->m_BurnoutDamageAmount * this->m_BurnoutValue * 100.0;
                (*(void(__stdcall**)(uint32_t))(this->m_Damage->m_Vtable + 0x2C))(LODWORD(v85));
            }
        }
    }
    this->m_Engine->m_ThrottleValue = m_Throttle;
    if (this->dword_158 != 0.0 && this->m_NumWheels == 4)
    {
        if (fabs(this->m_Collider->m_SomeInstParent->m_SomeInstParentTransform.m01) <= 0.69999999)
            this->m_Engine->m_HPScale = 1.0;
        else
            this->m_Engine->m_HPScale = this->dword_158 + 1.0;
    }
    if (this->dword_168 < 1.0)
    {
        v81 = this->dword_1b8;
        if (v81 == 4 || v81 == 2)
            this->m_Aero->dword_04 = this->dword_168;
    }
    if (this->m_Aero->dword_04 < 1.0 || (v82 = this->dword_1b8, v82 == 4) || v82 == 2)
    {
        vehCarSim::SetFrictionHandling(this, this->dword_15c);
        this->m_Steer = steering;
    }
    else
    {
        if (SLOBYTE(this->dword_1c8) >= 0)
            vehCarSim::SetFrictionHandling(this, this->dword_160);
        else
            vehCarSim::SetFrictionHandling(this, 10.0);
        this->m_Steer = steering;
    }
}
*/

int vehCarSim::OnGround()
{
    //return hook::Thunk<0x56A000>::Call<int>(this); // Call original

    int numWheelsOnGround = 0;

    for (int i = 0; i < m_NumWheels; i++)
    {
        if (m_Wheels[i]->m_OnGround)
            numWheelsOnGround++;
    }
    return numWheelsOnGround;
}

int vehCarSim::BottomedOut()
{
    int numWheelsBottomedOut = 0;

    for (int i = 0; i < m_NumWheels; i++)
    {
        if (m_Wheels[i]->m_BottomedOut)
            numWheelsBottomedOut++;
    }
    return numWheelsBottomedOut;
}

void vehCarSim::SetDrivable(int a2)
{
    hook::Thunk<0x4D2A50>::Call<void>(this, a2); // Call original
}

void vehCarSim::SetCenterOfMass(const Vector3& cg)
{
    this->m_CenterOfMass = cg;
}

void vehCarSim::SetFrictionHandling(float friction)
{
    for (int i = 0; i < m_NumWheels; i++)
    {
        m_Wheels[i]->m_FrictionHandling = friction;
    }
}

float vehCarSim::sub_4D2860(float a2) // Compute something
{
    return hook::Thunk<0x4D2860>::Call<float>(this, a2); // Call original
}

// WIP
/*
void vehCarSim::UpdateControlsComp()
{
    static bool toggle = false;

    if (m_Transmission->m_CallOriginal) // Debug flag
    {
        hook::Thunk<0x4D3290>::Call<void>(this); // Call original

        if (toggle == false)
        {
            Printf("vehCarSim::UpdateControls : CALLING ORIGINAL\n");
            toggle = !toggle;
        }
    }
    else if (m_Transmission->m_CallOriginal == false)
    {
        UpdateControls();

        if (toggle == true)
        {
            Printf("vehCarSim::UpdateControls : CALLING REWRITE\n");
            toggle = !toggle;
        }
    }
}
*/