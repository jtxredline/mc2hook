#include "rstate.h"
#include <mc2hook\mc2hook.h>
#include <age/vector/matrix44.h>
#include <age/core/output.h>

declfield(gfxState::sm_Camera) = 0x858548;
declfield(gfxState::sm_View) = 0x858440;
declfield(gfxState::lpD3DDevice) = 0x85836C;
declfield(gfxState::sm_Cull) = 0x858430;
declfield(gfxState::s_LastCull) = 0x859BEC;
declfield(gfxState::s_CullInitialized) = 0x859BE8;

void gfxState::SetCamera(Matrix44 const& mtx)
{
    // Mirror view
    Matrix44 mirrorMtx = mtx;

    static bool mirrorMode = HookConfig::GetBool("Graphics", "MirrorMode", false);
    if (mirrorMode == true)
    {
        mirrorMtx.m00 *= -1.0f;
        mirrorMtx.m01 *= -1.0f;
        mirrorMtx.m02 *= -1.0f;
    }

    static_cast<Matrix44&>(sm_Camera).Set(mtx);
    static_cast<Matrix44&>(sm_View).FastInverse(mirrorMtx); // mtx
    hook::StaticThunk<0x5EDB60>::Call<void>(); // sub_5EDB60()
    lpD3DDevice->SetTransform(D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(&sm_View));

    //hook::StaticThunk<0x5EE8D0>::Call<void>(&mtx);
}

void gfxState::SetCull(int mode)
{   
    static bool mirrorMode = HookConfig::GetBool("Graphics", "MirrorMode", false);
    if (mirrorMode == true)
    {
        mode = static_cast<int>(gfxCullMode::CCW); // Override mode, gfxCullMode::None fixes trees, but breaks many thin building walls
    }

    sm_Cull = mode;

    // Only update D3D if state changed or not initialized yet
    if (s_LastCull != mode || !s_CullInitialized)
    {
        s_LastCull = mode;
        s_CullInitialized = true;

        lpD3DDevice->SetRenderState(D3DRS_CULLMODE, mode);
    }

    //hook::StaticThunk<0x5EE1A0>::Call<void>(&mode);
}

void gfxState::SetCamera34(Matrix34 const& mtx)
{
    // Mirror view
    Matrix34 mirrorMtx = mtx;

    static bool mirrorMode = HookConfig::GetBool("Graphics", "MirrorMode", false);
    if (mirrorMode == true)
    {
        mirrorMtx.m00 *= -1.0f;
        mirrorMtx.m01 *= -1.0f;
        mirrorMtx.m02 *= -1.0f;
    }

    //Matrix44 m;
    //m.FromMatrix34(mtx);
    //gfxState::SetCamera(m);

    hook::StaticThunk<0x5EE900>::Call<void>(&mirrorMtx); // &mtx
}

Matrix44 gfxState::GetCameraMatrix() {
    return gfxState::sm_Camera;
}

void gfxState::SetWorld(Matrix44 const& mtx) {
    hook::StaticThunk<0x5EDB00>::Call<void>(&mtx);
}

void gfxState::SetWorld34(Matrix34 const& mtx) {
    hook::StaticThunk<0x5EDB20>::Call<void>(&mtx);
}

//void gfxState::SetLighting(uint8_t const& val) // Signature might be broken
//{
//    hook::StaticThunk<0x5EDF80>::Call<void>(&val);
//}

void gfxState::SetFogStart(float val) {

    static float fogStartMultiplier = HookConfig::GetFloat("Graphics", "FogStartMultiplier", 1.0f);
    float modVal = val * fogStartMultiplier;
    hook::StaticThunk<0x5EE840>::Call<void>(modVal);
}

void gfxState::SetFogEnd(float val) {
    static float fogEndMultiplier = HookConfig::GetFloat("Graphics", "FogEndMultiplier", 1.0f);
    float modVal = val * fogEndMultiplier;
    hook::StaticThunk<0x5EE880>::Call<void>(modVal);
}
