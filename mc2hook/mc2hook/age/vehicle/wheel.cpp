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
    hook::Thunk<0x56DAD0>::Call<void>(this, disp, dispTarget, step, limit, outStep, outFree); // Call original

    //float nextDisp;
    //float clamped;

    //nextDisp = step + *disp;

    //if (step < 0.0f)
    //{
    //    if (dispTarget > nextDisp)
    //    {
    //        clamped = limit + *disp;
    //        if (dispTarget < clamped)
    //            clamped = dispTarget;

    //        *disp = clamped;
    //        *outStep = 0.0f;
    //        *outFree = false;
    //    }
    //    else
    //    {
    //        *disp = nextDisp;
    //        *outStep = step;
    //        *outFree = true;
    //    }
    //}
    //else if (dispTarget < nextDisp)
    //{
    //    clamped = *disp - limit;
    //    if (dispTarget > clamped)
    //        clamped = dispTarget;

    //    *disp = clamped;
    //    *outStep = 0.0f;
    //    *outFree = false;
    //}
    //else
    //{
    //    *disp = nextDisp;
    //    *outStep = step;
    //    *outFree = true;
    //}
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

// WIP
void vehWheel::Update()
{
    float* p_dword_00; // ecx
    double v4; // st7
    double v5; // st7
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
    double m_SlipVelLong; // st7
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

    // Wheel not in contact with ground
    if (!this->m_HasIntersection)
    {
        this->m_OnGround = 0;
        this->m_SlidingStrength = 0.0;
        this->m_IsSliding = 0;
        this->m_TireDispLat = 0.0;
        this->m_TireDispLong = 0.0;
        this->m_WheelDriveTorque = 0.0;
        this->m_TireForceLong = 0.0;
        this->m_TireForceLat = 0.0;
        UpdateVisuals();
        return;
    }

    // Setup wheel/contact state
    this->m_OnGround = 1;

    // Contact point relative to car transform
    Vector3 relContactPos = m_ContactMatrix.GetRow(3) - m_CarSim->m_Collider->m_SomeInstParent->m_SomeInstParentTransform.GetRow(3);

    float comX = m_ICS->m_WorldTransform.m20 * m_CarSim->m_CenterOfMass.Z
               + m_ICS->m_WorldTransform.m10 * m_CarSim->m_CenterOfMass.Y
               + m_ICS->m_WorldTransform.m00 * m_CarSim->m_CenterOfMass.X;

    float comY = m_ICS->m_WorldTransform.m01 * m_CarSim->m_CenterOfMass.X
               + m_ICS->m_WorldTransform.m21 * m_CarSim->m_CenterOfMass.Z
               + m_ICS->m_WorldTransform.m11 * m_CarSim->m_CenterOfMass.Y;

    float comZ = m_ICS->m_WorldTransform.m02 * m_CarSim->m_CenterOfMass.X
               + m_ICS->m_WorldTransform.m22 * m_CarSim->m_CenterOfMass.Z
               + m_ICS->m_WorldTransform.m12 * m_CarSim->m_CenterOfMass.Y;

    relContactPos.X -= comX;
    relContactPos.Y -= comY;
    relContactPos.Z -= comZ;

    // Apply suspension/contact load
    if (this->m_SomeNormalLoad > 0.0)
    {
        m10 = this->m_ContactMatrix.m10;
        m11 = this->m_ContactMatrix.m11;
        some_force.Z = this->m_ContactMatrix.m12;
        some_normal_load = this->m_SomeNormalLoad;
        memset(&contact_response.m30, 0, 0xC);
        some_force.X = some_normal_load * m10;
        v11 = some_normal_load * m11;
        v12 = some_force.X * this->m_ContactMatrix.m11;
        v13 = some_force.X * this->m_ContactMatrix.m12;
        tire_disp_limit_lat = v11 * this->m_ContactMatrix.m12;
        v14 = some_force.X * this->m_ContactMatrix.m10;
        contact_response.m21 = tire_disp_limit_lat;
        contact_response.m00 = v14;
        contact_response.m01 = v12;
        contact_response.m02 = v13;
        contact_response.m10 = v12;
        contact_response.m11 = v11 * this->m_ContactMatrix.m11;
        contact_response.m12 = tire_disp_limit_lat;
        contact_response.m20 = v13;
        contact_response.m22 = some_normal_load * some_force.Z * this->m_ContactMatrix.m12;
        if (this->m_SuspensionValue >= (double)this->m_SuspensionLimit)
        {
            memset(&some_force, 0, sizeof(some_force));
        }
        else
        {
            v15 = this->m_SomeSuspensionForceOrLoad * this->m_SuspensionVelocity * datTimeManager::GetSeconds();
            some_force.X = v15 * this->m_ContactMatrix.m10;
            some_force.Y = v15 * this->m_ContactMatrix.m11;
            some_force.Z = v15 * this->m_ContactMatrix.m12;
        }
        m_ICS->ApplyContactForce(
            &some_force,
            (Vector3*)&this->m_ContactMatrix.m30,
            &contact_response,
            &relContactPos);
    }
    v16 = this->m_SomeAngularVelocity * this->m_Radius;
    vel_long = this->m_WheelVelLong;
    vel_lat = this->m_WheelVelLat;
    p_slip_percent_lat = &this->m_SlipPercentLat;
    this->m_SlipVelLong = v16 + vel_long;
    a4 = v16;
    v97 = fabs(this->m_WheelVelLat);
    v82 = fabs(this->m_WheelVelLong);
    ComputeSlipPercent(&this->m_SlipPercentLat, vel_lat, vel_long);
    slip_percent_long = &this->m_SlipPercentLong;
    ComputeSlipPercent(&this->m_SlipPercentLong, this->m_SlipVelLong, vel_long);
    m_SlipVelLong = this->m_SlipVelLong;
    
    if (m_SlipVelLong <= 0.0)
    {
        if (m_SlipVelLong >= 0.0)
            v20 = 0.0;
        else
            v20 = -1.0;
    }
    else
    {
        v20 = 1.0;
    }
    v74 = v20 / this->m_StiffnessLong * this->m_SomeTireLoad;
    v21 = this->m_WheelVelLat;
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
    a5 = v22 / this->m_StiffnessLat * this->m_SomeTireLoad;
    v88 = datTimeManager::GetSeconds() * this->m_WheelVelLat;
    v90 = datTimeManager::GetSeconds() * this->m_SlipVelLong;
    available_friction = this->m_StaticFric * this->m_SurfaceFriction;
    a6_2 = available_friction;
    a2 = available_friction * v74;
    tire_disp_limit_lat = fabs(*p_slip_percent_lat);
    abs_slip_percent_long = fabs(*slip_percent_long);
    v85 = abs_slip_percent_long;
    if (abs_slip_percent_long <= this->m_OptimumSlipPercent)
        goto LABEL_21;
    tire_disp_long = this->m_TireDispLong;
    if (tire_disp_long >= m_OptimumSlipPercent)
    {
        if (tire_disp_long <= a2 || a2 - this->m_TireDispLong > v90)
            goto LABEL_21;
    }
    else if (tire_disp_long >= a2 || a2 - this->m_TireDispLong < v90)
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
    if (tire_disp_limit_lat <= (double)this->m_OptimumSlipPercent)
    {
    LABEL_26:
        v33 = ComputeFriction(*p_slip_percent_lat, &some_normal_load);
        v91 = v33;
        v71 = v33 * a5;
        goto LABEL_27;
    }
    tire_disp_lat = this->m_TireDispLat;
    if (tire_disp_lat >= m_OptimumSlipPercent)
    {
        if (tire_disp_lat <= v71 || v71 - this->m_TireDispLat > v88)
            goto LABEL_26;
    }
    else if (tire_disp_lat >= v71 || v71 - this->m_TireDispLat < v88)
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
    if (v34 < this->m_OptimumSlipPercent)
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
    CalcDispAndDamp(&this->m_TireDispLong, a2, v90, a4, &a5, &a6_1);
    //p_tire_disp_lat = &this->m_TireDispLat;
    CalcDispAndDamp(&this->m_TireDispLat, v71, v88, v38, &v74, &a6);
    v40 = a6_1;
    v41 = a6;
    this->m_TireForceLat = -(this->m_TireDispLat * this->m_StiffnessLat)
        - v74 * datTimeManager::GetInvSeconds() * this->m_DampingLat;
    v42 = -(this->m_StiffnessLong * this->m_TireDispLong) - a5 * datTimeManager::GetInvSeconds() * this->m_DampingLong;
    this->m_TireForceLong = v42;
    v74 = this->m_TireForceLat * this->m_TireForceLat + v42 * v42;
    v43 = a6_2 * this->m_SomeTireLoad;
    v44 = v43 * v43;
    if (v74 > v43 * v43)
    {
        if ((v40 || v85 <= (double)this->m_OptimumSlipPercent)
            && (v41 || tire_disp_limit_lat <= (double)this->m_OptimumSlipPercent))
        {
            v46 = sqrt(v44 / v74);
            this->m_TireForceLat = v46 * this->m_TireForceLat;
            this->m_TireForceLong = v46 * this->m_TireForceLong;
        }
        else
        {
            v45 = sqrt(v44 / (this->m_SlipVelLong * this->m_SlipVelLong + this->m_WheelVelLat * this->m_WheelVelLat));
            this->m_TireForceLat = -v45 * this->m_WheelVelLat;
            this->m_TireForceLong = -v45 * this->m_SlipVelLong;
        }
    }
    if (v40)
    {
        if (v41)
            this->m_SlidingStrength = 0.0;
        else
            this->m_SlidingStrength = some_normal_load;
    }
    else if (v41)
    {
        this->m_SlidingStrength = a3;
    }
    else if (some_normal_load <= (double)a3)
    {
        this->m_SlidingStrength = a3;
    }
    else
    {
        this->m_SlidingStrength = some_normal_load;
    }
    tire_force_long = this->m_TireForceLong;
    this->m_IsSliding = this->m_SlidingStrength > 0.5;
    this->m_WheelDriveTorque = tire_force_long * this->m_Radius;
    v50 = this->m_SurfaceDrag * this->m_SomeTireLoad;
    v51 = this->m_TireForceLat - this->m_TireDragCoefLat * this->m_WheelVelLat * v50 * v97;
    v77.X = v51 * this->m_ContactMatrix.m00;
    v77.Y = v51 * this->m_ContactMatrix.m01;
    v52 = v51 * this->m_ContactMatrix.m02;
    v53 = -this->m_TireForceLong - -(v50 * this->m_TireDragCoefLong * this->m_WheelVelLong * v82) * (this->m_SurfaceDepth + 1.0);
    v77.X = v53 * this->m_ContactMatrix.m20 + v77.X;
    v77.Y = v53 * this->m_ContactMatrix.m21 + v77.Y;
    v77.Z = v53 * this->m_ContactMatrix.m22 + v52;
    v93 = -v77.X;
    v94 = -v77.Y;
    v95 = -v77.Z;

    v77.X = -v93;
    v77.Y = -v94;
    z = -v95;

    some_tire_load = this->m_SomeTireLoad;
    v77.X = some_tire_load * this->m_ContactMatrix.m10 + v77.X;
    v77.Y = some_tire_load * this->m_ContactMatrix.m11 + v77.Y;
    v77.Z = some_tire_load * this->m_ContactMatrix.m12 + z;
    m_ICS->AddForce(&v77, (Vector3*)&this->m_ContactMatrix.m30);
    p_dword_48 = &this->m_CarSim->dword_48;
    some_force.Z = v77.Y * relContactPos.X - relContactPos.Y * v77.X;
    some_force.X = (v77.Z * relContactPos.Y - v77.Y * relContactPos.Z) * p_dword_48->X;
    v59 = (relContactPos.Z * v77.X - v77.Z * relContactPos.X) * p_dword_48->Y;
    v60 = some_force.Z * p_dword_48->Z;
    p_torque = &this->m_ICS->m_Torque;
    p_torque->X = some_force.X + p_torque->X;
    p_torque->Y = v59 + p_torque->Y;
    p_torque->Z = v60 + p_torque->Z;

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
