#include "wheel.h"
#include <mc2hook\mc2hook.h>
#include <age/data/timemgr.h>
#include <age/math/math.h>
#include <age/vehicle/carsim.h>
#include <age/vehicle/transmission.h>
#include <age/physics/phcollider.h>
#include <age/physics/phsurface.h>
#include <age/physics/phlevel.h>
#include <age/physics/intersection.h>
#include <age/physics/simulator.h>

#include <age/core/output.h> //
#include <age/input/keyboard.h> //
#include <dinput.h> //

declfield(vehWheel::byte_6C5210)(0x6C5210);
declfield(vehWheel::byte_6C5211)(0x6C5211);
declfield(vehWheel::WeatherFriction)(0x2B07F8);

void vehWheel::UpdateSuspensionRay()
{
    //hook::Thunk<0x56D840>::Call<void>(this); // Call original
    //return;
    
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
    //hook::Thunk<0x56DAD0>::Call<void>(this, disp, dispTarget, step, limit, outStep, outFree); // Call original

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
    //return hook::Thunk<0x56D3C0>::Call<float>(this, slipPercent, outSlipRatio); // Call original
    
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
    return hook::Thunk<0x56D0C0>::Call<float>(this); // Call original

    // TODO: Double check code below

    //float maxDisp = m_Radius * 0.2f;
    //float disp = m_Radius * 0.05f / m_NormalLoad * m_SomeTireLoad;

    //if (disp < 0.0f)
    //    return 0.0f;

    //return (disp <= maxDisp) ? disp : maxDisp;
}

void vehWheel::SetInputs(float steering, float brake, float handbrake)
{
    //hook::Thunk<0x56E670>::Call<void>(this, steering, brake, handbrake); // Call original

    float baseAngle = -steering * m_SteeringLimit;

    float sideSign = math::Sign(m_LocalOffset.X);

    m_SteeringAngle = (1.0f - baseAngle * m_SteeringOffset * sideSign) * baseAngle;

    m_BrakeValue = brake * m_BrakeTorque + handbrake * m_HandbrakeTorque;
}

void vehWheel::SetBrake(float brake)
{
    m_BrakeValue = m_StaticFric * m_NormalLoad * m_Radius * brake;
}

void vehWheel::SetNormalLoad(float load)
{
    hook::Thunk<0x56D260>::Call<void>(this, load); // Call original
}

void vehWheel::ComputeConstants()
{
    hook::Thunk<0x56EAC0>::Call<void>(this); // Call original
}

float vehWheel::CalcSuspensionTarget(float a2, float a3)
{
    return hook::Thunk<0x56D4D0>::Call<float>(this, a2, a3); // Call original

    // TODO: Double check code below

    float limitOffset;
    float angleAdjustedRadius;
    float maxReach;
    float scaledExtent;

    maxReach = this->m_Radius * 1.414f;
    limitOffset = this->m_SuspensionLimit + 0.5f;
    scaledExtent = (maxReach + this->m_SuspensionExtent + limitOffset) * a2 + this->m_SurfaceDepth;
    angleAdjustedRadius = this->m_Radius / a3;

    if (angleAdjustedRadius > maxReach)
        angleAdjustedRadius = maxReach;

    return limitOffset + angleAdjustedRadius - scaledExtent;
}

void vehWheel::CalcSuspensionForce(float newSuspValue, bool collided, float upright, float bumpVelocity)
{        
    hook::Thunk<0x56D530>::Call<void>(this, newSuspValue, collided, upright, bumpVelocity); // Call original
    return;

    // TODO: Double check code below
    
    float lastSuspValue = this->m_SuspensionValue;
    float minSuspValue = -this->m_SuspensionExtent;
    this->m_BottomedOut = false;

    // Set suspension value and determine if we're at the extension limit
    bool suspAtLimit = false;

    if (collided)
    {
        this->m_SuspensionValue = newSuspValue;

        if (newSuspValue < minSuspValue)
        {
            // Drooped past max extension - clamp and zero bump velocity
            m_SuspensionValue = minSuspValue;
            bumpVelocity = 0.0f;
            suspAtLimit = true;
        }
    }
    else
    {
        // Not touching anything — fully extend suspension
        m_SuspensionValue = minSuspValue;
        suspAtLimit = true;
    }

    // Suspension velocity (rate of compression change)
    float suspVelocity = (this->m_SuspensionValue - lastSuspValue) * datTimeManager::InvSeconds;
    this->m_SuspensionVelocity = suspVelocity;

    float progressiveFactor = this->m_SuspensionValue * this->field_174 + 1.0f;
    float tireLoad = this->m_SuspensionValue * this->m_SomeSuspensionForceOrLoad * progressiveFactor
        + (suspVelocity + bumpVelocity) * this->dword_178
        + this->m_NormalLoad;

    this->m_SomeTireLoad = tireLoad;

    // Negative load — wheel lifting off, suspension fully unloaded
    if (tireLoad < 0.0)
    {
        this->m_SomeTireLoad = 0.0f;

        float springDt = datTimeManager::Seconds * this->m_SomeSuspensionForceOrLoad;
        float equilibrium = (lastSuspValue * this->dword_178 - springDt * this->m_SuspensionExtent) / (springDt + this->dword_178);

        this->m_SuspensionValue = equilibrium;
        this->m_SuspensionVelocity = (equilibrium - lastSuspValue) * datTimeManager::InvSeconds;
        this->m_SomeNormalLoad = 0.0f;
        return;
    }

    bool hitBottomOut = false;
    float bottomOutImpulse = 0.0f;
    float bottomOutDamp = 0.0f;

    if (this->m_SuspensionValue > this->m_SuspensionLimit)
    {
        if (byte_6C5211)
        {
            // No-friction collision impulse to bounce wheel off hard limit
            float impulse = m_ICS->CalcCollisionNoFriction(&this->m_ContactMatrix.GetRow(1), this->m_ForceOrVelLat, &this->m_LastContactPosition);

            if (impulse > 0.0f)
            {
                hitBottomOut = true;
                bottomOutImpulse = datTimeManager::InvSeconds * impulse * 0.25f;
                bottomOutDamp = -(this->m_SuspensionDampCoef / this->m_ForceOrVelLat * bottomOutImpulse);
            }
        }

        if (byte_6C5210)
        {
            // Push force proportional to over-compression depth along contact normal
            float overCompression = (this->m_SuspensionValue - this->m_SuspensionLimit) * upright;

            Vector3 pushOffset(0.0f, 0.0f, 0.0f);

            Vector3 pushForce = this->m_ContactMatrix.GetRow(1) * overCompression;

            m_ICS->ApplyPush(&pushForce, &pushOffset, 0xFFFFFFFF);
        }
        this->m_BottomedOut = true;
        goto LABEL_17;
    }

    // Drooped past maximum extension — clamp
    if (minSuspValue > this->m_SuspensionValue)
    {
        this->m_SuspensionValue = minSuspValue;

    LABEL_17:
        // Recalculate load after clamping — suspension value may have changed
        if (m_SuspensionValue > this->m_SuspensionLimit)
            this->m_SuspensionValue = this->m_SuspensionLimit;

        float clampedProgFactor = this->m_SuspensionValue * this->field_174 + 1.0f;
        float clampedVelocity = (this->m_SuspensionValue - lastSuspValue) * datTimeManager::InvSeconds;

        this->m_SuspensionVelocity = clampedVelocity;
        this->m_SomeTireLoad = m_SuspensionValue * this->m_SomeSuspensionForceOrLoad * clampedProgFactor
            + clampedVelocity * this->dword_178
            + this->m_NormalLoad;
        goto LABEL_20;
    }

    if (suspAtLimit)
    {
    LABEL_20:
        this->m_SomeNormalLoad = 0.0;

        if (hitBottomOut)
        {
            this->m_SomeTireLoad += bottomOutImpulse;
            this->m_SomeNormalLoad = bottomOutDamp;
        }

        goto LABEL_22;
    }

    // Normal rolling contact — compute normal load contribution for lateral force calculations
    this->m_SomeNormalLoad = this->m_SomeSuspensionForceOrLoad / upright * datTimeManager::Seconds
        + progressiveFactor * this->dword_178;

LABEL_22:
    // Clamp tire load to maximum the suspension can handle
    float maxTireLoad = this->m_SuspensionClamp * this->m_NormalLoad;
    if (maxTireLoad < this->m_SomeTireLoad)
        this->m_SomeTireLoad = maxTireLoad;
}

float vehWheel::GetBumpDisplacement(float a2)
{
    return hook::Thunk<0x56DA20>::Call<float>(this, a2); // Call original
}

// WIP
void vehWheel::ComputeDwtdw(float a2, float* a3, float* a4, float* a5)
{
    hook::Thunk<0x56EDB0>::Call<void>(this, a2, a3, a4, a5); // Call original

    // Vars
    Matrix34* p_m_SomeInstParentTransform; // eax
    double v8; // st7
    float Z; // ecx
    float dword_b4; // eax
    float dword_b8; // ecx
    float dword_bc; // eax
    float* p_m20; // edi
    long double v14; // st4
    long double v15; // st7
    double v19; // st7
    double v20; // st6
    double v21; // st5
    double v22; // st7
    phSurface* material; // eax
    long double v24; // st7
    //long double BumpDisplacement; // st7
    //phSurface* m_PhysMtl; // eax
    double v27; // st7
    double v28; // st7
    double v29; // st7
    double v30; // st6
    double v31; // st7
    double v32; // st7
    long double v33; // st6
    double v34; // st6
    double v35; // st6
    double v36; // st6
    double v37; // st6
    double v38; // st6
    unsigned __int8 v40; // c0
    unsigned __int8 v41; // c3
    char v42; // fps^1
    double m_WheelVelLong; // st5
    char v44; // ah
    bool v45; // c0
    bool v46; // c3
    double v47; // st7
    double m_Radius; // st7
    float newSuspValue; // [esp+Ch] [ebp-50h]
    float newSuspValuea; // [esp+Ch] [ebp-50h]
    bool v51; // [esp+2Fh] [ebp-2Dh]
    float v52; // [esp+30h] [ebp-2Ch]
    float v53; // [esp+34h] [ebp-28h]
    float m_OptimumSlipPercent; // [esp+34h] [ebp-28h]
    float v55; // [esp+34h] [ebp-28h]
    float v56; // [esp+38h] [ebp-24h]
    float v57; // [esp+38h] [ebp-24h]
    float upright; // [esp+3Ch] [ebp-20h]
    float uprighta; // [esp+3Ch] [ebp-20h]
    float uprightb; // [esp+3Ch] [ebp-20h]
    float dword_130; // [esp+40h] [ebp-1Ch]
    float v62; // [esp+40h] [ebp-1Ch]
    float m_SlipPercentLong; // [esp+40h] [ebp-1Ch]
    Vector3 contactVelocity; // [esp+44h] [ebp-18h] BYREF
    float v65; // [esp+50h] [ebp-Ch] BYREF
    float v66; // [esp+54h] [ebp-8h]
    float v67; // [esp+58h] [ebp-4h]
    float dispPerSecond; // [esp+6Ch] [ebp+10h]
    float a5b; // [esp+6Ch] [ebp+10h]
    float a5e; // [esp+6Ch] [ebp+10h]
    float a5c; // [esp+6Ch] [ebp+10h]
    float a5d; // [esp+6Ch] [ebp+10h]

    // Select reference transform
    if (m_CarSim)
        p_m_SomeInstParentTransform = &m_CarSim->m_Collider->m_SomeInstParent->m_SomeInstParentTransform;
    else
        p_m_SomeInstParentTransform = &this->m_ICS->m_WorldTransform;
    v51 = 0;
    upright = 1.0;

    if (this->m_HasIntersection)
    {
        // Compute how upright the wheel contact is relative to the car/body transform
        upright =
            p_m_SomeInstParentTransform->m12 * this->dword_bc +
            p_m_SomeInstParentTransform->m11 * this->dword_b8 +
            p_m_SomeInstParentTransform->m10 * this->dword_b4; // dot?

        // Reject upside-down or invalid contacts
        if (upright < 0.0f || fabs(this->dword_b8) < (flt_673E50 * 0.5f))
        {
            this->m_HasIntersection = false;
        }
        else
        {
            // Build contact matrix position
            this->m_ContactMatrix.m30 = this->m_LastContactPosition.X;
            this->m_ContactMatrix.m31 = this->m_LastContactPosition.Y;
            this->m_ContactMatrix.m32 = this->m_LastContactPosition.Z;

            // Set contact normal (row 1)
            this->m_ContactMatrix.m10 = this->dword_b4;
            this->m_ContactMatrix.m11 = this->dword_b8;
            this->m_ContactMatrix.m12 = this->dword_bc;

            // Build tangent vector (row 2)
            this->m_ContactMatrix.SetRow(2, Vector3::Cross(this->m_GlobalMatrixWheelCenter.GetRow(0), this->m_ContactMatrix.GetRow(1)));

            // Normalize tangent vector
            float tangentLenSq =
                this->m_ContactMatrix.m20 * this->m_ContactMatrix.m20 +
                this->m_ContactMatrix.m21 * this->m_ContactMatrix.m21 +
                this->m_ContactMatrix.m22 * this->m_ContactMatrix.m22;

            if (tangentLenSq < 0.0f)
            {
                this->m_HasIntersection = false;
            }
            else
            {
                float invTangentLen = 1.0f / sqrt(tangentLenSq);

                this->m_ContactMatrix.m20 *= invTangentLen;
                this->m_ContactMatrix.m21 *= invTangentLen;
                this->m_ContactMatrix.m22 *= invTangentLen;

                // Build final orthogonal basis vector (row 0)
                this->m_ContactMatrix.SetRow(0, Vector3::Cross(this->m_ContactMatrix.GetRow(1), this->m_ContactMatrix.GetRow(2)));

                // Detect near-vertical surface
                v51 = fabs(this->dword_b8) < flt_673E50;
            }
        }
    }

    // Compute relative contact velocity
    if (this->m_HasIntersection)
    {
        // Get wheel contact velocity in local space
        Vector3 contactVelocity;
        this->m_ICS->GetLocalFilteredVelocity2(
            &this->m_LastContactPosition,
            &contactVelocity);

        // Get collider at intersection point
        phCollider* collider = nullptr; // phLevel::Instance->GetCollider(this->m_Isect->m_LevelIndex);

        if (collider)
        {
            // Subtract collider surface velocity
            phInertialCS* colliderICS = collider->m_ICS;

            Vector3 surfaceVelocity(0.0f, 0.0f, 0.0f);

            if (colliderICS)
            {
                colliderICS->GetLocalVelocity(
                    &this->m_LastContactPosition,
                    &surfaceVelocity,
                    0.0f); // Original passed HIWORD(...)
            }

            contactVelocity -= surfaceVelocity;
        }
        else
        {
            // Fallback: subtract intersection object velocity
            Vector3 surfaceVelocity;

            // Disabling for now
            //this->m_Isect->sub_58FDD0(&this->m_LastContactPosition, &surfaceVelocity);

            contactVelocity -= surfaceVelocity;
        }

        // Project velocity into tire contact basis

        // Lateral wheel velocity
        this->m_WheelVelLat =
            contactVelocity.X * this->m_ContactMatrix.m00 +
            contactVelocity.Y * this->m_ContactMatrix.m01 +
            contactVelocity.Z * this->m_ContactMatrix.m02;

        // Longitudinal wheel velocity
        this->m_WheelVelLong =
            -(contactVelocity.X * this->m_ContactMatrix.m20 +
                contactVelocity.Y * this->m_ContactMatrix.m21 +
                contactVelocity.Z * this->m_ContactMatrix.m22);

        // Velocity/force along contact normal
        this->m_ForceOrVelLat =
            contactVelocity.X * this->m_ContactMatrix.m10 +
            contactVelocity.Y * this->m_ContactMatrix.m11 +
            contactVelocity.Z * this->m_ContactMatrix.m12;

        // Compute longitudinal slip velocity
        this->m_SlipVelLong = this->m_SomeAngularVelocity * this->m_Radius + this->m_WheelVelLong;

        // Surface / material lookup
        phSurface* material = hook::StaticThunk<0x5808C0>::Call<phSurface*>(m_Isect); // Call original//phLevel::Instance->GetMaterialFromIsect(m_Isect);

        if (!material || material->dword_04 != 2)
            material = phSurface::dword_6CE214;

        this->m_PhysMtl = material;

        //// Surface properties + bump response
        //float dispPerSecond = 0.0f;

        //if (material)
        //{
        //    float previousBumpDisp = this->dword_130;

        //    float wheelSpeed =
        //        sqrt(
        //            this->m_WheelVelLat * this->m_WheelVelLat +
        //            this->m_WheelVelLong * this->m_WheelVelLong);

        //    float scaledWheelSpeed = wheelSpeed;

        //    if (wheelSpeed > 1.0f && flt_673E54 != 1.0f)
        //    {
        //        scaledWheelSpeed = pow(scaledWheelSpeed, flt_673E54);
        //    }

        //    float bumpDisplacement = GetBumpDisplacement(scaledWheelSpeed);

        //    this->dword_130 = bumpDisplacement;

        //    dispPerSecond =
        //        (bumpDisplacement - previousBumpDisp) *
        //        datTimeManager::InvSeconds;

        //    // Copy material properties
        //    this->m_SurfaceDrag = m_PhysMtl->m_Drag;
        //    this->m_SurfaceFriction = m_PhysMtl->m_Friction;
        //    this->m_SurfaceDepth = m_PhysMtl->m_Depth;
        //    this->m_SurfaceHeight = m_PhysMtl->m_Height;
        //    this->m_SurfaceWidth = m_PhysMtl->m_Width;
        //}
        //else
        //{
        //    // Default surface properties
        //    this->dword_130 = 0.0f;
        //    this->m_SurfaceDrag = 0.0f;
        //    this->m_SurfaceFriction = 1.0f;
        //    this->m_SurfaceWidth = 1.0f;
        //    this->m_SurfaceHeight = 0.0f;
        //    this->m_SurfaceDepth = 0.0f;
        //}
    } //
        /*
        if (v51)                                  // Next block: Friction scaling
            this->m_SurfaceFriction = 0.0;
        if (this->m_SurfaceFriction > 0.0)
        {
            if (this->m_FrictionHandling >= 1.0)
                v27 = this->m_SurfaceFriction / this->m_FrictionHandling;
            else
                v27 = (1.0 - this->m_SurfaceFriction) * (1.0 - this->m_FrictionHandling) + this->m_SurfaceFriction;
            this->m_SurfaceFriction = v27;
        }
        v28 = vehWheel::WeatherFriction * this->m_SurfaceFriction;
        this->m_SurfaceFriction = v28;
        v53 = v28 * this->m_StaticFric;


        v29 = this->m_GlobalMatrixWheelCenter.m02 * this->m_ContactMatrix.m02// Suspension solver
            + this->m_GlobalMatrixWheelCenter.m01 * this->m_ContactMatrix.m01
            + this->m_GlobalMatrixWheelCenter.m00 * this->m_ContactMatrix.m00;
        v30 = this->m_ContactMatrix.m22 * this->m_GlobalMatrixWheelCenter.m22
            + this->m_ContactMatrix.m21 * this->m_GlobalMatrixWheelCenter.m21
            + this->m_ContactMatrix.m20 * this->m_GlobalMatrixWheelCenter.m20;
        if (v30 >= v29)
        {
            v56 = v29;
        }
        else
        {
            v62 = v30;
            v56 = v62;
        }
        newSuspValue = vehWheel::CalcSuspensionTarget(this, this->dword_c0, v56);
        vehWheel::CalcSuspensionForce(this, newSuspValue, 1, upright, dispPerSecond);


        a5e = this->m_SuspensionValue - this->m_Radius;// Visual wheel placement
        v31 = a5e - vehWheel::GetVisualDispVert(this);
        this->m_ContactMatrix.m30 = v31 * this->m_GlobalMatrixWheelCenter.m10 + this->m_GlobalMatrixWheelCenter.m30;
        this->m_ContactMatrix.m31 = v31 * this->m_GlobalMatrixWheelCenter.m11 + this->m_GlobalMatrixWheelCenter.m31;
        this->m_ContactMatrix.m32 = v31 * this->m_GlobalMatrixWheelCenter.m12 + this->m_GlobalMatrixWheelCenter.m32;


        v32 = (datTimeManager::InvSeconds * this->m_DampingLong + this->m_StiffnessLong) * this->m_Radius * this->m_Radius;// Long tire grip solver
        a5c = v32;
        v33 = fabs(this->m_WheelVelLong);
        if (v33 < 0.0000001
            || (v52 = 1.0 / this->m_OptimumSlipPercent / v33 * this->m_SomeTireLoad * v53 * this->m_Radius * this->m_Radius,
                v32 <= v52))
        {
            v34 = datTimeManager::InvSeconds * this->m_SomeTireLoad * v53 / (this->m_StiffnessLong * this->m_Radius);
            uprighta = v34;
            if (a2 >= 0.0)
            {
                if (v34 >= this->m_SomeAngularVelocity)
                {
                    v37 = -v34;
                    if (v37 >= this->m_SomeAngularVelocity)
                    {
                        *a4 = -1000000000.0;
                        *a5 = a5c;
                    }
                    else
                    {
                        *a4 = v37;
                        *a5 = 0.0;
                    }
                }
                else
                {
                    *a4 = v34;
                    *a5 = a5c;
                }
            }
            else
            {
                v35 = -v34;
                if (v35 <= this->m_SomeAngularVelocity)
                {
                    if (this->m_SomeAngularVelocity >= uprighta)
                    {
                        *a4 = 100000000.0;
                        *a5 = a5c;
                    }
                    else
                    {
                        *a4 = uprighta;
                        *a5 = 0.0;
                    }
                }
                else
                {
                    *a4 = v35;
                    *a5 = a5c;
                }
            }
            goto LABEL_87;
        }
        v32 = v52;
        m_OptimumSlipPercent = this->m_OptimumSlipPercent;
        a5d = -(this->m_WheelVelLong / this->m_Radius);
        uprightb = (1.0 - m_OptimumSlipPercent) * a5d;
        v57 = (m_OptimumSlipPercent + 1.0) * a5d;
        v36 = -m_OptimumSlipPercent;
        m_SlipPercentLong = this->m_SlipPercentLong;
        if (m_SlipPercentLong >= v36)
        {
            if (m_SlipPercentLong <= m_OptimumSlipPercent)
                v36 = m_SlipPercentLong;
            else
                v36 = m_OptimumSlipPercent;
        }
        v38 = (1.0 - v36) * a5d;
        if (v40 | v41)
        {
            if (a5d >= v38)
                v55 = v38;
            else
                v55 = -(this->m_WheelVelLong / this->m_Radius);
            if (a5d > v38)
                goto LABEL_63;
        }
        else
        {
            if (a5d <= v38)
                v55 = v38;
            else
                v55 = -(this->m_WheelVelLong / this->m_Radius);
            if (a5d < v38)
                LABEL_63:
            v38 = a5d;
        }
        m_WheelVelLong = this->m_WheelVelLong;
        v44 = v42;
        v45 = m_WheelVelLong < 0.0;
        v46 = m_WheelVelLong == 0.0;
        if (__SETP__(v44 & 5, 0))
        {
            if (v45 || v46)
            {
                if (v38 < this->m_SomeAngularVelocity)
                {
                    *a4 = v38;
                    *a5 = v52;
                    goto LABEL_87;
                }
                if (this->m_SomeAngularVelocity > uprightb)
                {
                    *a4 = uprightb;
                    *a5 = 0.0;
                    goto LABEL_87;
                }
            }
            else
            {
                if (this->m_SomeAngularVelocity > v55)
                {
                    *a4 = v55;
                    *a5 = v52;
                    goto LABEL_87;
                }
                if (this->m_SomeAngularVelocity > v57)
                {
                    *a4 = v57;
                    *a5 = 0.0;
                    goto LABEL_87;
                }
            }
            *a4 = -9.9999998e10;
        }
        else
        {
            if (v45 || v46)
            {
                if (this->m_SomeAngularVelocity >= v55)
                {
                    if (this->m_SomeAngularVelocity >= v57)
                    {
                        *a4 = 9.9999998e10;
                        *a5 = v52;
                    }
                    else
                    {
                        *a4 = v57;
                        *a5 = 0.0;
                    }
                }
                else
                {
                    *a4 = v55;
                    *a5 = v52;
                }
                goto LABEL_87;
            }
            if (v38 <= this->m_SomeAngularVelocity)
            {
                if (this->m_SomeAngularVelocity >= uprightb)
                {
                    *a4 = 9.9999998e10;
                    *a5 = v52;
                }
                else
                {
                    *a4 = uprightb;
                    *a5 = 0.0;
                }
                goto LABEL_87;
            }
            *a4 = v38;
        }
        *a5 = v52;
    LABEL_87:


        if (0.0 == *a5)                           // Final contraint outputs
            *a3 = v32;
        else
            *a3 = 0.0;
        if (this->m_SomeTireLoad == 0.0)
            LOBYTE(this->m_HasIntersection) = 0;
    }


    if (!LOBYTE(this->m_HasIntersection))       // Airborne / no contact fallback
    {
        newSuspValuea = -this->m_SuspensionExtent;
        vehWheel::CalcSuspensionForce(this, newSuspValuea, 0, 0.0, 0.0);
        *a5 = 0.0;
        *a3 = 0.0;
        if (a2 <= 0.0)
        {
            if (a2 >= 0.0)
                v47 = 0.0;
            else
                v47 = -1.0;
        }
        else
        {
            v47 = 1.0;
        }
        *a4 = v47 * -1.0e10;
        this->m_ContactMatrix = this->m_GlobalMatrixWheelCenter;
        m_Radius = this->m_Radius;
        this->m_ContactMatrix.m30 = this->m_ContactMatrix.m30 - m_Radius * this->m_GlobalMatrixWheelCenter.m10;
        this->m_ContactMatrix.m31 = this->m_ContactMatrix.m31 - m_Radius * this->m_GlobalMatrixWheelCenter.m11;
        this->m_ContactMatrix.m32 = this->m_ContactMatrix.m32 - m_Radius * this->m_GlobalMatrixWheelCenter.m12;
    }*/
}

void vehWheel::Update()
{
    // Wheel not in contact with ground
    if (!this->m_HasIntersection)
    {
        this->m_OnGround = 0;
        this->m_SlidingStrength = 0.0f;
        this->m_IsSliding = 0;
        this->m_TireDispLat = 0.0f;
        this->m_TireDispLong = 0.0f;
        this->m_WheelDriveTorque = 0.0f;
        this->m_TireForceLong = 0.0f;
        this->m_TireForceLat = 0.0f;
        UpdateVisuals();
        return;
    }

    // Setup wheel/contact state
    this->m_OnGround = 1;

    // Contact point relative to car transform
    Vector3 relContactPos = m_ContactMatrix.GetRow(3) - m_CarSim->m_Collider->m_SomeInstParent->m_SomeInstParentTransform.GetRow(3);

    float comX = m_ICS->m_WorldTransform.m00 * m_CarSim->m_CenterOfMass.X
               + m_ICS->m_WorldTransform.m10 * m_CarSim->m_CenterOfMass.Y
               + m_ICS->m_WorldTransform.m20 * m_CarSim->m_CenterOfMass.Z;

    float comY = m_ICS->m_WorldTransform.m01 * m_CarSim->m_CenterOfMass.X
               + m_ICS->m_WorldTransform.m11 * m_CarSim->m_CenterOfMass.Y
               + m_ICS->m_WorldTransform.m21 * m_CarSim->m_CenterOfMass.Z;

    float comZ = m_ICS->m_WorldTransform.m02 * m_CarSim->m_CenterOfMass.X
               + m_ICS->m_WorldTransform.m12 * m_CarSim->m_CenterOfMass.Y
               + m_ICS->m_WorldTransform.m22 * m_CarSim->m_CenterOfMass.Z;

    relContactPos.X -= comX;
    relContactPos.Y -= comY;
    relContactPos.Z -= comZ;

    // Apply suspension/contact load
    if (this->m_SomeNormalLoad > 0.0f)
    {
        Matrix34 contactResponse;
        
        float normalLoad = this->m_SomeNormalLoad;

        contactResponse.SetRow(3, Vector3(0.0f, 0.0f, 0.0f));

        float nX = this->m_ContactMatrix.m10;
        float nY = this->m_ContactMatrix.m11;
        float nZ = this->m_ContactMatrix.m12;

        float loadX = normalLoad * nX;
        float loadY = normalLoad * nY;

        contactResponse.m00 = loadX * nX;
        contactResponse.m01 = loadX * nY;
        contactResponse.m02 = loadX * nZ;

        contactResponse.m10 = loadX * nY;
        contactResponse.m11 = loadY * nY;
        contactResponse.m12 = loadY * nZ;

        contactResponse.m20 = loadX * nZ;
        contactResponse.m21 = loadY * nZ;
        contactResponse.m22 = normalLoad * nZ * nZ;

        // Suspension force
        Vector3 suspensionForce;

        if (this->m_SuspensionValue >= this->m_SuspensionLimit)
        {
            suspensionForce = Vector3(0.0f, 0.0f, 0.0f);
        }
        else
        {
            float suspensionLoad = this->m_SomeSuspensionForceOrLoad * this->m_SuspensionVelocity * datTimeManager::GetSeconds();

            suspensionForce = m_ContactMatrix.GetRow(1) * suspensionLoad;
        }

        m_ICS->ApplyContactForce(
            &suspensionForce,
            &this->m_ContactMatrix.GetRow(3),
            &contactResponse,
            &relContactPos);
    }

    // Wheel slip setup
    float wheelSurfaceVel = this->m_SomeAngularVelocity * this->m_Radius;

    this->m_SlipVelLong = wheelSurfaceVel + m_WheelVelLong;

    float absWheelVelLat = fabs(this->m_WheelVelLat);
    float absWheelVelLong = fabs(this->m_WheelVelLong);

    ComputeSlipPercent(&this->m_SlipPercentLat, m_WheelVelLat, m_WheelVelLong);
    ComputeSlipPercent(&this->m_SlipPercentLong, this->m_SlipVelLong, m_WheelVelLong);
    
    // Tire displacement targets
    float slipSignLong = math::Sign(this->m_SlipVelLong);
    float slipSignLat = math::Sign(this->m_WheelVelLat);

    float targetDispLong = slipSignLong / this->m_StiffnessLong * this->m_SomeTireLoad;
    float targetDispLat = slipSignLat / this->m_StiffnessLat * this->m_SomeTireLoad;

    float slipTravelLat = datTimeManager::GetSeconds() * this->m_WheelVelLat;
    float slipTravelLong = datTimeManager::GetSeconds() * this->m_SlipVelLong;

    // Friction setup
    float staticFriction = this->m_StaticFric * this->m_SurfaceFriction;

    float frictionLong = staticFriction;
    float targetDispLongScaled = frictionLong * targetDispLong;

    float absSlipLat = fabs(m_SlipPercentLat);
    float absSlipLong = fabs(m_SlipPercentLong);

    float slideStrengthLong;
    float slideStrengthLat;

    // Longitudinal friction
    if (absSlipLong <= this->m_OptimumSlipPercent)
        goto LABEL_21;

    if (m_TireDispLong >= m_OptimumSlipPercent)
    {
        if (m_TireDispLong <= targetDispLongScaled || targetDispLongScaled - this->m_TireDispLong > slipTravelLong)
            goto LABEL_21;
    }
    else if (m_TireDispLong >= targetDispLongScaled || targetDispLongScaled - this->m_TireDispLong < slipTravelLong)
    {
    LABEL_21:
        frictionLong = ComputeFriction(m_SlipPercentLong, &slideStrengthLong);
        targetDispLongScaled = frictionLong * targetDispLong;
        goto LABEL_22;
    }

    slideStrengthLong = 0.0f;

LABEL_22:

    // Lateral friction
    float frictionLat = staticFriction;
    float targetDispLatScaled = frictionLat * targetDispLat;

    if (absSlipLat <= this->m_OptimumSlipPercent)
    {
    LABEL_26:
        frictionLat = ComputeFriction(m_SlipPercentLat, &slideStrengthLat);
        targetDispLatScaled = frictionLat * targetDispLat;
        goto LABEL_27;
    }

    if (m_TireDispLat >= m_OptimumSlipPercent)
    {
        if (m_TireDispLat <= targetDispLatScaled || targetDispLatScaled - this->m_TireDispLat > slipTravelLat)
            goto LABEL_26;
    }
    else if (m_TireDispLat >= targetDispLatScaled || targetDispLatScaled - this->m_TireDispLat < slipTravelLat)
    {
        goto LABEL_26;
    }

    slideStrengthLat = 0.0f;
    
LABEL_27:

    bool longitudinalDominant = 1;

    if (m_SlipPercentLat > absSlipLong)
    {
        absSlipLong = m_SlipPercentLat;

    LABEL_37:
        longitudinalDominant = 0;
        goto LABEL_38;
    }

    if (-m_SlipPercentLat > absSlipLong)
    {
        absSlipLong = -m_SlipPercentLat;
        goto LABEL_37;
    }

LABEL_38:

    if (absSlipLong < this->m_OptimumSlipPercent)
    {
        frictionLong = staticFriction;
    }
    else
    {
        float minFriction;

        if (longitudinalDominant)
            minFriction = frictionLong;
        else
            minFriction = frictionLat;

        if (frictionLat > minFriction)
            targetDispLatScaled = targetDispLat * minFriction;
        if (frictionLong > minFriction)
            targetDispLongScaled = targetDispLong * minFriction;

        frictionLong = minFriction;
    }

    // Tire relaxation / damping
    float relaxation = fabs(wheelSurfaceVel) * datTimeManager::GetSeconds() * LRelaxCoef;
    
    float dampingLong = 0.0f;
    float dampingLat = 0.0f;

    bool hitLongLimit;
    bool hitLatLimit;

    CalcDispAndDamp(&this->m_TireDispLong, targetDispLongScaled, slipTravelLong, relaxation, &dampingLong, &hitLongLimit);
    CalcDispAndDamp(&this->m_TireDispLat, targetDispLatScaled, slipTravelLat, relaxation, &dampingLat, &hitLatLimit);

    // Tire forces
    this->m_TireForceLat = -(this->m_TireDispLat * this->m_StiffnessLat)
        - dampingLat * datTimeManager::GetInvSeconds() * this->m_DampingLat;

    this->m_TireForceLong = -(this->m_StiffnessLong * this->m_TireDispLong) - dampingLong * datTimeManager::GetInvSeconds() * this->m_DampingLong;

    float combinedForceSq = this->m_TireForceLat * this->m_TireForceLat + m_TireForceLong * m_TireForceLong;

    float frictionLimit = frictionLong * this->m_SomeTireLoad; // Might not be frictionLong anymore
    float frictionLimitSq = frictionLimit * frictionLimit;

    if (combinedForceSq > frictionLimit * frictionLimit)
    {
        if ((hitLongLimit || absSlipLong <= this->m_OptimumSlipPercent)
            && (hitLatLimit || absSlipLat <= this->m_OptimumSlipPercent))
        {
            float scale = sqrt(frictionLimitSq / combinedForceSq);

            this->m_TireForceLat *= scale;
            this->m_TireForceLong *= scale;
        }
        else
        {
            float velScale = sqrt(frictionLimitSq / (this->m_SlipVelLong * this->m_SlipVelLong + this->m_WheelVelLat * this->m_WheelVelLat));

            this->m_TireForceLat = -velScale * this->m_WheelVelLat;
            this->m_TireForceLong = -velScale * this->m_SlipVelLong;
        }
    }

    // Sliding state
    if (hitLongLimit)
    {
        if (hitLatLimit)
            this->m_SlidingStrength = 0.0f;
        else
            this->m_SlidingStrength = slideStrengthLat;
    }
    else if (hitLatLimit)
    {
        this->m_SlidingStrength = slideStrengthLong;
    }
    else if (slideStrengthLat <= slideStrengthLong)
    {
        this->m_SlidingStrength = slideStrengthLong;
    }
    else
    {
        this->m_SlidingStrength = slideStrengthLat;
    }

    this->m_IsSliding = this->m_SlidingStrength > 0.5f;

    // Build final tire force vector
    this->m_WheelDriveTorque = m_TireForceLong * this->m_Radius;

    float surfaceDrag = this->m_SurfaceDrag * this->m_SomeTireLoad;

    float lateralForce = this->m_TireForceLat - this->m_TireDragCoefLat * this->m_WheelVelLat * surfaceDrag * absWheelVelLat;

    Vector3 tireForce = m_ContactMatrix.GetRow(0) * lateralForce;

    float longitudinalDrag = surfaceDrag * this->m_TireDragCoefLong * this->m_WheelVelLong * absWheelVelLong * (this->m_SurfaceDepth + 1.0f);
    float longitudinalForce = -this->m_TireForceLong + longitudinalDrag;

    //v93 = -tireForce.X;
    //v94 = -tireForce.Y;
    //v95 = -tireForce.Z;
    //m_Mass = m_ICS->m_Mass;
    //LOWORD(m_ICS) = m_Isect->m_LevelIndex;
    //v82 = m_Mass;
    //if (phSimulator::ApplyForce(simulator_6C8EEC, m_ICS, &v93, &this->m_LastContactPosition.X, m_Mass))
    //{
    //    tireForce.X = -v93;
    //    tireForce.Y = -v94;
    //    Z = -v95;
    //}
    //else
    //{
    //    Z = tireForce.Z;
    //}

    //// Replaces the above for now
    tireForce.X = longitudinalForce * this->m_ContactMatrix.m20 + tireForce.X;
    tireForce.Y = longitudinalForce * this->m_ContactMatrix.m21 + tireForce.Y;
    tireForce.Z = longitudinalForce * this->m_ContactMatrix.m22 + tireForce.Z;

    // or
    //tireForce = m_ContactMatrix.GetRow(2) * longitudinalForce + tireForce;
    ////

    tireForce = m_ContactMatrix.GetRow(1) * m_SomeTireLoad + tireForce;

    m_ICS->AddForce(&tireForce, &m_ContactMatrix.GetRow(3));

    Vector3 inertiaScale = this->m_CarSim->m_InertiaScale;

    float torqueX = (relContactPos.Y * tireForce.Z - relContactPos.Z * tireForce.Y) * inertiaScale.X;
    float torqueY = (relContactPos.Z * tireForce.X - relContactPos.X * tireForce.Z) * inertiaScale.Y;
    float torqueZ = (relContactPos.X * tireForce.Y - relContactPos.Y * tireForce.X) * inertiaScale.Z;

    m_ICS->m_Torque.X += torqueX;
    m_ICS->m_Torque.Y += torqueY;
    m_ICS->m_Torque.Z += torqueZ;

    UpdateVisuals();
}

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
        Update(); //hook::Thunk<0x56DB90>::Call<void>(this); // vehWheel::Update();

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
