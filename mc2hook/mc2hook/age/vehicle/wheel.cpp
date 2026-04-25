#include <mc2hook\mc2hook.h>
#include <age/data/timemgr.h>
#include <age/math/math.h>
#include <age/vehicle/carsim.h>
#include "wheel.h"

#include <age/core/output.h> //
#include <age/input/keyboard.h> //
#include <dinput.h> //

void vehWheel::UpdateSuspensionRay()
{
    //hook::Thunk<0x56D840>::Call<void>(this); // Call original
    
    Matrix34* pWorldTransform;
    float v14, v17, v18, rayUp, rayDown;
    Vector3 rayStart, rayEnd;

    if (m_CarSim)
        pWorldTransform = &m_CarSim->m_Collider->m_Inst->m_WorldTransform; // What transform exactly is this? TODO: Verify this (some_inst_parent)
    else
        pWorldTransform = &m_ICS->m_WorldTransform;

    if (m_SteeringLimit == 0.0)
    {
        m_GlobalMatrixWheelCenter = Matrix34::I;
        m_GlobalMatrixWheelCenter.m30 = m_LocalOffset.X;
        m_GlobalMatrixWheelCenter.m31 = m_LocalOffset.Y;
        m_GlobalMatrixWheelCenter.m32 = m_LocalOffset.Z;
    }
    else
    {
        // Steering rotation around Y
        m_GlobalMatrixWheelCenter.MakeRotateY(m_SteeringAngle);

        m_GlobalMatrixWheelCenter.m30 = m_LocalOffset.X;
        m_GlobalMatrixWheelCenter.m31 = m_LocalOffset.Y;
        m_GlobalMatrixWheelCenter.m32 = m_LocalOffset.Z;
        m_GlobalMatrixWheelCenter.m30 = m_GlobalMatrixWheelCenter.m30 - m_SteeringPivotOffset;
        m_GlobalMatrixWheelCenter.m30 += m_SteeringPivotOffset * m_GlobalMatrixWheelCenter.m00;
        m_GlobalMatrixWheelCenter.m31 += m_SteeringPivotOffset * m_GlobalMatrixWheelCenter.m01;
        m_GlobalMatrixWheelCenter.m32 += m_SteeringPivotOffset * m_GlobalMatrixWheelCenter.m02;
    }

    m_GlobalMatrixWheelCenter.m31 = m_VerticalSurfaceThreshold + m_GlobalMatrixWheelCenter.m31;

    m_GlobalMatrixWheelCenter.Dot(*pWorldTransform);

    rayUp = m_SuspensionLimit + 0.5;
    rayStart.X = rayUp * pWorldTransform->m10 + m_GlobalMatrixWheelCenter.m30;
    rayStart.Y = rayUp * pWorldTransform->m11 + m_GlobalMatrixWheelCenter.m31;
    rayStart.Z = rayUp * pWorldTransform->m12 + m_GlobalMatrixWheelCenter.m32;
    rayDown = -(m_Radius * 1.414 + m_SuspensionExtent);
    rayEnd.X = rayDown * pWorldTransform->m10 + m_GlobalMatrixWheelCenter.m30;
    rayEnd.Y = rayDown * pWorldTransform->m11 + m_GlobalMatrixWheelCenter.m31;
    rayEnd.Z = rayDown * pWorldTransform->m12 + m_GlobalMatrixWheelCenter.m32;

    if (m_UsePivotOffset)
    {
        v17 = m_SteeringPivotOffset * m_GlobalMatrixWheelCenter.m00;
        v18 = m_SteeringPivotOffset * m_GlobalMatrixWheelCenter.m01;
        v14 = m_SteeringPivotOffset * m_GlobalMatrixWheelCenter.m02;
        rayStart.X += v17;
        rayStart.Y += v18;
        rayStart.Z += v14;
        rayEnd.X += v17;
        rayEnd.Y += v18;
        rayEnd.Z += v14;
    }

    m_RayStartPos = rayStart;    
    m_RayEndPos = rayEnd;
    m_MaybeHasIntersection = 0;
}

void vehWheel::CalcDispAndDamp(float* disp, float dispTarget, float step, float limit, float* outStep, bool* outFree)
{
    float nextDisp;
    float clamped;

    nextDisp = step + *disp;

    if (step < 0.0f)
    {
        if (dispTarget > nextDisp)
        {
            clamped = limit + *disp;
            if (dispTarget < clamped)
                clamped = dispTarget;

            *disp = clamped;
            *outStep = 0.0f;
            *outFree = false;
        }
        else
        {
            *disp = nextDisp;
            *outStep = step;
            *outFree = true;
        }
    }
    else if (dispTarget < nextDisp)
    {
        clamped = *disp - limit;
        if (dispTarget > clamped)
            clamped = dispTarget;

        *disp = clamped;
        *outStep = 0.0f;
        *outFree = false;
    }
    else
    {
        *disp = nextDisp;
        *outStep = step;
        *outFree = true;
    }
}

void vehWheel::ComputeSlipPercent(float* slipPercent, float slipVelocity, float referenceVelocity)
{
    const float absRef = fabsf(referenceVelocity);

    // No slip if slip velocity is zero
    if (slipVelocity == 0.0f)
    {
        *slipPercent = 0.0f;
        return;
    }

    // Clamp if slip exceeds reference
    if (fabsf(slipVelocity) >= absRef)
    {
        *slipPercent = math::Sign(slipVelocity);
        return;
    }

    // Normalized slip
    *slipPercent = slipVelocity / absRef;
}

float vehWheel::ComputeFriction(float slipPercent, float* outSlipRatio) const
{    
    float absSlip = fabs(slipPercent);

    // Maximum sliding friction for this surface
    float maxSlidingFriction = m_SlidingFric * m_SurfaceFriction;

    // Compute the friction
    float friction = m_StaticFric * m_SurfaceFriction * (2.0f * m_OptimumSlipPercent - absSlip) * m_InvOSPSquared * absSlip;

    // Compute friction blend factor
    if (absSlip <= m_OptimumSlipPercent)
        *outSlipRatio = 0.5f * absSlip / m_OptimumSlipPercent;
    else
    {
        *outSlipRatio = 0.5f + 0.5f * (absSlip - m_OptimumSlipPercent) / (1.0f - m_OptimumSlipPercent);

        // Clamp result to sliding friction if needed
        if (friction <= maxSlidingFriction)
        {
            return maxSlidingFriction;
        }
    }
    return friction;
}

float vehWheel::GetVisualDispVert() const
{
    float maxDisp = m_Radius * 0.2f;
    float disp = m_Radius * 0.05f / m_NormalLoad * m_SomeTireLoad;

    if (disp < 0.0f)
        return 0.0f;

    return (disp <= maxDisp) ? disp : maxDisp;
}

// Original baseline code
/*void vehWheel::Update()
{
    vehCarSim* vehcarsim; // eax
    float* p_dword_00; // ecx
    double v4; // st7
    double v5; // st7
    phInertialCS* ics; // ecx MAPDST
    double v7; // st7
    double v8; // st6
    double m10; // st7
    double m11; // st6
    double v11; // st7
    double v12; // st5
    double v13; // st4
    double v14; // st2
    double v15; // st7
    double v16; // st7
    float* p_slip_percent_lat; // edi
    float* slip_percent_long; // ebp
    double vel_long_relative; // st7
    double v20; // st7
    double v21; // st7
    double v22; // st7
    long double abs_slip_percent_long; // st7
    double tire_disp_long; // st7
    double v28; // st7
    double tire_disp_lat; // st7
    double v33; // st7
    double v34; // st7
    char v35; // cl
    double v36; // st6
    double v37; // st7
    float v38; // ebp
    float* p_tire_disp_lat; // ebx
    bool v40; // dl
    bool v41; // cl
    double v42; // st7
    double v43; // st6
    double v44; // st7
    long double v45; // st5
    long double v46; // st7
    double tire_force_long; // st7
    double v50; // st7
    double v51; // st6
    double v52; // rt1
    double v53; // st6
    float mass; // ecx
    double z; // st7
    double some_tire_load; // st6
    phInertialCS* ics_1; // ecx
    Vector3* p_dword_48; // eax
    double v59; // st7
    double v60; // st6
    Vector3* p_torque; // eax
    float vel_lat; // [esp-10h] [ebp-C0h]
    float vel_long; // [esp-Ch] [ebp-BCh]
    float v71; // [esp+8h] [ebp-A8h]
    float v74; // [esp+Ch] [ebp-A4h] BYREF
    float tire_disp_limit_lat; // [esp+10h] [ebp-A0h]
    float some_normal_load; // [esp+14h] [ebp-9Ch] BYREF
    Vector3 v77; // [esp+18h] [ebp-98h] BYREF
    float a2; // [esp+24h] [ebp-8Ch]
    bool a6; // [esp+2Ah] [ebp-86h] BYREF
    bool a6_1; // [esp+2Bh] [ebp-85h] BYREF
    float a6_2; // [esp+2Ch] [ebp-84h]
    float v82; // [esp+30h] [ebp-80h]
    float a5; // [esp+34h] [ebp-7Ch] BYREF
    float available_friction; // [esp+38h] [ebp-78h]
    float v85; // [esp+3Ch] [ebp-74h]
    Vector3 some_force; // [esp+40h] [ebp-70h] BYREF
    Vector3 contact_offset; // [esp+4Ch] [ebp-64h] BYREF
    float v88; // [esp+58h] [ebp-58h]
    float a3; // [esp+5Ch] [ebp-54h] BYREF
    float v90; // [esp+60h] [ebp-50h]
    float v91; // [esp+64h] [ebp-4Ch]
    float a4; // [esp+68h] [ebp-48h]
    float v93; // [esp+6Ch] [ebp-44h] BYREF
    float v94; // [esp+70h] [ebp-40h]
    float v95; // [esp+74h] [ebp-3Ch]
    float v96; // [esp+78h] [ebp-38h]
    float v97; // [esp+7Ch] [ebp-34h]
    Matrix34 contact_response; // [esp+80h] [ebp-30h] BYREF

    if (!this->has_intersection)
    {
        p_tire_disp_lat = &this->tire_disp_lat;
        this->on_ground = 0;
        this->sliding_strength = 0.0;
        this->is_sliding = 0;
        this->tire_disp_lat = 0.0;
        this->tire_disp_long = 0.0;
        this->wheel_drive_torque = 0.0;
        this->tire_force_long = 0.0;
        this->tire_force_lat = 0.0;
        UpdateVisuals();
        return;
    }

    vehcarsim = this->vehcarsim;
    this->on_ground = 1;
    p_dword_00 = (float*)&vehcarsim->collider->some_inst_parent->dword_00;
    v4 = this->contact_matrix.m30 - p_dword_00[0xD];
    p_dword_00 += 0xD;
    contact_offset.X = v4;
    contact_offset.Y = this->contact_matrix.m31 - p_dword_00[1];
    v5 = this->contact_matrix.m32 - p_dword_00[2];
    ics = this->ics;
    contact_offset.Z = v5;
    v7 = ics->world_transform.m20 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m10 * vehcarsim->center_of_mass.Y
        + ics->world_transform.m00 * vehcarsim->center_of_mass.X;
    v8 = ics->world_transform.m01 * vehcarsim->center_of_mass.X
        + ics->world_transform.m21 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m11 * vehcarsim->center_of_mass.Y;
    v95 = ics->world_transform.m02 * vehcarsim->center_of_mass.X
        + ics->world_transform.m22 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m12 * vehcarsim->center_of_mass.Y;
    contact_offset.X = contact_offset.X - v7;
    contact_offset.Y = contact_offset.Y - v8;
    contact_offset.Z = contact_offset.Z - v95;
    if (this->some_normal_load > 0.0)
    {
        m10 = this->contact_matrix.m10;
        m11 = this->contact_matrix.m11;
        some_force.Z = this->contact_matrix.m12;
        some_normal_load = this->some_normal_load;
        memset(&contact_response.m30, 0, 0xC);
        some_force.X = some_normal_load * m10;
        v11 = some_normal_load * m11;
        v12 = some_force.X * this->contact_matrix.m11;
        v13 = some_force.X * this->contact_matrix.m12;
        tire_disp_limit_lat = v11 * this->contact_matrix.m12;
        v14 = some_force.X * this->contact_matrix.m10;
        contact_response.m21 = tire_disp_limit_lat;
        contact_response.m00 = v14;
        contact_response.m01 = v12;
        contact_response.m02 = v13;
        contact_response.m10 = v12;
        contact_response.m11 = v11 * this->contact_matrix.m11;
        contact_response.m12 = tire_disp_limit_lat;
        contact_response.m20 = v13;
        contact_response.m22 = some_normal_load * some_force.Z * this->contact_matrix.m12;
        if (this->suspension_value >= (double)this->suspension_limit)
        {
            memset(&some_force, 0, sizeof(some_force));
        }
        else
        {
            v15 = this->some_suspension_force_or_load * this->suspension_velocity * datTimeManager::GetSeconds();
            some_force.X = v15 * this->contact_matrix.m10;
            some_force.Y = v15 * this->contact_matrix.m11;
            some_force.Z = v15 * this->contact_matrix.m12;
        }
        ics->ApplyContactForce(
            &some_force,
            (Vector3*)&this->contact_matrix.m30,
            &contact_response,
            &contact_offset);
    }
    v16 = this->some_angular_velocity * this->radius;
    vel_long = this->vel_long;
    vel_lat = this->vel_lat;
    p_slip_percent_lat = &this->slip_percent_lat;
    this->vel_long_relative = v16 + vel_long;
    a4 = v16;
    v97 = fabs(this->vel_lat);
    v82 = fabs(this->vel_long);
    ComputeSlipPercent(&this->slip_percent_lat, vel_lat, vel_long);
    slip_percent_long = &this->slip_percent_long;
    ComputeSlipPercent(&this->slip_percent_long, this->vel_long_relative, this->vel_long);
    vel_long_relative = this->vel_long_relative;
    if (vel_long_relative <= 0.0)
    {
        if (vel_long_relative >= 0.0)
            v20 = 0.0;
        else
            v20 = -1.0;
    }
    else
    {
        v20 = 1.0;
    }
    v74 = v20 / this->stiffness_long * this->some_tire_load;
    v21 = this->vel_lat;
    if (v21 <= 0.0)
    {
        if (v21 >= 0.0)
            v22 = 0.0;
        else
            v22 = -1.0;
    }
    else
    {
        v22 = 1.0;
    }
    a5 = v22 / this->stiffness_lat * this->some_tire_load;
    v88 = datTimeManager::GetSeconds() * this->vel_lat;
    v90 = datTimeManager::GetSeconds() * this->vel_long_relative;
    available_friction = this->static_fric * this->surface_friction;
    a6_2 = available_friction;
    a2 = available_friction * v74;
    tire_disp_limit_lat = fabs(*p_slip_percent_lat);
    abs_slip_percent_long = fabs(*slip_percent_long);
    v85 = abs_slip_percent_long;
    if (abs_slip_percent_long <= this->optimum_slip_percent)
        goto LABEL_21;
    tire_disp_long = this->tire_disp_long;
    if (tire_disp_long >= optimum_slip_percent)
    {
        if (tire_disp_long <= a2 || a2 - this->tire_disp_long > v90)
            goto LABEL_21;
    }
    else if (tire_disp_long >= a2 || a2 - this->tire_disp_long < v90)
    {
    LABEL_21:
        v28 = ComputeFriction(*slip_percent_long, &a3);
        a6_2 = v28;
        a2 = v28 * v74;
        goto LABEL_22;
    }
    a3 = 0.0;
LABEL_22:
    v91 = available_friction;
    v71 = available_friction * a5;
    if (tire_disp_limit_lat <= (double)this->optimum_slip_percent)
    {
    LABEL_26:
        v33 = ComputeFriction(*p_slip_percent_lat, &some_normal_load);
        v91 = v33;
        v71 = v33 * a5;
        goto LABEL_27;
    }
    tire_disp_lat = this->tire_disp_lat;
    if (tire_disp_lat >= optimum_slip_percent)
    {
        if (tire_disp_lat <= v71 || v71 - this->tire_disp_lat > v88)
            goto LABEL_26;
    }
    else if (tire_disp_lat >= v71 || v71 - this->tire_disp_lat < v88)
    {
        goto LABEL_26;
    }
    some_normal_load = 0.0;
LABEL_27:
    v34 = v85;
    v35 = 1;
    if (*p_slip_percent_lat > (double)v85)
    {
        v34 = *p_slip_percent_lat;
    LABEL_37:
        v35 = 0;
        goto LABEL_38;
    }
    v36 = -*p_slip_percent_lat;
    v96 = v36;
    if (v36 > v85)
    {
        v34 = v96;
        goto LABEL_37;
    }
LABEL_38:
    if (v34 < this->optimum_slip_percent)
    {
        a6_2 = available_friction;
    }
    else
    {
        if (v35)
            v37 = a6_2;
        else
            v37 = v91;
        if (v91 > v37)
            v71 = a5 * v37;
        if (a6_2 > v37)
            a2 = v74 * v37;
        a6_2 = v37;
    }
    a4 = fabs(a4) * datTimeManager::GetSeconds() * LRelaxCoef;
    a5 = 0.0;
    v38 = a4;
    v74 = 0.0;
    CalcDispAndDamp(&this->tire_disp_long, a2, v90, a4, &a5, &a6_1);
    p_tire_disp_lat = &this->tire_disp_lat;
    CalcDispAndDamp(&this->tire_disp_lat, v71, v88, v38, &v74, &a6);
    v40 = a6_1;
    v41 = a6;
    this->tire_force_lat = -(this->tire_disp_lat * this->stiffness_lat)
        - v74 * datTimeManager::GetInvSeconds() * this->damping_lat;
    v42 = -(this->stiffness_long * this->tire_disp_long) - a5 * datTimeManager::GetInvSeconds() * this->damping_long;
    this->tire_force_long = v42;
    v74 = this->tire_force_lat * this->tire_force_lat + v42 * v42;
    v43 = a6_2 * this->some_tire_load;
    v44 = v43 * v43;
    if (v74 > v43 * v43)
    {
        if ((v40 || v85 <= (double)this->optimum_slip_percent)
            && (v41 || tire_disp_limit_lat <= (double)this->optimum_slip_percent))
        {
            v46 = sqrt(v44 / v74);
            this->tire_force_lat = v46 * this->tire_force_lat;
            this->tire_force_long = v46 * this->tire_force_long;
        }
        else
        {
            v45 = sqrt(v44 / (this->vel_long_relative * this->vel_long_relative + this->vel_lat * this->vel_lat));
            this->tire_force_lat = -v45 * this->vel_lat;
            this->tire_force_long = -v45 * this->vel_long_relative;
        }
    }
    if (v40)
    {
        if (v41)
            this->sliding_strength = 0.0;
        else
            this->sliding_strength = some_normal_load;
    }
    else if (v41)
    {
        this->sliding_strength = a3;
    }
    else if (some_normal_load <= (double)a3)
    {
        this->sliding_strength = a3;
    }
    else
    {
        this->sliding_strength = some_normal_load;
    }
    tire_force_long = this->tire_force_long;
    this->is_sliding = this->sliding_strength > 0.5;
    ics = this->ics;
    some_unk_ptr = this->some_unk_ptr;
    this->wheel_drive_torque = tire_force_long * this->radius;
    v50 = this->surface_drag * this->some_tire_load;
    v51 = this->tire_force_lat - this->tire_drag_coef_lat * this->vel_lat * v50 * v97;
    v77.X = v51 * this->contact_matrix.m00;
    v77.Y = v51 * this->contact_matrix.m01;
    v52 = v51 * this->contact_matrix.m02;
    v53 = -this->tire_force_long - -(v50 * this->tire_drag_coef_long * this->vel_long * v82) * (this->surface_depth + 1.0);
    v77.X = v53 * this->contact_matrix.m20 + v77.X;
    v77.Y = v53 * this->contact_matrix.m21 + v77.Y;
    v77.Z = v53 * this->contact_matrix.m22 + v52;
    v93 = -v77.X;
    v94 = -v77.Y;
    v95 = -v77.Z;
    mass = ics->mass;

    v77.X = -v93;
    v77.Y = -v94;
    z = -v95;

    some_tire_load = this->some_tire_load;
    ics_1 = this->ics;
    v77.X = some_tire_load * this->contact_matrix.m10 + v77.X;
    v77.Y = some_tire_load * this->contact_matrix.m11 + v77.Y;
    v77.Z = some_tire_load * this->contact_matrix.m12 + z;
    ics_1->sub_595280(&v77, (Vector3*)&this->contact_matrix.m30);
    p_dword_48 = &this->vehcarsim->dword_48;
    some_force.Z = v77.Y * contact_offset.X - contact_offset.Y * v77.X;
    some_force.X = (v77.Z * contact_offset.Y - v77.Y * contact_offset.Z) * p_dword_48->X;
    v59 = (contact_offset.Z * v77.X - v77.Z * contact_offset.X) * p_dword_48->Y;
    v60 = some_force.Z * p_dword_48->Z;
    p_torque = &this->ics->torque;
    p_torque->X = some_force.X + p_torque->X;
    p_torque->Y = v59 + p_torque->Y;
    p_torque->Z = v60 + p_torque->Z;

    UpdateVisuals();
}*/

// Slightly cleaner, still works
/*void vehWheel::Update()
{
    double v7; // st7
    double v8; // st6
    double m10; // st7
    double m11; // st6
    double v11; // st7
    double v12; // st5
    double v13; // st4
    double v14; // st2
    double v15; // st7
    double v16; // st7
    float* p_slip_percent_lat; // edi
    float* slip_percent_long; // ebp
    double vel_long_relative; // st7
    double v20; // st7
    double v21; // st7
    double v22; // st7
    long double abs_slip_percent_long; // st7
    double tire_disp_long; // st7
    double v28; // st7
    double tire_disp_lat; // st7
    double v33; // st7
    double v34; // st7
    char v35; // cl
    double v36; // st6
    double v37; // st7
    float v38; // ebp
    bool v40; // dl
    bool v41; // cl
    double v42; // st7
    double v43; // st6
    double v44; // st7
    long double v45; // st5
    long double v46; // st7
    double tire_force_long; // st7
    double v50; // st7
    double v51; // st6
    double v52; // rt1
    double v53; // st6
    float mass; // ecx
    double z; // st7
    double some_tire_load; // st6
    Vector3* p_dword_48; // eax
    double v59; // st7
    double v60; // st6
    Vector3* p_torque; // eax
    float vel_lat; // [esp-10h] [ebp-C0h]
    float vel_long; // [esp-Ch] [ebp-BCh]
    float v71; // [esp+8h] [ebp-A8h]
    float v74; // [esp+Ch] [ebp-A4h] BYREF
    float tire_disp_limit_lat; // [esp+10h] [ebp-A0h]
    float some_normal_load; // [esp+14h] [ebp-9Ch] BYREF
    Vector3 v77; // [esp+18h] [ebp-98h] BYREF
    float a2; // [esp+24h] [ebp-8Ch]
    bool a6; // [esp+2Ah] [ebp-86h] BYREF
    bool a6_1; // [esp+2Bh] [ebp-85h] BYREF
    float a6_2; // [esp+2Ch] [ebp-84h]
    float v82; // [esp+30h] [ebp-80h]
    float a5; // [esp+34h] [ebp-7Ch] BYREF
    float available_friction; // [esp+38h] [ebp-78h]
    float v85; // [esp+3Ch] [ebp-74h]
    Vector3 some_force; // [esp+40h] [ebp-70h] BYREF
    float v88; // [esp+58h] [ebp-58h]
    float a3; // [esp+5Ch] [ebp-54h] BYREF
    float v90; // [esp+60h] [ebp-50h]
    float v91; // [esp+64h] [ebp-4Ch]
    float a4; // [esp+68h] [ebp-48h]
    float v93; // [esp+6Ch] [ebp-44h] BYREF
    float v94; // [esp+70h] [ebp-40h]
    float v95; // [esp+74h] [ebp-3Ch]
    float v96; // [esp+78h] [ebp-38h]
    float v97; // [esp+7Ch] [ebp-34h]
    Matrix34 contact_response; // [esp+80h] [ebp-30h] BYREF

    if (!this->has_intersection)
    {
        this->on_ground = 0;
        this->sliding_strength = 0.0;
        this->is_sliding = 0;
        this->tire_disp_lat = 0.0;
        this->tire_disp_long = 0.0;
        this->wheel_drive_torque = 0.0;
        this->tire_force_long = 0.0;
        this->tire_force_lat = 0.0;
        UpdateVisuals();
        return;
    }

    this->on_ground = 1;

    Vector3 parent_pos = vehcarsim->collider->some_inst_parent->some_inst_parent_transform.GetRow(3);
    Vector3 contact_offset = contact_matrix.GetRow(3) - parent_pos;

    v7 = ics->world_transform.m20 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m10 * vehcarsim->center_of_mass.Y
        + ics->world_transform.m00 * vehcarsim->center_of_mass.X;
    v8 = ics->world_transform.m01 * vehcarsim->center_of_mass.X
        + ics->world_transform.m21 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m11 * vehcarsim->center_of_mass.Y;
    v95 = ics->world_transform.m02 * vehcarsim->center_of_mass.X
        + ics->world_transform.m22 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m12 * vehcarsim->center_of_mass.Y;
    contact_offset.X -= v7;
    contact_offset.Y -= v8;
    contact_offset.Z -= v95;

    if (this->some_normal_load > 0.0)
    {
        contact_response = Matrix34::I;
        m10 = this->contact_matrix.m10;
        m11 = this->contact_matrix.m11;
        some_force.Z = this->contact_matrix.m12;
        some_normal_load = this->some_normal_load;
        some_force.X = some_normal_load * m10;
        v11 = some_normal_load * m11;
        v12 = some_force.X * this->contact_matrix.m11;
        v13 = some_force.X * this->contact_matrix.m12;
        tire_disp_limit_lat = v11 * this->contact_matrix.m12;
        v14 = some_force.X * this->contact_matrix.m10;
        contact_response.m21 = tire_disp_limit_lat;
        contact_response.m00 = v14;
        contact_response.m01 = v12;
        contact_response.m02 = v13;
        contact_response.m10 = v12;
        contact_response.m11 = v11 * this->contact_matrix.m11;
        contact_response.m12 = tire_disp_limit_lat;
        contact_response.m20 = v13;
        contact_response.m22 = some_normal_load * some_force.Z * this->contact_matrix.m12;
        if (this->suspension_value >= (double)this->suspension_limit)
        {
            some_force = Vector3(0.0f, 0.0f, 0.0f);
        }
        else
        {
            v15 = this->some_suspension_force_or_load * this->suspension_velocity * datTimeManager::GetSeconds();
            some_force.X = v15 * this->contact_matrix.m10;
            some_force.Y = v15 * this->contact_matrix.m11;
            some_force.Z = v15 * this->contact_matrix.m12;
        }
        ics->ApplyContactForce(
            &some_force,
            &this->contact_matrix.GetRow(3),
            &contact_response,
            &contact_offset);
    }
    v16 = this->some_angular_velocity * this->radius;
    vel_long = this->vel_long;
    vel_lat = this->vel_lat;
    p_slip_percent_lat = &this->slip_percent_lat;
    this->vel_long_relative = v16 + vel_long;
    a4 = v16;
    v97 = fabs(this->vel_lat);
    v82 = fabs(this->vel_long);
    ComputeSlipPercent(&this->slip_percent_lat, vel_lat, vel_long);
    slip_percent_long = &this->slip_percent_long;
    ComputeSlipPercent(&this->slip_percent_long, this->vel_long_relative, this->vel_long);
    vel_long_relative = this->vel_long_relative;
    if (vel_long_relative <= 0.0)
    {
        if (vel_long_relative >= 0.0)
            v20 = 0.0;
        else
            v20 = -1.0;
    }
    else
    {
        v20 = 1.0;
    }
    v74 = v20 / this->stiffness_long * this->some_tire_load;
    v21 = this->vel_lat;
    if (v21 <= 0.0)
    {
        if (v21 >= 0.0)
            v22 = 0.0;
        else
            v22 = -1.0;
    }
    else
    {
        v22 = 1.0;
    }
    a5 = v22 / this->stiffness_lat * this->some_tire_load;
    v88 = datTimeManager::GetSeconds() * this->vel_lat;
    v90 = datTimeManager::GetSeconds() * this->vel_long_relative;
    available_friction = this->static_fric * this->surface_friction;
    a6_2 = available_friction;
    a2 = available_friction * v74;
    tire_disp_limit_lat = fabs(*p_slip_percent_lat);
    abs_slip_percent_long = fabs(*slip_percent_long);
    v85 = abs_slip_percent_long;
    if (abs_slip_percent_long <= this->optimum_slip_percent)
        goto LABEL_21;
    tire_disp_long = this->tire_disp_long;
    if (tire_disp_long >= optimum_slip_percent)
    {
        if (tire_disp_long <= a2 || a2 - this->tire_disp_long > v90)
            goto LABEL_21;
    }
    else if (tire_disp_long >= a2 || a2 - this->tire_disp_long < v90)
    {
    LABEL_21:
        v28 = ComputeFriction(*slip_percent_long, &a3);
        a6_2 = v28;
        a2 = v28 * v74;
        goto LABEL_22;
    }
    a3 = 0.0;
LABEL_22:
    v91 = available_friction;
    v71 = available_friction * a5;
    if (tire_disp_limit_lat <= (double)this->optimum_slip_percent)
    {
    LABEL_26:
        v33 = ComputeFriction(*p_slip_percent_lat, &some_normal_load);
        v91 = v33;
        v71 = v33 * a5;
        goto LABEL_27;
    }
    tire_disp_lat = this->tire_disp_lat;
    if (tire_disp_lat >= optimum_slip_percent)
    {
        if (tire_disp_lat <= v71 || v71 - this->tire_disp_lat > v88)
            goto LABEL_26;
    }
    else if (tire_disp_lat >= v71 || v71 - this->tire_disp_lat < v88)
    {
        goto LABEL_26;
    }
    some_normal_load = 0.0;
LABEL_27:
    v34 = v85;
    v35 = 1;
    if (*p_slip_percent_lat > (double)v85)
    {
        v34 = *p_slip_percent_lat;
    LABEL_37:
        v35 = 0;
        goto LABEL_38;
    }
    v36 = -*p_slip_percent_lat;
    v96 = v36;
    if (v36 > v85)
    {
        v34 = v96;
        goto LABEL_37;
    }
LABEL_38:
    if (v34 < this->optimum_slip_percent)
    {
        a6_2 = available_friction;
    }
    else
    {
        if (v35)
            v37 = a6_2;
        else
            v37 = v91;
        if (v91 > v37)
            v71 = a5 * v37;
        if (a6_2 > v37)
            a2 = v74 * v37;
        a6_2 = v37;
    }
    a4 = fabs(a4) * datTimeManager::GetSeconds() * LRelaxCoef;
    a5 = 0.0;
    v38 = a4;
    v74 = 0.0;
    CalcDispAndDamp(&this->tire_disp_long, a2, v90, a4, &a5, &a6_1);
    CalcDispAndDamp(&this->tire_disp_lat, v71, v88, v38, &v74, &a6);
    v40 = a6_1;
    v41 = a6;
    this->tire_force_lat = -(this->tire_disp_lat * this->stiffness_lat)
        - v74 * datTimeManager::GetInvSeconds() * this->damping_lat;
    v42 = -(this->stiffness_long * this->tire_disp_long) - a5 * datTimeManager::GetInvSeconds() * this->damping_long;
    this->tire_force_long = v42;
    v74 = this->tire_force_lat * this->tire_force_lat + v42 * v42;
    v43 = a6_2 * this->some_tire_load;
    v44 = v43 * v43;
    if (v74 > v43 * v43)
    {
        if ((v40 || v85 <= (double)this->optimum_slip_percent)
            && (v41 || tire_disp_limit_lat <= (double)this->optimum_slip_percent))
        {
            v46 = sqrt(v44 / v74);
            this->tire_force_lat = v46 * this->tire_force_lat;
            this->tire_force_long = v46 * this->tire_force_long;
        }
        else
        {
            v45 = sqrt(v44 / (this->vel_long_relative * this->vel_long_relative + this->vel_lat * this->vel_lat));
            this->tire_force_lat = -v45 * this->vel_lat;
            this->tire_force_long = -v45 * this->vel_long_relative;
        }
    }
    if (v40)
    {
        if (v41)
            this->sliding_strength = 0.0;
        else
            this->sliding_strength = some_normal_load;
    }
    else if (v41)
    {
        this->sliding_strength = a3;
    }
    else if (some_normal_load <= (double)a3)
    {
        this->sliding_strength = a3;
    }
    else
    {
        this->sliding_strength = some_normal_load;
    }
    tire_force_long = this->tire_force_long;
    this->is_sliding = this->sliding_strength > 0.5;
    ics = this->ics;
    some_unk_ptr = this->some_unk_ptr;
    this->wheel_drive_torque = tire_force_long * this->radius;
    v50 = this->surface_drag * this->some_tire_load;
    v51 = this->tire_force_lat - this->tire_drag_coef_lat * this->vel_lat * v50 * v97;
    v77.X = v51 * this->contact_matrix.m00;
    v77.Y = v51 * this->contact_matrix.m01;
    v52 = v51 * this->contact_matrix.m02;
    v53 = -this->tire_force_long - -(v50 * this->tire_drag_coef_long * this->vel_long * v82) * (this->surface_depth + 1.0);
    v77.X = v53 * this->contact_matrix.m20 + v77.X;
    v77.Y = v53 * this->contact_matrix.m21 + v77.Y;
    v77.Z = v53 * this->contact_matrix.m22 + v52;
    v93 = -v77.X;
    v94 = -v77.Y;
    v95 = -v77.Z;
    mass = ics->mass;

    v77.X = -v93;
    v77.Y = -v94;
    z = -v95;

    some_tire_load = this->some_tire_load;
    v77.X = some_tire_load * this->contact_matrix.m10 + v77.X;
    v77.Y = some_tire_load * this->contact_matrix.m11 + v77.Y;
    v77.Z = some_tire_load * this->contact_matrix.m12 + z;
    ics->sub_595280(&v77, &this->contact_matrix.GetRow(3));
    p_dword_48 = &this->vehcarsim->dword_48;
    some_force.Z = v77.Y * contact_offset.X - contact_offset.Y * v77.X;
    some_force.X = (v77.Z * contact_offset.Y - v77.Y * contact_offset.Z) * p_dword_48->X;
    v59 = (contact_offset.Z * v77.X - v77.Z * contact_offset.X) * p_dword_48->Y;
    v60 = some_force.Z * p_dword_48->Z;

    p_torque = &this->ics->torque;
    p_torque->X = some_force.X + p_torque->X;
    p_torque->Y = v59 + p_torque->Y;
    p_torque->Z = v60 + p_torque->Z;

    UpdateVisuals();
}*/

// Goto-free
/*void vehWheel::Update()
{
    double v7; // st7
    double v8; // st6
    double m10; // st7
    double m11; // st6
    double v11; // st7
    double v12; // st5
    double v13; // st4
    double v14; // st2
    double v15; // st7
    double v16; // st7
    float* p_slip_percent_lat; // edi
    float* slip_percent_long; // ebp
    double vel_long_relative; // st7
    double v20; // st7
    double v21; // st7
    double v22; // st7
    long double abs_slip_percent_long; // st7
    double tire_disp_long; // st7
    double v28; // st7
    double tire_disp_lat; // st7
    double v33; // st7
    double v34; // st7
    char v35; // cl
    double v37; // st7
    float v38; // ebp
    bool v40; // dl
    bool v41; // cl
    double v42; // st7
    double v43; // st6
    double v44; // st7
    long double v45; // st5
    long double v46; // st7
    double tire_force_long; // st7
    double v50; // st7
    double v51; // st6
    double v52; // rt1
    double v53; // st6
    float mass; // ecx
    double z; // st7
    double some_tire_load; // st6
    Vector3* p_dword_48; // eax
    double v59; // st7
    double v60; // st6
    Vector3* p_torque; // eax
    float vel_lat; // [esp-10h] [ebp-C0h]
    float vel_long; // [esp-Ch] [ebp-BCh]
    float v71; // [esp+8h] [ebp-A8h]
    float v74; // [esp+Ch] [ebp-A4h] BYREF
    float tire_disp_limit_lat; // [esp+10h] [ebp-A0h]
    float some_normal_load; // [esp+14h] [ebp-9Ch] BYREF
    Vector3 v77; // [esp+18h] [ebp-98h] BYREF
    float a2; // [esp+24h] [ebp-8Ch]
    bool a6; // [esp+2Ah] [ebp-86h] BYREF
    bool a6_1; // [esp+2Bh] [ebp-85h] BYREF
    float a6_2; // [esp+2Ch] [ebp-84h]
    float v82; // [esp+30h] [ebp-80h]
    float a5; // [esp+34h] [ebp-7Ch] BYREF
    float available_friction; // [esp+38h] [ebp-78h]
    float v85; // [esp+3Ch] [ebp-74h]
    Vector3 some_force; // [esp+40h] [ebp-70h] BYREF
    float v88; // [esp+58h] [ebp-58h]
    float a3; // [esp+5Ch] [ebp-54h] BYREF
    float v90; // [esp+60h] [ebp-50h]
    float v91; // [esp+64h] [ebp-4Ch]
    float a4; // [esp+68h] [ebp-48h]
    float v93; // [esp+6Ch] [ebp-44h] BYREF
    float v94; // [esp+70h] [ebp-40h]
    float v95; // [esp+74h] [ebp-3Ch]
    float v97; // [esp+7Ch] [ebp-34h]

    if (!this->has_intersection)
    {
        this->on_ground = 0;
        this->sliding_strength = 0.0;
        this->is_sliding = 0;
        this->tire_disp_lat = 0.0;
        this->tire_disp_long = 0.0;
        this->wheel_drive_torque = 0.0;
        this->tire_force_long = 0.0;
        this->tire_force_lat = 0.0;

        UpdateVisuals();
        return;
    }

    this->on_ground = 1;

    Vector3 parent_pos = vehcarsim->collider->some_inst_parent->some_inst_parent_transform.GetRow(3);
    Vector3 contact_offset = contact_matrix.GetRow(3) - parent_pos;

    v7 = ics->world_transform.m20 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m10 * vehcarsim->center_of_mass.Y
        + ics->world_transform.m00 * vehcarsim->center_of_mass.X;
    v8 = ics->world_transform.m01 * vehcarsim->center_of_mass.X
        + ics->world_transform.m21 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m11 * vehcarsim->center_of_mass.Y;
    v95 = ics->world_transform.m02 * vehcarsim->center_of_mass.X
        + ics->world_transform.m22 * vehcarsim->center_of_mass.Z
        + ics->world_transform.m12 * vehcarsim->center_of_mass.Y;
    contact_offset.X -= v7;
    contact_offset.Y -= v8;
    contact_offset.Z -= v95;

    if (this->some_normal_load > 0.0)
    {
        Matrix34 contact_response = Matrix34::I;
        m10 = this->contact_matrix.m10;
        m11 = this->contact_matrix.m11;
        some_force.Z = this->contact_matrix.m12;
        some_normal_load = this->some_normal_load;
        some_force.X = some_normal_load * m10;
        v11 = some_normal_load * m11;
        v12 = some_force.X * this->contact_matrix.m11;
        v13 = some_force.X * this->contact_matrix.m12;
        tire_disp_limit_lat = v11 * this->contact_matrix.m12;
        v14 = some_force.X * this->contact_matrix.m10;
        contact_response.m21 = tire_disp_limit_lat;
        contact_response.m00 = v14;
        contact_response.m01 = v12;
        contact_response.m02 = v13;
        contact_response.m10 = v12;
        contact_response.m11 = v11 * this->contact_matrix.m11;
        contact_response.m12 = tire_disp_limit_lat;
        contact_response.m20 = v13;
        contact_response.m22 = some_normal_load * some_force.Z * this->contact_matrix.m12;
        if (this->suspension_value >= (double)this->suspension_limit)
        {
            some_force = Vector3(0.0f, 0.0f, 0.0f);
        }
        else
        {
            v15 = this->some_suspension_force_or_load * this->suspension_velocity * datTimeManager::GetSeconds();
            some_force.X = v15 * this->contact_matrix.m10;
            some_force.Y = v15 * this->contact_matrix.m11;
            some_force.Z = v15 * this->contact_matrix.m12;
        }
        ics->ApplyContactForce(
            &some_force,
            &this->contact_matrix.GetRow(3),
            &contact_response,
            &contact_offset);
    }
    v16 = this->some_angular_velocity * this->radius;
    vel_long = this->vel_long;
    vel_lat = this->vel_lat;
    p_slip_percent_lat = &this->slip_percent_lat;
    this->vel_long_relative = v16 + vel_long;
    a4 = v16;
    v97 = fabs(this->vel_lat);
    v82 = fabs(this->vel_long);
    ComputeSlipPercent(&this->slip_percent_lat, vel_lat, vel_long);
    slip_percent_long = &this->slip_percent_long;
    ComputeSlipPercent(&this->slip_percent_long, this->vel_long_relative, this->vel_long);
    vel_long_relative = this->vel_long_relative;
    if (vel_long_relative <= 0.0)
    {
        if (vel_long_relative >= 0.0)
            v20 = 0.0;
        else
            v20 = -1.0;
    }
    else
    {
        v20 = 1.0;
    }
    v74 = v20 / this->stiffness_long * this->some_tire_load;
    v21 = this->vel_lat;
    if (v21 <= 0.0)
    {
        if (v21 >= 0.0)
            v22 = 0.0;
        else
            v22 = -1.0;
    }
    else
    {
        v22 = 1.0;
    }
    a5 = v22 / this->stiffness_lat * this->some_tire_load;
    v88 = datTimeManager::GetSeconds() * this->vel_lat;
    v90 = datTimeManager::GetSeconds() * this->vel_long_relative;
    available_friction = this->static_fric * this->surface_friction;
    a6_2 = available_friction;
    a2 = available_friction * v74;
    tire_disp_limit_lat = fabs(*p_slip_percent_lat);
    abs_slip_percent_long = fabs(*slip_percent_long);
    v85 = abs_slip_percent_long;

    // LABEL 21
    bool recompute_long_friction = false;

    if (abs_slip_percent_long <= this->optimum_slip_percent)
    {
        recompute_long_friction = true;
    }
    else
    {
        tire_disp_long = this->tire_disp_long;
        if (tire_disp_long >= optimum_slip_percent)
        {
            if (tire_disp_long <= a2 || a2 - this->tire_disp_long > v90)
                recompute_long_friction = true;
        }
        else
        {
            if (tire_disp_long >= a2 || a2 - this->tire_disp_long < v90)
                recompute_long_friction = true;
        }
    }

    if (recompute_long_friction)
    {
        v28 = ComputeFriction(*slip_percent_long, &a3);
        a6_2 = v28;
        a2 = v28 * v74;
    }
    else
    {
        a3 = 0.0;
    }
    // END LABEL

    v91 = available_friction;
    v71 = available_friction * a5;

    // LABEL 26
    bool recompute_lat_friction = false;

    if (tire_disp_limit_lat <= (double)this->optimum_slip_percent)
    {
        recompute_lat_friction = true;
    }
    else
    {
        tire_disp_lat = this->tire_disp_lat;
        if (tire_disp_lat >= optimum_slip_percent)
        {
            if (tire_disp_lat <= v71 || v71 - this->tire_disp_lat > v88)
                recompute_lat_friction = true;
        }
        else
        {
            if (tire_disp_lat >= v71 || v71 - this->tire_disp_lat < v88)
                recompute_lat_friction = true;
        }
    }

    if (recompute_lat_friction)
    {
        v33 = ComputeFriction(*p_slip_percent_lat, &some_normal_load);
        v91 = v33;
        v71 = v33 * a5;
    }
    else
    {
        some_normal_load = 0.0;
    }
    // END LABEL

    // LABEL 27/37/38
    // Determine dominant slip axis (long vs lat)
    // v34 = max(|long slip|, |lat slip|)
    // v35 = 1 -> longitudinal dominant
    // v35 = 0 -> lateral dominant

    v34 = v85;   // abs long slip
    v35 = 1;     // assume longitudinal dominant

    double lat_slip = *p_slip_percent_lat;

    // Check positive lateral slip
    if (lat_slip > v85)
    {
        v34 = lat_slip;
        v35 = 0;
    }
    else
    {
        // Check negative lateral slip (absolute value)
        double neg_lat_slip = -lat_slip;
        if (neg_lat_slip > v85)
        {
            v34 = neg_lat_slip;
            v35 = 0;
        }
    }
    // END LABEL

    if (v34 < this->optimum_slip_percent)
    {
        a6_2 = available_friction;
    }
    else
    {
        if (v35)
            v37 = a6_2;
        else
            v37 = v91;
        if (v91 > v37)
            v71 = a5 * v37;
        if (a6_2 > v37)
            a2 = v74 * v37;
        a6_2 = v37;
    }
    a4 = fabs(a4) * datTimeManager::GetSeconds() * LRelaxCoef;
    a5 = 0.0;
    v38 = a4;
    v74 = 0.0;
    CalcDispAndDamp(&this->tire_disp_long, a2, v90, a4, &a5, &a6_1);
    CalcDispAndDamp(&this->tire_disp_lat, v71, v88, v38, &v74, &a6);
    v40 = a6_1;
    v41 = a6;
    this->tire_force_lat = -(this->tire_disp_lat * this->stiffness_lat)
        - v74 * datTimeManager::GetInvSeconds() * this->damping_lat;
    v42 = -(this->stiffness_long * this->tire_disp_long) - a5 * datTimeManager::GetInvSeconds() * this->damping_long;
    this->tire_force_long = v42;
    v74 = this->tire_force_lat * this->tire_force_lat + v42 * v42;
    v43 = a6_2 * this->some_tire_load;
    v44 = v43 * v43;
    if (v74 > v43 * v43)
    {
        if ((v40 || v85 <= (double)this->optimum_slip_percent)
            && (v41 || tire_disp_limit_lat <= (double)this->optimum_slip_percent))
        {
            v46 = sqrt(v44 / v74);
            this->tire_force_lat = v46 * this->tire_force_lat;
            this->tire_force_long = v46 * this->tire_force_long;
        }
        else
        {
            v45 = sqrt(v44 / (this->vel_long_relative * this->vel_long_relative + this->vel_lat * this->vel_lat));
            this->tire_force_lat = -v45 * this->vel_lat;
            this->tire_force_long = -v45 * this->vel_long_relative;
        }
    }
    if (v40)
    {
        if (v41)
            this->sliding_strength = 0.0;
        else
            this->sliding_strength = some_normal_load;
    }
    else if (v41)
    {
        this->sliding_strength = a3;
    }
    else if (some_normal_load <= (double)a3)
    {
        this->sliding_strength = a3;
    }
    else
    {
        this->sliding_strength = some_normal_load;
    }
    tire_force_long = this->tire_force_long;
    this->is_sliding = this->sliding_strength > 0.5;
    ics = this->ics;
    some_unk_ptr = this->some_unk_ptr;
    this->wheel_drive_torque = tire_force_long * this->radius;
    v50 = this->surface_drag * this->some_tire_load;
    v51 = this->tire_force_lat - this->tire_drag_coef_lat * this->vel_lat * v50 * v97;
    v77.X = v51 * this->contact_matrix.m00;
    v77.Y = v51 * this->contact_matrix.m01;
    v52 = v51 * this->contact_matrix.m02;
    v53 = -this->tire_force_long - -(v50 * this->tire_drag_coef_long * this->vel_long * v82) * (this->surface_depth + 1.0);
    v77.X = v53 * this->contact_matrix.m20 + v77.X;
    v77.Y = v53 * this->contact_matrix.m21 + v77.Y;
    v77.Z = v53 * this->contact_matrix.m22 + v52;
    v93 = -v77.X;
    v94 = -v77.Y;
    v95 = -v77.Z;
    mass = ics->mass;

    v77.X = -v93;
    v77.Y = -v94;
    z = -v95;

    some_tire_load = this->some_tire_load;
    v77.X = some_tire_load * this->contact_matrix.m10 + v77.X;
    v77.Y = some_tire_load * this->contact_matrix.m11 + v77.Y;
    v77.Z = some_tire_load * this->contact_matrix.m12 + z;
    ics->sub_595280(&v77, &this->contact_matrix.GetRow(3));
    p_dword_48 = &this->vehcarsim->dword_48;
    some_force.Z = v77.Y * contact_offset.X - contact_offset.Y * v77.X;
    some_force.X = (v77.Z * contact_offset.Y - v77.Y * contact_offset.Z) * p_dword_48->X;
    v59 = (contact_offset.Z * v77.X - v77.Z * contact_offset.X) * p_dword_48->Y;
    v60 = some_force.Z * p_dword_48->Z;

    p_torque = &this->ics->torque;
    p_torque->X = some_force.X + p_torque->X;
    p_torque->Y = v59 + p_torque->Y;
    p_torque->Z = v60 + p_torque->Z;

    UpdateVisuals();
}*/

/*void vehWheel::Update()
{
    if (!this->has_intersection)
    {
        this->on_ground = 0;
        this->sliding_strength = 0.0;
        this->is_sliding = 0;
        this->tire_disp_lat = 0.0;
        this->tire_disp_long = 0.0;
        this->wheel_drive_torque = 0.0;
        this->tire_force_long = 0.0;
        this->tire_force_lat = 0.0;

        UpdateVisuals();
        return;
    }

    this->on_ground = 1;

    // ---------------------------------------------------------------------
    // Contact offset relative to COM
    // ---------------------------------------------------------------------
    Vector3 parent_pos = vehcarsim->collider->some_inst_parent->some_inst_parent_transform.GetRow(3);
    Vector3 contact_offset = contact_matrix.GetRow(3) - parent_pos;

    Vector3 com_world;
    com_world.X = ics->world_transform.m00 * vehcarsim->center_of_mass.X
                + ics->world_transform.m10 * vehcarsim->center_of_mass.Y
                + ics->world_transform.m20 * vehcarsim->center_of_mass.Z;
    com_world.Y = ics->world_transform.m01 * vehcarsim->center_of_mass.X
                + ics->world_transform.m11 * vehcarsim->center_of_mass.Y
                + ics->world_transform.m21 * vehcarsim->center_of_mass.Z;
    com_world.Z = ics->world_transform.m02 * vehcarsim->center_of_mass.X
                + ics->world_transform.m12 * vehcarsim->center_of_mass.Y
                + ics->world_transform.m22 * vehcarsim->center_of_mass.Z;

    contact_offset -= com_world;

    // ---------------------------------------------------------------------
    // Contact impulse smoothing (very subtle if anything)
    // ---------------------------------------------------------------------
    if (this->some_normal_load > 0.0)
    {
        Matrix34 contact_response = Matrix34::I;
        Vector3 some_force;
        some_force.Z = this->contact_matrix.m12;
        some_force.X = this->some_normal_load * this->contact_matrix.m10;

        float v11 = this->some_normal_load * this->contact_matrix.m11;
        float v12 = some_force.X * this->contact_matrix.m11;
        float v13 = some_force.X * this->contact_matrix.m12;
        float v15 = v11 * this->contact_matrix.m12;

        contact_response.m00 = some_force.X * this->contact_matrix.m10;
        contact_response.m01 = v12;
        contact_response.m02 = v13;
        contact_response.m10 = v12;
        contact_response.m11 = v11 * this->contact_matrix.m11;
        contact_response.m12 = v15;
        contact_response.m20 = v13;
        contact_response.m21 = v15;
        contact_response.m22 = some_normal_load * some_force.Z * this->contact_matrix.m12;

        if (this->suspension_value >= this->suspension_limit)
            some_force = Vector3(0.0f, 0.0f, 0.0f);
        else
        {
            float impulse = this->some_suspension_force_or_load * this->suspension_velocity * datTimeManager::GetSeconds();
            some_force.X = impulse * this->contact_matrix.m10;
            some_force.Y = impulse * this->contact_matrix.m11;
            some_force.Z = impulse * this->contact_matrix.m12;
        }

        ics->ApplyContactForce(
            &some_force,
            &this->contact_matrix.GetRow(3),
            &contact_response,
            &contact_offset);
    }

    // ---------------------------------------------------------------------
    // Wheel kinematics & slip computation
    // ---------------------------------------------------------------------
    float wheel_surface_speed = this->some_angular_velocity * this->radius;
    this->slip_vel_long = wheel_surface_speed + wheel_vel_long;
    float relax_factor = wheel_surface_speed;
    float abs_wheel_vel_lat = fabs(this->wheel_vel_lat);
    float abs_wheel_vel_long = fabs(this->wheel_vel_long);

    ComputeSlipPercent(&this->slip_percent_lat, wheel_vel_lat, wheel_vel_long);
    ComputeSlipPercent(&this->slip_percent_long, this->slip_vel_long, this->wheel_vel_long);

    // Slip direction signs
    float long_slip_sign = math::Sign(slip_vel_long);
    float lat_slip_sign = math::Sign(wheel_vel_lat);

    // Slip directions are intentionally crosssed
    float disp_vel_lat = long_slip_sign / this->stiffness_long * this->some_tire_load;
    float disp_vel_long = lat_slip_sign / this->stiffness_lat * this->some_tire_load;

    float slip_delta_lat = datTimeManager::GetSeconds() * this->wheel_vel_lat;
    float slip_delta_long = datTimeManager::GetSeconds() * this->slip_vel_long;
    float friction_available = this->static_fric * this->surface_friction;
    float friction_effective = friction_available;
    float target_disp_long = friction_available * disp_vel_lat;
    float abs_slip_lat = fabs(this->slip_percent_lat);
    float abs_slip_long = fabs(this->slip_percent_long);

    // Long friction recompute
    bool recompute_long_friction = false;

    if (abs_slip_long <= this->optimum_slip_percent)
        recompute_long_friction = true;
    else
    {
        if (tire_disp_long >= optimum_slip_percent)
        {
            if (tire_disp_long <= target_disp_long || target_disp_long - this->tire_disp_long > slip_delta_long)
                recompute_long_friction = true;
        }
        else
        {
            if (tire_disp_long >= target_disp_long || target_disp_long - this->tire_disp_long < slip_delta_long)
                recompute_long_friction = true;
        }
    }

    float long_slip_ratio = 0.0f;
    if (recompute_long_friction)
    {
        float long_friction = ComputeFriction(this->slip_percent_long, &long_slip_ratio);
        friction_effective = long_friction;
        target_disp_long = long_friction * disp_vel_lat;
    }
    else
        long_slip_ratio = 0.0f;

    float friction_lat_limit = friction_available;
    float target_disp_lat = friction_available * disp_vel_long;

    // Lat friction recompute
    bool recompute_lat_friction = false;

    if (abs_slip_lat <= this->optimum_slip_percent)
        recompute_lat_friction = true;
    else
    {
        if (tire_disp_lat >= optimum_slip_percent)
        {
            if (tire_disp_lat <= target_disp_lat || target_disp_lat - this->tire_disp_lat > slip_delta_lat)
                recompute_lat_friction = true;
        }
        else
        {
            if (tire_disp_lat >= target_disp_lat || target_disp_lat - this->tire_disp_lat < slip_delta_lat)
                recompute_lat_friction = true;
        }
    }

    float lat_slip_ratio = 0.0f;
    if (recompute_lat_friction)
    {
        float lat_friction = ComputeFriction(this->slip_percent_lat, &lat_slip_ratio);
        friction_lat_limit = lat_friction;
        target_disp_lat = lat_friction * disp_vel_long;
    }
    else
        lat_slip_ratio = 0.0f;

    // ---------------------------------------------------------------------
    // Dominant slip axis selection (friction circle)
    // ---------------------------------------------------------------------
    float dominant_slip = abs_slip_long;
    bool long_dominant = true;

    if (this->slip_percent_lat > abs_slip_long)
    {
        dominant_slip = this->slip_percent_lat;
        long_dominant = false;
    }
    else
    {
        if (-this->slip_percent_lat > abs_slip_long)
        {
            dominant_slip = -this->slip_percent_lat;
            long_dominant = false;
        }
    }

    if (dominant_slip < this->optimum_slip_percent)
        friction_effective = friction_available;
    else
    {
        float friction_circle_limit;
        if (long_dominant)
            friction_circle_limit = friction_effective;
        else
            friction_circle_limit = friction_lat_limit;
        if (friction_lat_limit > friction_circle_limit)
            target_disp_lat = disp_vel_long * friction_circle_limit;
        if (friction_effective > friction_circle_limit)
            target_disp_long = disp_vel_lat * friction_circle_limit;
        friction_effective = friction_circle_limit;
    }

    // ---------------------------------------------------------------------
    // Slip relaxation
    // ---------------------------------------------------------------------
    relax_factor = fabs(relax_factor) * datTimeManager::GetSeconds() * LRelaxCoef;
    disp_vel_long = 0.0;
    disp_vel_lat = 0.0;
    bool long_sliding = false;
    bool lat_sliding = false;
    CalcDispAndDamp(&this->tire_disp_long, target_disp_long, slip_delta_long, relax_factor, &disp_vel_long, &long_sliding);
    CalcDispAndDamp(&this->tire_disp_lat, target_disp_lat, slip_delta_lat, relax_factor, &disp_vel_lat, &lat_sliding);

    // ---------------------------------------------------------------------
    // Convert displacement to forces (spring + damper)
    // ---------------------------------------------------------------------
    this->tire_force_lat = -(this->tire_disp_lat * this->stiffness_lat) - disp_vel_lat * datTimeManager::GetInvSeconds() * this->damping_lat;
    this->tire_force_long = -(this->stiffness_long * this->tire_disp_long) - disp_vel_long * datTimeManager::GetInvSeconds() * this->damping_long;

    // ---------------------------------------------------------------------
    // Friction circle enforcement
    // ---------------------------------------------------------------------
    disp_vel_lat = this->tire_force_lat * this->tire_force_lat + this->tire_force_long * this->tire_force_long;
    float max_force = friction_effective * this->some_tire_load;
    float max_force_sq = max_force * max_force;
    float force_scale = 0.0f;
    if (disp_vel_lat > max_force_sq)
    {
        if ((long_sliding || abs_slip_long <= this->optimum_slip_percent)
            && (lat_sliding || abs_slip_lat <= this->optimum_slip_percent))
        {
            force_scale = sqrt(max_force_sq / disp_vel_lat);
            this->tire_force_lat = force_scale * this->tire_force_lat;
            this->tire_force_long = force_scale * this->tire_force_long;
        }
        else
        {
            force_scale = sqrt(max_force_sq / (this->slip_vel_long * this->slip_vel_long + this->wheel_vel_lat * this->wheel_vel_lat));
            this->tire_force_lat = -force_scale * this->wheel_vel_lat;
            this->tire_force_long = -force_scale * this->slip_vel_long;
        }
    }

    // ---------------------------------------------------------------------
    // Sliding strength state
    // ---------------------------------------------------------------------
    if (long_sliding)
    {
        if (lat_sliding)
            this->sliding_strength = 0.0;
        else
            this->sliding_strength = lat_slip_ratio;
    }
    else if (lat_sliding)
        this->sliding_strength = long_slip_ratio;
    else if (lat_slip_ratio <= long_slip_ratio)
        this->sliding_strength = long_slip_ratio;
    else
        this->sliding_strength = lat_slip_ratio;

    this->is_sliding = this->sliding_strength > SlidingThreshold;
    this->wheel_drive_torque = tire_force_long * this->radius;

    // ---------------------------------------------------------------------
    // Apply torques and forces
    // ---------------------------------------------------------------------
    float surface_drag_force = this->surface_drag * this->some_tire_load;
    float lat_force = this->tire_force_lat - this->tire_drag_coef_lat * this->wheel_vel_lat * surface_drag_force * abs_wheel_vel_lat;
    float long_force = -this->tire_force_long + this->tire_drag_coef_long * this->wheel_vel_long * surface_drag_force * abs_wheel_vel_long * (this->surface_depth + 1.0f);

    Vector3 final_force = contact_matrix.GetRow(0) * lat_force       // lateral
                          + contact_matrix.GetRow(2) * long_force      // longitudinal
                          + contact_matrix.GetRow(1) * this->some_tire_load; // normal load

    ics->AddForce(final_force);

    Vector3 torque = contact_offset; // copy is critical
    torque.Cross(final_force);     // torque = r x F

    this->ics->torque.X += torque.X * this->vehcarsim->dword_48.X;
    this->ics->torque.Y += torque.Y * this->vehcarsim->dword_48.Y;
    this->ics->torque.Z += torque.Z * this->vehcarsim->dword_48.Z;

    UpdateVisuals();
}*/

void vehWheel::UpdateComp()
{
    static bool toggle = false;

    if (m_CarSim->m_Transmission->m_CallOriginal) // Debug flag
    {
        hook::Thunk<0x56DB90>::Call<void>(this); // Call original

        if (toggle == false)
        {
            Printf("vehWheel: CALLING ORIGINAL\n");
            toggle = !toggle;
        }
    }
    else if (m_CarSim->m_Transmission->m_CallOriginal == false)
    {
        hook::Thunk<0x56DB90>::Call<void>(this); // vehWheel::Update();

        if (toggle == true)
        {
            Printf("vehWheel: CALLING REWRITE\n");
            toggle = !toggle;
        }
    }
}

void vehWheel::UpdateVisuals()
{
    // ------------------------------------------------------------
    // IDA: LABEL_69 — camber + axle update
    // ------------------------------------------------------------

    if (m_CamberLimit > -1.0f && m_CamberLimit != 0.0f)
    {
        this->dword_180 =
            math::Sign(m_LocalOffset.X) *
            m_SuspensionValue *
            m_CamberLimit;
    }

    // Axle logic
    if (m_AxleLimit <= 0.0f)
    {
        // Free-spinning axle
        m_AxleValue = fmod(datTimeManager::GetSeconds() * m_SomeAngularVelocity + m_AxleValue, math::PI2);
    }
    else
    {
        float target = 0.0f;

        if (m_OnGround)
        {
            float contactAngle = asin(m_ContactMatrix.m21);
            float wheelAngle = asin(m_GlobalMatrixWheelCenter.m21);

            target = -(contactAngle - wheelAngle);

            if (target < -m_AxleLimit)
            {
                math::Approach(
                    m_AxleValue,
                    -m_AxleLimit,
                    4.0f,
                    datTimeManager::GetSeconds());

                goto VISUALS_ONLY; // exact IDA early-out
            }

            if (target > m_AxleLimit)
                target = m_AxleLimit;
        }

        math::Approach(
            m_AxleValue,
            target,
            4.0f,
            datTimeManager::GetSeconds());
    }

VISUALS_ONLY:
    // ------------------------------------------------------------
    // IDA: LABEL_80 — visual wheel placement
    // ------------------------------------------------------------

    float clampedLat = -m_TireDispLimitLat;
    float limit = m_TireDispLimitLat;

    if (m_TireDispLat >= clampedLat)
    {
        if (m_TireDispLat <= limit)
            clampedLat = m_TireDispLat;
        else
            clampedLat = limit;
    }

    float lateralOffset = clampedLat * -0.1f;

    m_GlobalMatrixWheelCenter.m30 += lateralOffset * m_GlobalMatrixWheelCenter.m00;
    m_GlobalMatrixWheelCenter.m31 += lateralOffset * m_GlobalMatrixWheelCenter.m01;
    m_GlobalMatrixWheelCenter.m32 += lateralOffset * m_GlobalMatrixWheelCenter.m02;

    float verticalOffset = m_SuspensionValue - GetVisualDispVert();

    m_GlobalMatrixWheelCenter.m30 += verticalOffset * m_GlobalMatrixWheelCenter.m10;
    m_GlobalMatrixWheelCenter.m31 += verticalOffset * m_GlobalMatrixWheelCenter.m11;
    m_GlobalMatrixWheelCenter.m32 += verticalOffset * m_GlobalMatrixWheelCenter.m12;
}
