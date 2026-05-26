#include "playercamera.h"

#include <age/data/timemgr.h>
#include <age/math/math.h>
#include <age/core/output.h>
#include <age/physics/phinertia.h>
#include <age/physics/phcollider.h>
#include <age/vehicle/carSSTurbo.h>
#include <age/vehicle/nitro.h>
#include <age/vehicle/aero.h>
#include <age/vehicle/transmission.h>
#include <age/vehicle/automgr.h>
#include <age/managers/netmanager.h>

#include <age/input/keyboard.h>
#include <dinput.h>

void camTrackCS::UpdateSS()
{
    //hook::Thunk<0x46F140>::Call<void>(this); // Call original

    // Spectator mode    
    static bool isSpectating = false;
    static int currentPlayerID;
    static int switchSpectator = HookConfig::GetInt("Input Setup", "SwitchSpectator", DIK_F2);

    if (ioKeyboard::GetKeyDown(switchSpectator))
    {
        if (!isSpectating)
        {
            // Toggle ON
            Printf("Spectator ON\n");
            isSpectating = true;

            // Start ID
            if (mcNetManager::IsNetworkMode == false) currentPlayerID = vehAutoMgr::Instance->m_NumSims - 1;
            else currentPlayerID = mcNetManager::LocalPlayerID;

            // Start ID should not be local player
            currentPlayerID++;
            if (currentPlayerID >= vehAutoMgr::Instance->m_NumSims) currentPlayerID = 0;

            // Set sim and inst to this ID
            m_CarSim = vehAutoMgr::Instance->m_Sim[currentPlayerID];
            m_Instance = vehAutoMgr::Instance->m_Sim[currentPlayerID]->m_Collider->m_Inst;
        }
        else
        {
            // Next spectator
            Printf("Next spectator\n");
            currentPlayerID++;
            if (currentPlayerID >= vehAutoMgr::Instance->m_NumSims) currentPlayerID = 0;
            m_CarSim = vehAutoMgr::Instance->m_Sim[currentPlayerID];
            m_Instance = vehAutoMgr::Instance->m_Sim[currentPlayerID]->m_Collider->m_Inst;
        }
    }

    // Rest of UpdateSS
    float v8, v12;
    float speed;

    speed = m_CarSim->m_Speed;
    if (!m_CarSim->m_Transmission->m_CurrentGear)
        speed = 0.0;

    m_SomeSpeed = (1.0 - m_SSFilter) * m_SomeSpeed + speed * m_SSFilter;
    if (m_SomeSpeed > m_SSSpeedMin)
    {
        if (m_SomeSpeed < m_SSSpeedMax)
            v12 = (m_SomeSpeed - m_SSSpeedMin) / (m_SSSpeedMax - m_SSSpeedMin);
        else
            v12 = 1.0;
    }
    else
    {
        v12 = 0.0;
    }

    float v4 = math::EaseIn(v12);
    float fov = m_SSFOVMin + (m_SSFOVMax - m_SSFOVMin) * v4;

    if (m_CarSim->m_SSTurbo)
        fov += m_CarSim->m_SSTurbo->dword_3c;

    if (m_CarSim->m_Nitro)
        fov += m_CarSim->m_Nitro->dword_2c;

    m_CurrentFOV = fov;

    v8 = v4 * (m_SSDistScale - 1.0) + 1.0;
    m_SomeSSDistMin = m_SSDistMin * v8;
    m_SomeSSDistMax = m_SSDistMax * v8;

    /*if (speed >= 22.351999)
        dword_210 = dword_210 & 0xFFFDFFFF;
    else
        dword_210 = dword_210 | 0x20000;*/ // Byte stuff

    camTrackCS::Update();

    //Printf("X: %f\r", vehcarsim->collider->ics->world_transform.GetRow(3).X);
}

void camTrackCS::Update()
{
    //hook::Thunk<0x57B660>::Call<void>(this); // Call original function

    camTrackCS::UpdateCar();
    camTrackCS::UpdateHill();
    camTrackCS::UpdateTrack();
    camTrackCS::PreApproach();

    camTrackCS::ApproachIt();
    
    //hook::Thunk<0x579C00>::Call<void>(this);

    if (dword_bc)
        dword_bc = 0;
    
    if (dword_104)
    {
        m_GlobalMatrix = m_SomeGlobalMatrixCopy;
        dword_104 = 0;
    }

    /*// Address stuff
    if (ioKeyboard::GetKeyDown(DIK_LALT))
    {
        void* pt = getPtr<void*>(this, 0x0);
        Printf("camTrackCS: %p\n", pt);
    }*/
}

void camTrackCS::UpdateCar()
{
    //hook::Thunk<0x57A080>::Call<void>(this); // Call original

    m_Steer = m_CarSim->m_Steer;
    m_Speed = m_CarSim->m_Speed;

    if (m_ReverseOn == 1)
    {
        if (m_CarSim->m_Transmission->m_CurrentGear)
        {
            if (this->field_1d0)
            {
                m_SomeDir_1d8 = (m_Steer <= 0.1f) ? -1.0f : 1.0f; // Some dir

                float& angle = *reinterpret_cast<float*>(dword_178 + 8);
                if (fabs(angle) > math::PI)
                {
                    angle = m_SomeDir_1d8 * math::PI;
                    this->field_1d0 = 0;
                    this->dword_1d4 = 0;

                    if ((m_Instance->m_WorldTransform.m11 < 0.35f && m_TrackBreak == 1) || (m_TrackBreak == 2))
                    {
                        this->dword_184 = 0;
                        this->dword_188 = 1;
                        return;
                    }
                }

                this->field_1d0 = 0;
            }
            this->dword_1d4 = 0;
        }
        else
        {
            if (m_CarSim->m_Handbrake > 0.5)
                m_SomeMaxAppXZPos = 0.0;

            if (!this->field_1d0 && m_CarSim->m_Throttle >= 0.05)
            {
                this->dword_1d4 += datTimeManager::GetSeconds();

                if (dword_1d4 >= m_RevDelay)
                {
                    this->field_1d0 = 1;
                    m_SomeDir_1d8 = (m_Steer <= 0.1f) ? 1.0f : -1.0f;
                }
            }
        }
    }
    else
    {
        this->field_1d0 = m_ReverseOn == -1;
    }

    if ((m_Instance->m_WorldTransform.m11 < 0.35f && m_TrackBreak == 1) || (m_TrackBreak == 2))
    {
        this->dword_184 = 0;
        this->dword_188 = 1;
        return;
    }

    phInertialCS* ics = m_CarSim->m_Collider->m_ICS;
    float angMomSq = pow(ics->m_AngularMomentum.X, 2) + pow(ics->m_AngularMomentum.Y, 2) + pow(ics->m_AngularMomentum.Z, 2);

    if (m_CarSim->OnGround() <= m_CarSim->m_NumWheels * 0.51)
    {
        this->dword_17c += datTimeManager::GetSeconds(); // Some airtime?
        if (dword_17c > 0.1)
        {
            this->dword_184 = 0;
            m_SomeTimer = 0.0;
        }
    }
    else
    {
        m_SomeTimer += datTimeManager::GetSeconds();
        if (m_SomeTimer > 0.1)
        {
            this->dword_184 = 1;
            this->dword_17c = 0.0;
        }
    }
    if (angMomSq > 2250000.0 && !this->dword_184)
    {
        this->dword_188 = 1;
        return;
    }

    this->dword_188 = 0;
}

void camTrackCS::UpdateHill()
{
    //hook::Thunk<0x57A2E0>::Call<void>(this); // Call original function

    float v3, v7, v8;
    int numWheelsOnGround;

    if (m_HillLerp > 0.0 && m_HillMax > m_HillMin)
    {
        v3 = 0.0;
        numWheelsOnGround = 0;
        if (m_CarSim->m_NumWheels > 0)
        {
            for (int i = 0; i < m_CarSim->m_NumWheels; i++)
            {
                if (m_CarSim->m_Wheels[i]->m_OnGround)
                {
                    v3 += m_CarSim->m_Wheels[i]->m_ContactMatrix.m21;
                    numWheelsOnGround++;
                }
            }

            if (numWheelsOnGround)
                v3 /= numWheelsOnGround;
        }
        if (this->field_1d0 || m_Offset.Z < 0.0)
            v3 *= -1.0;
        
        if (v3 >= 0.0)
        {
            v8 = v3 * m_HillMax;
            if (v8 > 0.9)
                v8 = 0.9;
            m_Hill = asin(v8) * m_HillLerp + (1.0 - m_HillLerp) * m_Hill;
        }
        else
        {
            v7 = v3 * m_HillMin;
            if (v7 > 0.9)
                v7 = 0.9;
            m_Hill = (1.0 - m_HillLerp) * m_Hill - asin(v7) * m_HillLerp;
        }
    }
}

void camTrackCS::UpdateTrack()
{
    //hook::Thunk<0x57AD60>::Call<void>(this); // Call original function

    float z, x, v51, v52, v11, v12, v15, v16, v17;
    //float unk1, unk2, v23, v24, v40, v26;
    Vector3 v48, v47;

    if (m_Instance)
    {
        m_LookAbove = m_VertOffset * m_Offset.Y;
        z = m_TrackTo.Z;

        if (field_1d0)
        {
            z = -z;
            x = -m_TrackTo.X;
        }
        else
        {
            x = m_TrackTo.X;
        }
        
        m_SomeLookAtPos.X = x;
        m_SomeLookAtPos.Y = -m_CarSim->m_ModelOffset.Y;
        m_SomeLookAtPos.Z = z;

        m_SomeLookAtPos.Dot(m_Instance->m_WorldTransform);

        m_SomeLookAtPos.Y += m_CarSim->m_ModelOffset.Y + m_TrackTo.Y + m_LookAbove;
        dword_18c = 0;
        m_SomePos_5.Set(m_SomeLookAtPos);

        static Vector3 upDir = Vector3(0.0, 1.0, 0.0);

        v48.X = upDir.Y * m_Instance->m_WorldTransform.m22 - upDir.Z * m_Instance->m_WorldTransform.m21;
        v48.Y = upDir.Z * m_Instance->m_WorldTransform.m20 - upDir.X * m_Instance->m_WorldTransform.m22;
        v48.Z = upDir.X * m_Instance->m_WorldTransform.m21 - upDir.Y * m_Instance->m_WorldTransform.m20;
        v48.Normalize();

        v51 = v48.Y * upDir.X - v48.X * upDir.Y;
        v52 = (v48.Z * upDir.Y - v48.Y * upDir.Z) * m_Offset.Z;
        v11 = (v48.X * upDir.Z - v48.Z * upDir.Z) * m_Offset.Z;

        v12 = v51 * m_Offset.Z;
        v15 = v48.X * m_Offset.X + m_SomeLookAtPos.X;
        v16 = v48.Y * m_Offset.X + m_SomeLookAtPos.Y;
        v17 = v48.Z * m_Offset.X + m_SomeLookAtPos.Z;

        v48.X = v15 - v52;
        v48.Y = v16 - v11;
        v48.Z = v17 - v12;

        if (m_ApproachOn && (!dword_184 && dword_188 || dword_188 == 2))
        {
            dword_188 = 1;
            dword_18c = 1;
        }

        m_SomePos_3.X = v48.X;
        m_SomePos_3.Y = m_Offset.Y + v48.Y - m_LookAbove;
        m_SomePos_3.Z = v48.Z;

        m_SomePos_4 = m_SomePos_3;

        m_SomeGlobalMatrixCopy.LookAt(m_SomePos_3, m_SomeLookAtPos);
        //global_matrix.LookAt(some_pos_3, some_lookat_pos);

        /*//unk1 = *(float*)(dword_178 + 4); // ??
        //unk2 = *(float*)(dword_178 + 8);

        if (reverse_on == 1)
        {
            if (this->field_1d0)
            {
                v40 = this->dword_1d8 * math::PI;
                math::Approach(unk2, v40, this->rev_on_app, datTimeManager::GetSeconds());
            }
            else
            {
                math::Approach(unk2, 0.0, this->rev_off_app, datTimeManager::GetSeconds());
            }
            v23 = fabs(unk2);
            if (v23 > 0.0)
                v24 = v23 < math::PI ? v23 * 0.31830987 : 1.0;
            else
                v24 = 0.0;
            v26 = v24 * (-this->hill - this->hill) + this->hill + *(float*)(dword_178 + 0xC);
            *(float*)(dword_178 + 8) = unk2;

            Printf("REVERSE_ON == 1\r");
        }
        else if (reverse_on == 0xFFFFFFFF)
        {
            unk2 = math::PI;
            if (!this->field_1d0)
                unk2 = 0.0;
            v26 = math::InverseLerp(fabs(unk2), 0.0, math::PI) * (-this->hill - hill) + hill + *(float*)(dword_178 + 0xC);
            *(float*)(dword_178 + 8) = unk2;

            Printf("REVERSE_ON == 0xFFFFFFFF\r");
        }
        else
        {
            v26 = *(float*)(dword_178 + 0xC) + this->hill;
            Printf("REVERSE_ON == ELSE\r");
        }*/
        //v26 = hill; // Outcome of the above

        v48.X = upDir.Y * m_SomeGlobalMatrixCopy.m22 - upDir.Z * m_SomeGlobalMatrixCopy.m21;
        v48.Y = upDir.Z * m_SomeGlobalMatrixCopy.m20 - upDir.X * m_SomeGlobalMatrixCopy.m22;
        v48.Z = upDir.X * m_SomeGlobalMatrixCopy.m21 - upDir.Y * m_SomeGlobalMatrixCopy.m20;

        //Printf("unk1 = %f, unk2 = %f, v26 = %f, hill = %f\r", unk1, unk2, v26, hill);

        if (m_Hill != 0.0) //if (unk1 != 0.0 || unk2 != 0.0 || v26 != 0.0)
        {
            m_SomeGlobalMatrixCopy.SetRow(3, m_SomeGlobalMatrixCopy.GetRow(3) - m_SomeLookAtPos);

            /*if (v26 != 0.0)
            {
                this->some_global_matrix_copy.RotateFull(v48, -v26);
            }*/
            m_SomeGlobalMatrixCopy.RotateFull(v48, -m_Hill);

            //if (unk2 != 0.0)
            //    this->some_global_matrix_copy.RotateFull(some_up_dir, unk2);

            m_SomeGlobalMatrixCopy.SetRow(3, m_SomeGlobalMatrixCopy.GetRow(3) + m_SomeLookAtPos);

            /*if (unk1 != 0.0)
            {
                v47 = some_global_matrix_copy.GetRow(3) - some_lookat_pos;
                v47 = v47 * unk1;
                some_global_matrix_copy.SetRow(3, some_global_matrix_copy.GetRow(3) + v47);
            }*/
        }
    }
}

void camTrackCS::PreApproach()
{
    //hook::Thunk<0x57A430>::Call<void>(this); // Call original function

    //TODO: Clean up LOWORD!
    //some_max_app_xz_pos_low = LODWORD(this->some_max_app_xz_pos);
    //min_app_xz_pos = this->min_app_xz_pos;
    //current = some_max_app_xz_pos_low;
    
    float current = m_SomeMaxAppXZPos;
    float target, factor;

    if (!m_CarSim || !m_AppAppOn)
        return;
    
    if (m_MinAppXZPos <= 0.0)
    {
        target = m_MaxAppXZPos;
    }
    else
    {
        if (m_MaxSpeed == m_MinSpeed)
            factor = 0.0;
        else
            factor = math::InverseLerp(m_Speed, m_MinSpeed, m_MaxSpeed);

        target = (m_MinAppXZPos - m_MaxAppXZPos) * factor + m_MaxAppXZPos;
        if (dword_188 || dword_18c)
            target = m_MinAppXZPos;
    }

    if (m_CarSim->m_Aero->dword_04 < 1.0)
        target *= m_CarSim->m_Aero->dword_04;

    if (dword_1dc > 0)
        target = m_MaxAppXZPos;

    if (!m_ReverseOn && !m_CarSim->m_Transmission->m_CurrentGear)
        target = 20.0;

    if (target == current)
    {
        m_SomeMaxAppXZPos = current;
        return;
    }

    float step = (target > current) ? m_AppInc : m_AppDec;

    math::Approach(current, target, step, datTimeManager::GetSeconds());
    m_SomeMaxAppXZPos = current;
}

void camTrackCS::ApproachIt() // camAppCS
{
    //hook::Thunk<0x57DC00>::Call<void>(this);
    
    if (m_ApproachOn)
    {
        if (dword_bc)
            m_GlobalMatrix.Set(m_SomeGlobalMatrixCopy);
        else
            camTrackCS::UpdateApproach();
    }
    else
    {
        m_GlobalMatrix = m_SomeGlobalMatrixCopy;
    }
}

void camTrackCS::UpdateApproach()
{
    float v35, v36, localAppRot;
    Matrix34 v48, v51, v52;
    Vector3 a2, v42, v43, v47;
    float v49, v23, v50, invLookAt;
    float v38, v39, v40;

    Vector3 globalPos = m_GlobalMatrix.GetRow(3);
    v35 = datTimeManager::GetSeconds() * m_SomeMaxAppXZPos;
    camTrackCS::DApproach(m_GlobalMatrix.m30, m_SomeGlobalMatrixCopy.m30, m_AppPosMin, 0.0, dword_e4, v35);
    v36 = m_AppYPos * datTimeManager::GetSeconds();
    camTrackCS::DApproach(m_GlobalMatrix.m31, m_SomeGlobalMatrixCopy.m31, m_AppPosMin, 0.0, dword_e8, v36);
    camTrackCS::DApproach(m_GlobalMatrix.m32, m_SomeGlobalMatrixCopy.m32, m_AppPosMin, 0.0, dword_ec, v35);

    if (m_SomeSSDistMax != 0.0)
        camTrackCS::UpdateMaxDist();

    v48.Set(m_GlobalMatrix);
    v48.SetRow(3, Vector3(0.f, 0.f, 0.f));


    v51.Set(m_SomeGlobalMatrixCopy);
    v51.SetRow(3, Vector3(0.f, 0.f, 0.f));

    v48.GetEulers(a2, "zxy");
    v51.GetEulers(v42, "zxy");

    if (a2.X > math::HALF_PI && v42.X < -math::HALF_PI)
        v42.X = v42.X + math::PI2;
    if (a2.X < -math::HALF_PI && v42.X > math::HALF_PI)
        v42.X = v42.X - math::PI2;
    if (a2.Y > math::HALF_PI && v42.Y < -math::HALF_PI)
        v42.Y = v42.Y + math::PI2;
    if (a2.Y < -math::HALF_PI && v42.Y > math::HALF_PI)
        v42.Y = v42.Y - math::PI2;
    if (a2.Z > math::HALF_PI && v42.Z < -math::HALF_PI)
        v42.Z = v42.Z + math::PI2;
    if (a2.Z < -math::HALF_PI && v42.Z > math::HALF_PI)
        v42.Z = v42.Z - math::PI2;

    if (m_LookAt > 0.001)
    {
        v52.LookAt(globalPos, m_SomeLookAtPos);
        v52.SetRow(3, Vector3(0.0f, 0.0f, 0.0f));

        v52.GetEulers(v43, "zxy");

        if (a2.X > math::HALF_PI && v43.X < -math::HALF_PI)
            v43.X = v43.X + math::PI2;
        if (a2.X < -math::HALF_PI && v43.X > math::HALF_PI)
            v43.X = v43.X - math::PI2;
        if (a2.Y > math::HALF_PI && v43.Y < -math::HALF_PI)
            v43.Y = v43.Y + math::PI2;
        if (a2.Y < -math::HALF_PI && v43.Y > math::HALF_PI)
            v43.Y = v43.Y - math::PI2;
        if (a2.Z > math::HALF_PI && v43.Z < -math::HALF_PI)
            v43.Z = v43.Z + math::PI2;
        if (a2.Z < -math::HALF_PI && v43.Z > math::HALF_PI)
            v43.Z = v43.Z - math::PI2;

        v23 = m_LookAt * v43.X;
        v49 = m_LookAt * v43.Y;
        v50 = m_LookAt * v43.Z;

        invLookAt = 1.0 - m_LookAt;

        v43 = v42 * invLookAt;

        v47.X = v43.X + v23;
        v47.Y = v43.Y + v49;
        v47.Z = v43.Z + v50;

        v42 = v47;
    }
    
    if (m_AppXRot == 0.0)
        localAppRot = m_AppRot;
    else
        localAppRot = m_AppXRot;

    
    v38 = datTimeManager::GetSeconds() * m_AppRot;
    camTrackCS::DApproach(a2.Z, v42.Z, m_AppRotMin, 0.0, dword_e0, v38);
    v39 = localAppRot * datTimeManager::GetSeconds();
    camTrackCS::DApproach(a2.X, v42.X, m_AppRotMin, 0.0, dword_d8, v39);
    v40 = datTimeManager::GetSeconds() * m_AppRot;
    camTrackCS::DApproach(a2.Y, v42.Y, m_AppRotMin, 0.0, dword_dc, v40);

    v48.FromEulers(a2, "zxy");
    
    m_GlobalMatrix.SetRow(0, v48.GetRow(0));
    m_GlobalMatrix.SetRow(1, v48.GetRow(1));
    m_GlobalMatrix.SetRow(2, v48.GetRow(2));
}

void camTrackCS::DApproach(float& a2, float a3, float a4, float a5, float& a6, float a7)
{
    float v8, v13;
    
    v8 = fabs(a3 - a2);
    if (a4 != 0.0 && v8 <= a4)
        v8 = v8 * v8 / a4;
    if (a5 != 0.0 && v8 > a5)
        v8 = a5;
    if (m_AppAppOn)
    {
        v13 = (v8 - a6) * m_AppApp + a6;
        a6 = v13;
        if (v13 < 0.0 && v8 > 0.0)
        {
            v8 = 0.0;
            a6 = 0.0;
        }
        else if (v13 <= 0.0 || v8 >= 0.0)
        {
            v8 = v13;
        }
        else
        {
            v8 = 0.0;
            a6 = 0.0;
        }
    }

    math::Approach(a2, a3, v8, a7);
}

void camTrackCS::UpdateMaxDist()
{
    float distanceSq;
    Vector3 v16;

    if (m_SomeSSDistMax >= m_SomeSSDistMin)
    {
        if (m_GlobalMatrix.m30 != m_SomeLookAtPos.X
            || m_GlobalMatrix.m31 != m_SomeLookAtPos.Y
            || m_GlobalMatrix.m32 != m_SomeLookAtPos.Z)
        {
            distanceSq = math::DistanceSq(m_GlobalMatrix.GetRow(3), m_SomeLookAtPos);

            if (m_SomeSSDistMax * m_SomeSSDistMax < distanceSq)
            {
                v16 = m_GlobalMatrix.GetRow(3) - m_SomeLookAtPos;
                v16.Normalize();
                v16 = v16 * m_SomeSSDistMax + m_SomeLookAtPos;
                m_GlobalMatrix.SetRow(3, v16);
            }

            if (m_SomeSSDistMin * m_SomeSSDistMin > distanceSq)
            {
                v16 = m_GlobalMatrix.GetRow(3) - m_SomeLookAtPos;
                v16.Normalize();
                v16 = v16 * m_SomeSSDistMin + m_SomeLookAtPos;
                m_GlobalMatrix.SetRow(3, v16);
            }
        }
    }
}
