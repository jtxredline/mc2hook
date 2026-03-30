#pragma once
#include <mc2hook\mc2hook.h>
#include <age/vector/matrix44.h>
#include <age/vector/matrix34.h>
#include <d3d9.h>

enum class gfxCullMode
{
    None = 1,
    CW = 2,
    CCW = 3
};

class gfxState
{
public:
    static hook::Type<Matrix44> sm_Camera; // TypeProxy?
    static hook::Type<Matrix44> sm_View;
    static hook::Type<int> sm_Cull; // gfxCullMode
    static hook::Type<int> s_LastCull; // gfxCullMode
    static hook::Type<bool> s_CullInitialized;
public:
    static hook::Type<IDirect3DDevice9*> lpD3DDevice;
public:
    static void SetCamera(Matrix44 const& mtx);
    static void SetCamera34(Matrix34 const& mtx);
    static void SetWorld(Matrix44 const& mtx);
    static void SetWorld34(Matrix34 const& mtx);
    //static void SetLighting(uint8_t const& val);
    static void SetFogStart(float val);
    static void SetFogEnd(float val);

    static Matrix44 GetCameraMatrix();
    
    static void SetCull(int mode);
};