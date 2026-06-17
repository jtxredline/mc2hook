#include "ShaderHandler.h"
#include <d3d9.h>
#include <cmath>

// Engine's global D3D device pointer at 0x85836C
static hook::Type<IDirect3DDevice9*> DevicePtr(0x85836C);

typedef HRESULT(WINAPI* CreateVertexShader_t)(IDirect3DDevice9*, const DWORD*, IDirect3DVertexShader9**);
typedef HRESULT(WINAPI* CreatePixelShader_t)(IDirect3DDevice9*, const DWORD*, IDirect3DPixelShader9**);
typedef HRESULT(WINAPI* SetVertexShader_t)(IDirect3DDevice9*, IDirect3DVertexShader9*);
typedef HRESULT(WINAPI* SetPixelShader_t)(IDirect3DDevice9*, IDirect3DPixelShader9*);
typedef HRESULT(WINAPI* SetVSConstF_t)(IDirect3DDevice9*, UINT, const float*, DWORD);
typedef HRESULT(WINAPI* SetRenderState_t)(IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD);

static CreateVertexShader_t  OrgCreateVertexShader        = nullptr;
static CreatePixelShader_t   OrgCreatePixelShader         = nullptr;
static SetVertexShader_t     OrgSetVertexShader           = nullptr;
static SetPixelShader_t      OrgSetPixelShader            = nullptr;
static SetVSConstF_t         OrgSetVertexShaderConstantF  = nullptr;
static SetRenderState_t      OrgSetRenderState            = nullptr;

// ps_3_0 disables fixed-function fog, so each upgraded PS lerps fog manually.
// We capture D3DRS_FOGCOLOR here and push it to the active PS's fog register.
// g_ActiveFogReg = the register the currently-bound upgraded PS reads fog from
// (PS_648B38=c2; -1 = none, incl. vanilla chrome which uses FFP fog). Lets a mid-pass
// FOGCOLOR change reach the cached active PS even when the PS cache suppresses a re-bind.
static float g_FogColor[4]  = { 0.5f, 0.5f, 0.5f, 1.0f };
static int   g_ActiveFogReg = -1;

// Cache of VS c0-c3 (LightColor0/1/2 + Ambient). Written by Hook_SetVertexShaderConstantF
// as the engine uploads them; read by Hook_SetPixelShader to push to PS c4-c7.
// Avoids GetVertexShaderConstantF which forces a GPU pipeline flush.
static float g_CachedVSConst[16] = {};  // 4 × float4

// ── Per-material Specular → PS c3 (route #2: scoped per-submesh callback shim) ─────────────────
// The engine forwards ONLY material Diffuse to a constant (c11). To get per-submesh Specular to
// the body PS WITHOUT a global engine patch, we shim the per-draw callback slot dword_85B34C
// (0x85B34C). In mode-3 the engine sets it to sub_5E9CA0 (per-material: c16 + sampler-0 bind) and
// invokes it per submesh — for EVERY submesh, textured or not — as cdecl(this[2]=D3DMATERIAL9*,
// this[0], this[1]) @ 0x5FB4BE. Our shim reads the live material's Specular (+0x20) → c3,
// then tail-calls the original.
// Scoped: the slot is ours only during the mode-3 body pass (the engine re-points it per pass via
// sub_5E9F80 / clears it via sub_5E9AA0), so rims/glass/road/FFP are untouched.
typedef void(__cdecl* PerMaterialCb_t)(void* material, void* baseTex, void* mask);
static PerMaterialCb_t g_OrgPerMaterialCb = nullptr;   // captured engine callback (sub_5E9CA0 in mode-3)

// PS_648B38's dynamic light loop reads WORLD-space light dirs (c8..c10)
// and colors (c5..c7) as constants, with the count in i0. The hook supplies all three:
//   • colors : forwarded from cached VS c0-c2 at PS bind (g_CachedVSConst).
//   • dirs   : computed here as normRow (c20-c22, inverse-world) × object-space light
//              dirs (c12-c14) = world-space dirs — the exact transform the old VS did
//              per-vertex — pushed to c8 when the engine sets the per-draw normal rows
//              (while the body PS is active). See UploadWorldSpaceLights().
//   • count  : SetPixelShaderConstantI(0, …) at PS bind (in-game directional cap = 2).
static float g_CachedLightDirW[12] = {};   // c12,c13,c14  OBJECT-space light dirs (negated)
static float g_CachedNormRow[12]   = {};   // c20,c21,c22  inverse-world normal rows (→ world)
static bool  g_BodyPSActive        = false;
static int   g_DirLightCount       = 2;    // i0; loaded from config in Install()

// ── Shader bytecode ──────────────────────────────────────────────────────────
// Creation-time bytecode swaps — zero runtime overhead per frame.
//
// Upgrades — BODY pair only:
//   0x6486F8  FresnelHeadlights VS  vs_1_1 → vs_3_0  geometry/material + world incident (lighting → PS)
//   0x648B38  projectedmaskcar PS   ps_1_1 → ps_3_0  IBL (world-anchored) + real rep-i0 light loop +
//                                                     Cook-Torrance direct specular + manual fog (c2)
//
// CHROME pair (0x675DF0 / 0x676088, System A CubeMap Fresnel) — swap infra kept, bytecode
// STUBBED ({0}) so HasBytecode()==false → the hook skips it → chrome renders VANILLA. It only
// draws the menu/garage preview (in-game bodies use the System B mode-3 pair above), and sharing
// the in-game [Shader] params (EnvironmentScale=15) blew the garage chrome out. Reverting it to
// stock fixes the preview with zero in-game impact. Re-enable by un-stubbing the arrays + adding
// the two shaders back to config.json. Road PSes (648A10/AA0/BB8) are also vanilla.
//
// Modified VS/PS pair:
//   0x6486F8 (VS_6486F8) ↔ 0x648B38 (PS_648B38) — headlight receiver (sub_512BB0 mode 3)
// ─────────────────────────────────────────────────────────────────────────────

// ── CubeMapFresnel VS — vs_2_0 (chrome; DORMANT, bytecode stubbed below) ─────
// Constants: c0-c3=lightColors/ambient, c4-c7=MVP, c10=ambientFloor, c11=material,
//            c12-c14=object lightDirs, c15=fog, c16=FresnelParams, c17-c19=WorldView, c20-c22=inverse-world
// Outputs: oPos, oFog, oD0(diffuse), oD1.w(Fresnel), oT0(baseUV), oT1(reflVec), oT2(N)
static const DWORD VS_675DF0[] = { 0 };  // STUBBED: chrome reverted to vanilla — HasBytecode()==false so no swap (removed from config.json so compile.ps1 will not refill)

// ── CubeMapFresnel PS — ps_2_a (chrome; FFP fog) ─────────────────────────────
static const DWORD PS_676088[] = { 0 };  // STUBBED: chrome reverted to vanilla — HasBytecode()==false so no swap (removed from config.json so compile.ps1 will not refill)

// ── FresnelHeadlights VS — vs_3_0 (geometry/material; lighting moved to PS) ──
// Constants: c4=MVP, c11=material, c15=fog, c16=shaderConst, c17-c19=WorldView,
//            c20-c22=inverse-world (normals), c23-c30=headlight projectors. No light loop.
// Outputs: oPos, COLOR1=Specular(material.rgb,shineMax), TEXCOORD0=baseUV,
//            TEXCOORD2/3=projA/B, TEXCOORD4=ViewNorm(world normal),
//            TEXCOORD5=IncidentDir(view .xyz, fog .w), TEXCOORD1=WorldInc(world incident).
static const DWORD VS_6486F8[] = {
    0xFFFE0300, 0x009EFFFE, 0x42415443, 0x0000001C,
    0x0000024C, 0xFFFE0300, 0x00000010, 0x0000001C,
    0x00000108, 0x00000245, 0x0000015C, 0x000F0002,
    0x003E0001, 0x00000168, 0x00000000, 0x00000178,
    0x00040002, 0x00120004, 0x00000180, 0x00000000,
    0x00000190, 0x000B0002, 0x002E0001, 0x00000168,
    0x00000000, 0x0000019B, 0x00140002, 0x00520001,
    0x00000168, 0x00000000, 0x000001A6, 0x00150002,
    0x00560001, 0x00000168, 0x00000000, 0x000001B1,
    0x00160002, 0x005A0001, 0x00000168, 0x00000000,
    0x000001BC, 0x00170002, 0x005E0001, 0x00000168,
    0x00000000, 0x000001CB, 0x00180002, 0x00620001,
    0x00000168, 0x00000000, 0x000001DA, 0x001A0002,
    0x006A0001, 0x00000168, 0x00000000, 0x000001E9,
    0x001B0002, 0x006E0001, 0x00000168, 0x00000000,
    0x000001F8, 0x001C0002, 0x00720001, 0x00000168,
    0x00000000, 0x00000207, 0x001E0002, 0x007A0001,
    0x00000168, 0x00000000, 0x00000216, 0x00100002,
    0x00420001, 0x00000168, 0x00000000, 0x00000224,
    0x00110002, 0x00460001, 0x00000168, 0x00000000,
    0x0000022F, 0x00120002, 0x004A0001, 0x00000168,
    0x00000000, 0x0000023A, 0x00130002, 0x004E0001,
    0x00000168, 0x00000000, 0x6F465F67, 0x72615067,
    0x00736D61, 0x00030001, 0x00040001, 0x00000001,
    0x00000000, 0x564D5F67, 0xABAB0050, 0x00030002,
    0x00040004, 0x00000001, 0x00000000, 0x614D5F67,
    0x69726574, 0x67006C61, 0x726F4E5F, 0x776F526D,
    0x5F670030, 0x6D726F4E, 0x31776F52, 0x4E5F6700,
    0x526D726F, 0x0032776F, 0x72505F67, 0x65546A6F,
    0x6F523178, 0x67003077, 0x6F72505F, 0x7865546A,
    0x776F5231, 0x5F670031, 0x6A6F7250, 0x31786554,
    0x57776F52, 0x505F6700, 0x546A6F72, 0x52327865,
    0x0030776F, 0x72505F67, 0x65546A6F, 0x6F523278,
    0x67003177, 0x6F72505F, 0x7865546A, 0x776F5232,
    0x5F670057, 0x64616853, 0x6F437265, 0x0074736E,
    0x69565F67, 0x6F527765, 0x67003077, 0x6569565F,
    0x776F5277, 0x5F670031, 0x77656956, 0x32776F52,
    0x5F737600, 0x00305F33, 0x7263694D, 0x666F736F,
    0x52282074, 0x4C482029, 0x53204C53, 0x65646168,
    0x6F432072, 0x6C69706D, 0x31207265, 0x00312E30,
    0x0200001F, 0x80000000, 0x900F0000, 0x0200001F,
    0x80000003, 0x900F0001, 0x0200001F, 0x80000005,
    0x900F0002, 0x0200001F, 0x80000000, 0xE00F0000,
    0x0200001F, 0x8001000A, 0xE00F0001, 0x0200001F,
    0x80000005, 0xE0030002, 0x0200001F, 0x80020005,
    0xE00F0003, 0x0200001F, 0x80030005, 0xE00F0004,
    0x0200001F, 0x80040005, 0xE0070005, 0x0200001F,
    0x80050005, 0xE00F0006, 0x0200001F, 0x80010005,
    0xE0070007, 0x03000009, 0xE0010000, 0xA0E40004,
    0x90E40000, 0x03000009, 0xE0020000, 0xA0E40005,
    0x90E40000, 0x03000009, 0xE0040000, 0xA0E40006,
    0x90E40000, 0x03000009, 0xE0080000, 0xA0E40007,
    0x90E40000, 0x03000009, 0x80040000, 0x90E40000,
    0xA0E40013, 0x04000004, 0xE0080006, 0x80AA0000,
    0xA055000F, 0xA000000F, 0x03000008, 0x80010001,
    0x90E40001, 0xA0E40014, 0x03000008, 0x80020001,
    0x90E40001, 0xA0E40015, 0x03000008, 0x80040001,
    0x90E40001, 0xA0E40016, 0x03000008, 0x80080001,
    0x80E40001, 0x80E40001, 0x02000007, 0x80080001,
    0x80FF0001, 0x03000005, 0xE0070005, 0x80FF0001,
    0x80E40001, 0x03000009, 0x80010000, 0x90E40000,
    0xA0E40011, 0x03000009, 0x80020000, 0x90E40000,
    0xA0E40012, 0x02000024, 0x80070001, 0x80E40000,
    0x03000005, 0x80070002, 0x80550001, 0xA0E40012,
    0x04000004, 0x80070002, 0xA0E40011, 0x80000001,
    0x80E40002, 0x04000004, 0x80070002, 0xA0E40013,
    0x80AA0001, 0x80E40002, 0x02000001, 0xE0070006,
    0x80E40001, 0x03000008, 0xE0010007, 0x80E40002,
    0xA0E40014, 0x03000008, 0xE0020007, 0x80E40002,
    0xA0E40015, 0x03000008, 0xE0040007, 0x80E40002,
    0xA0E40016, 0x02000001, 0x80080000, 0xA0000010,
    0x03000009, 0xE0010003, 0x80E40000, 0xA0E40017,
    0x03000009, 0xE0020003, 0x80E40000, 0xA0E40018,
    0x03000009, 0xE0080003, 0x80E40000, 0xA0E4001A,
    0x03000009, 0xE0010004, 0x80E40000, 0xA0E4001B,
    0x03000009, 0xE0020004, 0x80E40000, 0xA0E4001C,
    0x03000009, 0xE0080004, 0x80E40000, 0xA0E4001E,
    0x02000001, 0xE0070001, 0xA0E4000B, 0x02000001,
    0xE0080001, 0xA0AA0010, 0x02000001, 0xE0030002,
    0x90E40002, 0x02000001, 0xE0040003, 0xA0000010,
    0x02000001, 0xE0040004, 0xA0000010, 0x0000FFFF,
};


// ── projectedmaskcar PS — ps_3_0 (IBL, real rep-i0 light loop, manual fog c2) ──
// Genuine dynamic loop: count in i0 (SetPixelShaderConstantI), dirs c8-c10, colors c5-c7.
static const DWORD PS_648B38[] = {
    0xFFFF0300, 0x0092FFFE, 0x42415443, 0x0000001C,
    0x0000021B, 0xFFFF0300, 0x0000000C, 0x0000001C,
    0x00000108, 0x00000214, 0x0000010C, 0x00040002,
    0x00120001, 0x00000118, 0x00000000, 0x00000128,
    0x00020002, 0x000A0001, 0x00000118, 0x00000000,
    0x00000133, 0x00050002, 0x00160003, 0x00000140,
    0x00000000, 0x00000150, 0x00000001, 0x00000001,
    0x00000160, 0x00000000, 0x00000170, 0x00080002,
    0x00220003, 0x00000180, 0x00000000, 0x00000190,
    0x00030002, 0x000E0001, 0x00000118, 0x00000000,
    0x0000019E, 0x00000002, 0x00020001, 0x00000118,
    0x00000000, 0x000001A8, 0x00010002, 0x00060001,
    0x00000118, 0x00000000, 0x000001B2, 0x00000003,
    0x00020001, 0x000001BC, 0x00000000, 0x000001CC,
    0x00010003, 0x00060001, 0x000001D4, 0x00000000,
    0x000001E4, 0x00020003, 0x000A0001, 0x000001EC,
    0x00000000, 0x000001FC, 0x00030003, 0x000E0001,
    0x00000204, 0x00000000, 0x6D415F67, 0x6E656962,
    0xABAB0074, 0x00030001, 0x00040001, 0x00000001,
    0x00000000, 0x6F465F67, 0x6C6F4367, 0x6700726F,
    0x67694C5F, 0x6F437468, 0x00726F6C, 0x00030001,
    0x00040001, 0x00000003, 0x00000000, 0x694C5F67,
    0x43746867, 0x746E756F, 0xABABAB00, 0x00020000,
    0x00010001, 0x00000001, 0x00000000, 0x694C5F67,
    0x44746867, 0x53567269, 0xABABAB00, 0x00030001,
    0x00040001, 0x00000003, 0x00000000, 0x614D5F67,
    0x65705374, 0x616C7563, 0x5F670072, 0x61726150,
    0x0030736D, 0x61505F67, 0x736D6172, 0x5F730031,
    0x65736142, 0xABABAB00, 0x000C0004, 0x00010001,
    0x00000001, 0x00000000, 0x75435F73, 0xAB006562,
    0x000E0004, 0x00010001, 0x00000001, 0x00000000,
    0x614D5F73, 0x00416B73, 0x000C0004, 0x00010001,
    0x00000001, 0x00000000, 0x614D5F73, 0x00426B73,
    0x000C0004, 0x00010001, 0x00000001, 0x00000000,
    0x335F7370, 0x4D00305F, 0x6F726369, 0x74666F73,
    0x29522820, 0x534C4820, 0x6853204C, 0x72656461,
    0x6D6F4320, 0x656C6970, 0x30312072, 0xAB00312E,
    0x05000051, 0xA00F000B, 0xBF266666, 0x3F800000,
    0xBF800000, 0x00000000, 0x05000051, 0xA00F000C,
    0x42800000, 0xC1147AE1, 0xBF851EB8, 0x3F851EB8,
    0x05000051, 0xA00F000D, 0x40490FDB, 0xB727C5AC,
    0x47C35000, 0x3F000000, 0x05000051, 0xA00F000E,
    0x80000000, 0xBF800000, 0xC0000000, 0x3F800000,
    0x05000051, 0xA00F000F, 0xBF800000, 0xBCE147AE,
    0xBF126E98, 0x3CB43958, 0x05000051, 0xA00F0010,
    0x3F800000, 0x3D2E147B, 0x3F851EB8, 0xBD23D70A,
    0x0200001F, 0x8001000A, 0x900F0000, 0x0200001F,
    0x80000005, 0x90030001, 0x0200001F, 0x80020005,
    0x900F0002, 0x0200001F, 0x80030005, 0x900F0003,
    0x0200001F, 0x80040005, 0x90070004, 0x0200001F,
    0x80050005, 0x90080005, 0x0200001F, 0x80010005,
    0x90070006, 0x0200001F, 0x90000000, 0xA00F0800,
    0x0200001F, 0x98000000, 0xA00F0801, 0x0200001F,
    0x90000000, 0xA00F0802, 0x0200001F, 0x90000000,
    0xA00F0803, 0x03000042, 0x800F0000, 0x90E40001,
    0xA0E40800, 0x03010042, 0x800F0001, 0x90E40002,
    0xA0E40802, 0x03010042, 0x800F0002, 0x90E40003,
    0xA0E40803, 0x02000024, 0x80070003, 0x90E40004,
    0x03000008, 0x80080002, 0x91E40006, 0x91E40006,
    0x02000007, 0x80080002, 0x80FF0002, 0x03000005,
    0x80070004, 0x80FF0002, 0x91E40006, 0x02000024,
    0x80070005, 0x90E40006, 0x03000008, 0x80080003,
    0x80E40005, 0x80E40003, 0x03000002, 0x80080003,
    0x80FF0003, 0x80FF0003, 0x04000004, 0x80070005,
    0x80E40003, 0x81FF0003, 0x80E40005, 0x03000008,
    0x80180003, 0x80E40003, 0x80E40004, 0x03000005,
    0x80070006, 0x80E40000, 0x90E40000, 0x02000001,
    0x80180004, 0x90FF0000, 0x04000004, 0x80080004,
    0x80FF0004, 0xA000000B, 0xA055000B, 0x03000005,
    0x80080006, 0x80FF0004, 0x80FF0004, 0x03000005,
    0x80010007, 0x80FF0006, 0x80FF0006, 0x03000002,
    0x80020007, 0x81FF0004, 0xA055000B, 0x0300000B,
    0x80010008, 0x80550007, 0xA0000000, 0x03000002,
    0x80020007, 0x81FF0003, 0xA055000B, 0x03000005,
    0x80040007, 0x80550007, 0x80550007, 0x03000005,
    0x80040007, 0x80AA0007, 0x80AA0007, 0x03000005,
    0x80020007, 0x80AA0007, 0x80550007, 0x04000012,
    0x80010009, 0x80550007, 0x80000008, 0xA0000000,
    0x03000002, 0x80020007, 0x81000009, 0xA055000B,
    0x04000004, 0x80040007, 0x80FF0006, 0x80FF0006,
    0xA0AA000B, 0x02000001, 0x800A0008, 0xA0E4000B,
    0x03000005, 0x80080007, 0x80FF0003, 0x80FF0003,
    0x04000004, 0x80080006, 0x80FF0006, 0x81FF0006,
    0xA055000B, 0x04000004, 0x80080007, 0x80FF0007,
    0x80FF0006, 0x80000007, 0x02000007, 0x80080007,
    0x80FF0007, 0x02000006, 0x80080007, 0x80FF0007,
    0x02000001, 0x80070009, 0xA0FF000B, 0x02000001,
    0x8007000A, 0xA0FF000B, 0x02000001, 0x8001000B,
    0xA0FF000B, 0x01000026, 0xF0E40000, 0x03000002,
    0x800F000B, 0x8000000B, 0xA093000E, 0x04000058,
    0x8007000C, 0x8C55000B, 0xA0E40008, 0x80FF0008,
    0x04000058, 0x8007000C, 0x8CAA000B, 0xA0E40009,
    0x80E4000C, 0x04000058, 0x8007000C, 0x8CFF000B,
    0xA0E4000A, 0x80E4000C, 0x03000008, 0x80010008,
    0x80E40003, 0x80E4000C, 0x0300000B, 0x80080009,
    0x80000008, 0xA0FF000B, 0x04000058, 0x8007000D,
    0x8C55000B, 0xA0E40005, 0x80FF0008, 0x04000058,
    0x8007000D, 0x8CAA000B, 0xA0E40006, 0x80E4000D,
    0x04000058, 0x800E000B, 0x8CFF000B, 0xA0900007,
    0x8090000D, 0x04000004, 0x80070009, 0x80FF0009,
    0x80F9000B, 0x80E40009, 0x04000004, 0x8007000C,
    0x91E40006, 0x80FF0002, 0x80E4000C, 0x02000024,
    0x8007000D, 0x80E4000C, 0x03000008, 0x80010008,
    0x80E40003, 0x80E4000D, 0x0300000B, 0x8008000A,
    0x80000008, 0xA0FF000B, 0x03000008, 0x80010008,
    0x80E40004, 0x80E4000D, 0x03000005, 0x80040008,
    0x80FF000A, 0x80FF000A, 0x04000004, 0x80040008,
    0x80AA0008, 0x80AA0007, 0xA055000B, 0x03000005,
    0x80040008, 0x80AA0008, 0x80AA0008, 0x03000005,
    0x80040008, 0x80AA0008, 0xA000000D, 0x02000006,
    0x80040008, 0x80AA0008, 0x03000005, 0x80040008,
    0x80000007, 0x80AA0008, 0x03000002, 0x8008000A,
    0x81000008, 0xA055000B, 0x04000058, 0x80010008,
    0x80000008, 0x80FF000A, 0xA055000B, 0x03000005,
    0x8008000A, 0x80000008, 0x80000008, 0x03000005,
    0x8008000A, 0x80FF000A, 0x80FF000A, 0x03000005,
    0x80010008, 0x80000008, 0x80FF000A, 0x04000012,
    0x8008000A, 0x80000008, 0x80550008, 0xA0000000,
    0x03000005, 0x80010008, 0x80FF0009, 0x80FF0009,
    0x04000004, 0x80010008, 0x80000008, 0x80FF0006,
    0x80000007, 0x02000007, 0x80010008, 0x80000008,
    0x02000006, 0x80010008, 0x80000008, 0x03000005,
    0x80010008, 0x80FF0003, 0x80000008, 0x04000004,
    0x80010008, 0x80FF0009, 0x80FF0007, 0x80000008,
    0x03000002, 0x8001000C, 0x80000008, 0xA055000D,
    0x02000006, 0x80010008, 0x80000008, 0x04000058,
    0x80010008, 0x8000000C, 0x80000008, 0xA0AA000D,
    0x03000005, 0x80010008, 0x80000008, 0x80AA0008,
    0x03000005, 0x80010008, 0x80000008, 0xA0FF000D,
    0x0300000A, 0x8001000C, 0x80000008, 0xA000000C,
    0x03000005, 0x80010008, 0x80FF000A, 0x8000000C,
    0x03000005, 0x800E000B, 0x80E4000B, 0x80000008,
    0x04000004, 0x800E000B, 0x80E4000B, 0x80FF0009,
    0x8090000A, 0x04000058, 0x8007000A, 0x81FF0009,
    0x80E4000A, 0x80F9000B, 0x00000027, 0x03000002,
    0x80070003, 0x80E40009, 0xA0E40004, 0x03000005,
    0x80070003, 0x80E40006, 0x80E40003, 0x02000001,
    0x80040004, 0xA0AA0000, 0x03000005, 0x80070004,
    0x80AA0004, 0xA0E40003, 0x03000005, 0x80070004,
    0x80E40004, 0x80E4000A, 0x03000005, 0x80080005,
    0x80FF0004, 0xA0550001, 0x0300005F, 0x800F0005,
    0x80E40005, 0xA0E40801, 0x02000001, 0x800F0006,
    0xA0E4000F, 0x04000004, 0x800F0006, 0x80FF0004,
    0x80E40006, 0xA0E40010, 0x03000005, 0x80080002,
    0x80000006, 0x80000006, 0x03000005, 0x80080003,
    0x80FF0003, 0xA055000C, 0x0200000E, 0x80080003,
    0x80FF0003, 0x0300000A, 0x80080004, 0x80FF0003,
    0x80FF0002, 0x04000004, 0x80080002, 0x80FF0004,
    0x80000006, 0x80550006, 0x04000004, 0x80030006,
    0x80FF0002, 0xA0EE000C, 0x80EE0006, 0x04000004,
    0x80080002, 0xA0000000, 0x80000006, 0x80550006,
    0x03000005, 0x80070005, 0x80FF0002, 0x80E40005,
    0x03000005, 0x80070005, 0x80E40005, 0xA0550000,
    0x04000004, 0x80070003, 0x80550007, 0x80E40003,
    0x80E40005, 0x04000004, 0x80070001, 0x80E40002,
    0x80FF0001, 0x80E40001, 0x03000005, 0x80070001,
    0x80E40001, 0x80E40003, 0x04000004, 0x80070001,
    0x80E40001, 0xA0FF0000, 0x81E40003, 0x04000004,
    0x80070001, 0xA0000001, 0x80E40001, 0x80E40003,
    0x04000004, 0x80070000, 0x80E40004, 0x80E40000,
    0x80E40001, 0x02000001, 0x80110001, 0x90FF0005,
    0x03000002, 0x80070000, 0x80E40000, 0xA1E40002,
    0x04000004, 0x80170800, 0x80000001, 0x80E40000,
    0xA0E40002, 0x02000001, 0x80080800, 0x80FF0000,
    0x0000FFFF,
};

// ── Runtime shader constants ─────────────────────────────────────────────────
// The s1 env cube is the "envmap" (gfxCubemap); the s2 mask is "fx_headlight_mask"
// (projectedmaskcar pass). Exposed mc2hook.ini [Shader] keys: ShaderEnable, Reflectance,
// EnvmapStrength, SpecularIntensity, HeadlightMaskScale, HeadlightMaskOnBody.
// PS_648B38: c0 = Reflectance(F0) | EnvmapStrength | SpecularIntensity | HeadlightMaskScale
//            c1 = HeadlightMaskOnBody | MAX_MIP | (unused)
//            c3 = per-material Specular (dword_85B34C shim, D3DMATERIAL9 +0x20)
//            c4 = Ambient (VS c3)   c5-c7 = LightColor0-2 (VS c0-c2)   c8-c10 = world dirs
// PS_676088: c0 = FresnelScale | EnvironmentScale (DORMANT — chrome is vanilla/stubbed)
static float g_PS648B38[8] = {
 /* c0.x Reflectance (F0)       */ 0.04f,   // dielectric clearcoat F0; per-material Specular (c3) modulates
 /* c0.y EnvmapStrength         */ 6.00f,   // envmap (s1 env cube) reflection strength
 /* c0.z SpecularIntensity      */ 1.00f,   // direct Cook-Torrance spec scale (× material Specular c3)
 /* c0.w HeadlightMaskScale     */ 3.50f,   // fx_headlight_mask (s2) projected-mask gain on the body
 /* c1.x HeadlightMaskOnBody    */ 1.50f,   // headlight-mask projection blend (0=off; >1 over-applies)
 /* c1.y MAX_MIP                */ 0.00f,   // set at first body bind from cube GetLevelCount()
 /* c1.z (unused)               */ 0.00f,
 /* c1.w (unused)               */ 0.00f
};

static float g_PS676088[8] = { 1.00f, 15.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f };

static IDirect3DPixelShader9*  sPS_648B38_handle = nullptr;
static IDirect3DPixelShader9*  sPS_676088_handle = nullptr;

// ── Light constants — raw pass-through (no smoothing) ────────────────────────
// Light constants (c0-c2 colors, c12-c14 dirs, c3 ambient) are uploaded raw (see
// Hook_SetVertexShaderConstantF); only the c0-c3 PS-forwarding cache is maintained.
// An earlier EMA smoothing of these was dropped: layered on the engine's churning
// nearest-N light selection, it made specular highlights wander.
// ─────────────────────────────────────────────────────────────────────────────

// Returns false if the bytecode pointer is null or its first DWORD is zero (stub/unset).
static bool HasBytecode(const DWORD* p) { return p && *p; }

// Compute the directional light dirs the PS loop reads (c8..c10). c20-c22 are the
// INVERSE-WORLD normal rows and c12-c14 the OBJECT-space light dirs, so
// normRow · objectLightDir yields a WORLD-space light dir — matching the PS's N
// (also normRow·objectNormal = world). So both N and L live in world space; NdotL is
// consistent.
static void UploadWorldSpaceLights(IDirect3DDevice9* pDevice)
{
    float worldL[12];
    for (int li = 0; li < 3; ++li)
    {
        const float* oL = g_CachedLightDirW + li * 4;   // object-space light dir (c12-c14)
        float x = g_CachedNormRow[0] * oL[0] + g_CachedNormRow[1] * oL[1] + g_CachedNormRow[2]  * oL[2];
        float y = g_CachedNormRow[4] * oL[0] + g_CachedNormRow[5] * oL[1] + g_CachedNormRow[6]  * oL[2];
        float z = g_CachedNormRow[8] * oL[0] + g_CachedNormRow[9] * oL[1] + g_CachedNormRow[10] * oL[2];
        float len = sqrtf(x * x + y * y + z * z);
        float inv = (len > 1e-6f) ? 1.0f / len : 0.0f;
        worldL[li * 4 + 0] = x * inv;
        worldL[li * 4 + 1] = y * inv;
        worldL[li * 4 + 2] = z * inv;
        worldL[li * 4 + 3] = 0.0f;
    }
    pDevice->SetPixelShaderConstantF(8, worldL, 3);   // c8,c9,c10  world-space light dirs
}

static HRESULT WINAPI Hook_CreateVertexShader(IDirect3DDevice9* pDevice, const DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
    const DWORD* finalBytecode = pFunction;
    const char*  label         = nullptr;

    if      (pFunction == reinterpret_cast<const DWORD*>(0x006486F8) && HasBytecode(VS_6486F8))
    { finalBytecode = VS_6486F8; label = "FresnelHeadlights VS (0x6486F8) [vs_3_0]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00675DF0) && HasBytecode(VS_675DF0))
    { finalBytecode = VS_675DF0;     label = "CubeMapFresnel VS (0x675DF0) [vs_2_0]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00675868))
    { label = "UnlitSkinned (0x675868) [vs_1_1]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00675950))
    { label = "LitSkinned (0x675950) [vs_1_1]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00675B30))
    { label = "LitSkinned3 (0x675B30) [vs_1_1]"; }

    HRESULT hr = OrgCreateVertexShader(pDevice, finalBytecode, ppShader);

    if (FAILED(hr) && finalBytecode != pFunction)
    {
        hook_output("[Shader] VS upgrade FAILED (0x%08X): %s — reverting", hr, label ? label : "?");
        hr = OrgCreateVertexShader(pDevice, pFunction, ppShader);
    }

    if (SUCCEEDED(hr) && ppShader && *ppShader)
    {
        char buf[32];
        if (!label) { _snprintf_s(buf, sizeof(buf), "VS@0x%08X", (unsigned)(uintptr_t)pFunction); label = buf; }
        hook_output("[Shader] VS %s: %s", finalBytecode != pFunction ? "Upgraded" : "Created", label);
        // VS_6486F8 handle not needed — no per-bind VS constants (geometry/material only)
    }

    return hr;
}

static HRESULT WINAPI Hook_CreatePixelShader(IDirect3DDevice9* pDevice, const DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
    const DWORD* finalBytecode = pFunction;
    const char*  label         = nullptr;

    if      (pFunction == reinterpret_cast<const DWORD*>(0x00676088) && HasBytecode(PS_676088))
    { finalBytecode = PS_676088; label = "FresnelPS (0x676088) [ps_2_a]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00648B38) && HasBytecode(PS_648B38))
    { finalBytecode = PS_648B38; label = "projectedmaskcar PS (0x648B38) [ps_3_0]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00648A10))
    { label = "projectedmask (0x648A10) [ps_1_1]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00648AA0))
    { label = "projectedmaskwindows (0x648AA0) [ps_1_1]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00648BB8))
    { label = "projectedmaskdetailmap (0x648BB8) [ps_1_1]"; }

    HRESULT hr = OrgCreatePixelShader(pDevice, finalBytecode, ppShader);

    if (FAILED(hr) && finalBytecode != pFunction)
    {
        hook_output("[Shader] PS upgrade FAILED (0x%08X): %s — reverting", hr, label ? label : "?");
        hr = OrgCreatePixelShader(pDevice, pFunction, ppShader);
    }

    if (SUCCEEDED(hr) && ppShader && *ppShader)
    {
        char buf[32];
        if (!label) { _snprintf_s(buf, sizeof(buf), "PS@0x%08X", (unsigned)(uintptr_t)pFunction); label = buf; }
        hook_output("[Shader] PS %s: %s", finalBytecode != pFunction ? "Upgraded" : "Created", label);
        if (finalBytecode == PS_648B38) sPS_648B38_handle = *ppShader;
        // Record the chrome handle ONLY when we actually swapped it (mirrors the body line
        // above; was keyed on pFunction, which recorded the VANILLA chrome PS while stubbed
        // and let Hook_SetPixelShader push g_PS676088 into a shader we no longer own). With
        // the bytecode stubbed, finalBytecode never == PS_676088 → handle stays null → the
        // dormant chrome branch below never fires. Un-stub the array to re-enable the path.
        if (finalBytecode == PS_676088) sPS_676088_handle = *ppShader;
    }

    return hr;
}

static HRESULT WINAPI Hook_SetVertexShader(IDirect3DDevice9* pDevice, IDirect3DVertexShader9* pShader)
{
    return OrgSetVertexShader(pDevice, pShader);
}

// Per-material callback shim — see the route-#2 note above the typedef. Forwards the live submesh
// material's Specular to PS c3, then runs the engine's original per-material callback (c16 + s0).
static void __cdecl Hook_PerMaterialCb(void* material, void* baseTex, void* mask)
{
    if (g_BodyPSActive && material)
    {
        IDirect3DDevice9* dev = DevicePtr.get();
        if (dev)
            dev->SetPixelShaderConstantF(3, (const float*)((const char*)material + 0x20), 1);  // D3DMATERIAL9.Specular → c3
    }
    if (g_OrgPerMaterialCb)
        g_OrgPerMaterialCb(material, baseTex, mask);   // sub_5E9CA0: c16 (cached) + sampler-0 bind
}

// Install the shim into the per-draw callback slot (dword_85B34C). Captures the engine's current
// value as the original, then points the slot at our shim. Idempotent while ours is installed; the
// engine re-points (sub_5E9F80) / clears (sub_5E9AA0) the slot at pass boundaries, so the shim is
// live only across the mode-3 body pass. Called at the body PS bind and re-asserted per draw.
static inline void EnsurePerMaterialHook()
{
    void** slot = (void**)0x85B34C;                    // writable data global (engine mov's into it)
    void*  cur  = *slot;
    if (cur && cur != (void*)&Hook_PerMaterialCb)      // skip if torn-down (0) or already ours
    {
        g_OrgPerMaterialCb = (PerMaterialCb_t)cur;     // = sub_5E9CA0 in mode-3 (only installed while g_BodyPSActive)
        *slot = (void*)&Hook_PerMaterialCb;
    }
}

static HRESULT WINAPI Hook_SetVertexShaderConstantF(IDirect3DDevice9* pDevice, UINT StartReg, const float* pData, DWORD Count)
{
    // No EMA light-smoothing: pass light colors (c0-c2), directions (c12-c14) and
    // ambient (c3) through RAW; just keep the PS-forwarding cache (c0-c3) in sync with
    // what the VS receives. An earlier EMA rewrote those toward a smoothed value, and
    // layered on the engine's churning nearest-N light selection (sub_518100) it made
    // the specular highlights wander/"bounce".
    for (UINT i = 0; i < Count; ++i)
    {
        UINT reg = StartReg + i;
        if (reg < 4)
            memcpy(g_CachedVSConst + reg * 4, pData + i * 4, 16);
        else if (reg >= 12 && reg <= 14)                      // object-space light dirs (for PS world-space loop)
            memcpy(g_CachedLightDirW + (reg - 12) * 4, pData + i * 4, 16);
        else if (reg >= 20 && reg <= 22)                      // inverse-world normal rows (per draw)
            memcpy(g_CachedNormRow + (reg - 20) * 4, pData + i * 4, 16);
    }

    // Forward the per-vehicle light state to the body PS LIVE as the engine writes it.
    // The engine's PS cache suppresses per-vehicle PS re-binds (traffic bodies draw back-to-
    // back with one 648B38 bind), so a PS-bind-only forward strands ALL traffic on a single
    // stale colour/ambient snapshot (observed: all traffic flips colour together, player goes
    // dark). Pushing on the c0-c2 / c3 writes keeps c5-c7 (colours) and c4 (ambient) current —
    // the directions (c8-c10) are already refreshed per draw on the c20-c22 trigger below.
    if (StartReg <= 2 && (StartReg + Count) > 0)
        pDevice->SetPixelShaderConstantF(5, g_CachedVSConst,      3); // c5-c7 = LightColor0-2 (VS c0-c2)
    if (StartReg <= 3 && (StartReg + Count) > 3)
        pDevice->SetPixelShaderConstantF(4, g_CachedVSConst + 12, 1); // c4 = Ambient (VS c3)

    // On the per-draw normal-row upload (c20-c22) during a body draw, recompute the PS
    // world-space light dirs so they're current for this model's submeshes. Also re-assert the
    // per-material shim here: this fires per draw (sub_5E9BA0, before the submesh loop), so the
    // shim survives even if the engine re-points the callback slot per vehicle.
    if (g_BodyPSActive && StartReg <= 22 && (StartReg + Count) > 20)
    {
        UploadWorldSpaceLights(pDevice);
        EnsurePerMaterialHook();
    }

    return OrgSetVertexShaderConstantF(pDevice, StartReg, pData, Count);
}

static HRESULT WINAPI Hook_SetPixelShader(IDirect3DDevice9* pDevice, IDirect3DPixelShader9* pShader)
{
    HRESULT hr = OrgSetPixelShader(pDevice, pShader);
    if (SUCCEEDED(hr) && pShader)
    {
        if (pShader == sPS_648B38_handle)
        {
            g_ActiveFogReg  = 2;     // PS_648B38 reads manual fog from c2
            g_BodyPSActive  = true;  // gate the per-draw world-space light recompute
            EnsurePerMaterialHook(); // shim dword_85B34C → per-submesh material Specular reaches c3

            // IBL — resolve MAX_MIP (c1.y) once from the s1 environment cube.
            // texCUBElod maps roughness→LOD as roughness * MAX_MIP, so the cube
            // needs a full mip chain. Query the bound cube's level count; if it has none,
            // GenerateMipSubLevels (a no-op unless it was created D3DUSAGE_AUTOGENMIPMAP,
            // in which case MAX_MIP stays 0 → always-sharp reflections, roughness inert).
            static bool s_maxMipResolved = false;
            if (!s_maxMipResolved)
            {
                IDirect3DBaseTexture9* tex = nullptr;
                if (SUCCEEDED(pDevice->GetTexture(1, &tex)) && tex)
                {
                    if (tex->GetType() == D3DRTYPE_CUBETEXTURE)
                    {
                        DWORD levels = tex->GetLevelCount();
                        if (levels <= 1) { tex->GenerateMipSubLevels(); levels = tex->GetLevelCount(); }
                        g_PS648B38[5] = (levels > 1) ? float(levels - 1) : 0.0f;  // c1.y = MAX_MIP
                        s_maxMipResolved = true;
                        hook_output("[Shader][C1] env cube levels=%lu -> MAX_MIP=%.1f", levels, g_PS648B38[5]);
                    }
                    tex->Release();
                }
            }

            pDevice->SetPixelShaderConstantF(0, g_PS648B38,          2);  // c0/c1 tunable params (c1.y=MAX_MIP)
            pDevice->SetPixelShaderConstantF(2, g_FogColor,          1);  // c2 = fog color (ps_3_0 manual fog)
            static const float kWhiteSpec[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            pDevice->SetPixelShaderConstantF(3, kWhiteSpec,          1);  // c3 seed = white (full highlight); the shim refines it per submesh
            pDevice->SetPixelShaderConstantF(4, g_CachedVSConst + 12, 1); // c4 = Ambient  (VS c3, cached — scene-lit diffuse)
            pDevice->SetPixelShaderConstantF(5, g_CachedVSConst,      3); // c5-c7 = LightColors (VS c0-c2, cached)

            // Real-loop inputs: integer light count (i0) drives `rep i0`; world-space dirs (c8..).
            const int lightCountI[4] = { g_DirLightCount, 0, 0, 0 };
            pDevice->SetPixelShaderConstantI(0, lightCountI, 1);          // i0 = loop trip count
            UploadWorldSpaceLights(pDevice);                            // c8-c10 (refreshed per draw too)

            // Neutralise the stale material Diffuse (c11). Textured body submeshes skip
            // the engine's c11 upload (sub_5FB480 `!*this` gate) and would inherit a leaked material
            // Diffuse → tinted paint. Force c11 = white at the bind so the body is coloured by its
            // texture. (Diffuse stays global/white by design; route #2 delivers only per-material
            // SPECULAR — via the c3 shim — without disturbing the body's albedo.)
            static const float kWhiteC11[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            pDevice->SetVertexShaderConstantF(11, kWhiteC11, 1);
        }
        else if (pShader == sPS_676088_handle)
        {
            // DORMANT while chrome is stubbed: sPS_676088_handle stays null (handle only
            // recorded on an actual swap), so this never runs. Live only if chrome is re-enabled.
            g_ActiveFogReg = -1;  // chrome is ps_2_a → FFP fog, no manual-fog push
            g_BodyPSActive = false;
            pDevice->SetPixelShaderConstantF(0, g_PS676088, 1);    // c0 (FresnelScale, EnvironmentScale)
        }
        else
        {
            g_ActiveFogReg = -1;  // a shader we don't manage — no manual-fog push
            g_BodyPSActive = false;
        }
    }
    return hr;
}

// Capture D3DRS_FOGCOLOR for ps_3_0 manual fog and push it to the active PS's fog
// register so a mid-pass change reaches the cached PS. D3DRS_FOGCOLOR=34, 0xAARRGGBB.
static HRESULT WINAPI Hook_SetRenderState(IDirect3DDevice9* pDevice, D3DRENDERSTATETYPE State, DWORD Value)
{
    if (State == D3DRS_FOGCOLOR)
    {
        g_FogColor[0] = ((Value >> 16) & 0xFF) / 255.0f;
        g_FogColor[1] = ((Value >>  8) & 0xFF) / 255.0f;
        g_FogColor[2] = ((Value >>  0) & 0xFF) / 255.0f;
        g_FogColor[3] = ((Value >> 24) & 0xFF) / 255.0f;
        if (g_ActiveFogReg >= 0)
            pDevice->SetPixelShaderConstantF(g_ActiveFogReg, g_FogColor, 1);
    }
    return OrgSetRenderState(pDevice, State, Value);
}

static void __cdecl Hook_ShaderInitializationStage()
{
    IDirect3DDevice9* device = DevicePtr.get();
    if (device)
    {
        void** vtable = *(void***)device;

        OrgSetRenderState           = (SetRenderState_t)vtable[57];
        OrgCreateVertexShader       = (CreateVertexShader_t)vtable[91];
        OrgSetVertexShader          = (SetVertexShader_t)vtable[92];
        OrgSetVertexShaderConstantF = (SetVSConstF_t)vtable[94];
        OrgCreatePixelShader        = (CreatePixelShader_t)vtable[106];
        OrgSetPixelShader           = (SetPixelShader_t)vtable[107];

        DWORD old;
        VirtualProtect(&vtable[57],  sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[57]  = &Hook_SetRenderState;          VirtualProtect(&vtable[57],  sizeof(void*), old, &old);
        VirtualProtect(&vtable[91],  sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[91]  = &Hook_CreateVertexShader;       VirtualProtect(&vtable[91],  sizeof(void*), old, &old);
        VirtualProtect(&vtable[92],  sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[92]  = &Hook_SetVertexShader;           VirtualProtect(&vtable[92],  sizeof(void*), old, &old);
        VirtualProtect(&vtable[94],  sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[94]  = &Hook_SetVertexShaderConstantF; VirtualProtect(&vtable[94],  sizeof(void*), old, &old);
        VirtualProtect(&vtable[106], sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[106] = &Hook_CreatePixelShader;         VirtualProtect(&vtable[106], sizeof(void*), old, &old);
        VirtualProtect(&vtable[107], sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[107] = &Hook_SetPixelShader;            VirtualProtect(&vtable[107], sizeof(void*), old, &old);

        hook_output("[Shader] VTable hooks installed (57/91/92/94/106/107)");
    }

    typedef void(__cdecl* InitAllShaders_t)();
    ((InitAllShaders_t)0x5FC170)();
}

void ShaderHandler::Install()
{
    if (!HookConfig::GetBool("Shader", "ShaderEnable", false))   // default OFF — opt-in via mc2hook.ini
    {
        hook_output("[Shader] Disabled in config");
        return;
    }

    // ── Exposed mc2hook.ini [Shader] knobs ────────────────────────────────────
    // EnvmapStrength = s1 env cube ("envmap"); HeadlightMask* = s2 "fx_headlight_mask"
    // (projectedmaskcar). Cook-Torrance direct specular.
    g_PS648B38[0] = HookConfig::GetFloat("Shader", "Reflectance",         0.04f);  // F0 (dielectric clearcoat)
    g_PS648B38[1] = HookConfig::GetFloat("Shader", "EnvmapStrength",      6.00f);  // s1 envmap reflection strength
    g_PS648B38[2] = HookConfig::GetFloat("Shader", "SpecularIntensity",   70.00f);  // direct Cook-Torrance spec scale (× material Specular c3)
    g_PS648B38[3] = HookConfig::GetFloat("Shader", "MaskScale",  3.60f);  // fx_headlight_mask (s2) projected-mask gain
    g_PS648B38[4] = HookConfig::GetFloat("Shader", "HeadlightIntensity", 1.0f);  // headlight-mask projection blend (0=off)

    // ── Baked, NOT exposed in the INI ────────────────────────────────────────
    g_DirLightCount = 3;      // directional lights consumed by the PS loop; max 3; needs the cap bump below

    g_PS676088[0] = g_PS648B38[0];  // chrome inherits Reflectance (dormant)
    g_PS676088[1] = g_PS648B38[1];  // chrome inherits EnvmapStrength (dormant)

    // Light-selection patch — flips a "dead gate" jnz→jz at 0x5181CE (`test ah,41h` @0x5181CB).
    // ⚠ This alone does NOT fix the selection instability — the real bug is sub_518100's
    // threshold/argmax maintenance (seed dword_6C2630/6C2634 to +inf + track the MAX so eviction
    // drops the FARTHEST; the complete fix would be a sub_518100 detour). This patch is baked on
    // but PARTIAL; observed harmless (in-game A/B showed no visible difference — which also
    // confirms the jz polarity isn't inverted). The cap bump below likewise only relocates the
    // churn boundary.
    {
        DWORD old;
        VirtualProtect((void*)0x5181CE, 1, PAGE_EXECUTE_READWRITE, &old); *(BYTE*)0x5181CE = 0x74; VirtualProtect((void*)0x5181CE, 1, old, &old);
        hook_output("[Shader] Gather fix applied (0x5181CE jnz->jz)");
    }

    // Cap bump 2->3 in sub_5198C0 — applied ONLY when the shader actually consumes a 3rd
    // directional light (DirLightCount==3). Stock (in-game directional cap = 2) clears c2/c14,
    // so without the bump the loop's 3rd light is black; tying it to DirLightCount keeps engine
    // selection and the PS loop count consistent.
    if (g_DirLightCount >= 3)
    {
        DWORD old;
        VirtualProtect((void*)0x5198CF, 1, PAGE_EXECUTE_READWRITE, &old); *(BYTE*)0x5198CF = 0x03; VirtualProtect((void*)0x5198CF, 1, old, &old);
        VirtualProtect((void*)0x5198F9, 1, PAGE_EXECUTE_READWRITE, &old); *(BYTE*)0x5198F9 = 0x03; VirtualProtect((void*)0x5198F9, 1, old, &old);
    }

    // Per-material Specular reaches the body PS via the dword_85B34C callback shim (installed at the
    // body PS bind, see EnsurePerMaterialHook) — NOT a global engine patch. The earlier MaterialUngate
    // byte-patch (NOP jnz @0x5FB4D5) was removed: it un-gated sub_5FB480 for EVERY textured submesh in
    // the game (rims/glass/road/props), darkening the whole world. The shim is scoped to mode-3 only.

    InstallCallback("ShaderHandler::EarlyInit", "Intercept early engine shader setup",
        &Hook_ShaderInitializationStage, { cb::call(0x5F15EC) });

    hook_output("[Shader] Installed");
}
