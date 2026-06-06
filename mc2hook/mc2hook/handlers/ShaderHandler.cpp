#include "ShaderHandler.h"
#include <d3d9.h>

// Engine's global D3D device pointer at 0x85836C
static hook::Type<IDirect3DDevice9*> DevicePtr(0x85836C);

typedef HRESULT(WINAPI* CreateVertexShader_t)(IDirect3DDevice9*, const DWORD*, IDirect3DVertexShader9**);
typedef HRESULT(WINAPI* CreatePixelShader_t)(IDirect3DDevice9*, const DWORD*, IDirect3DPixelShader9**);
typedef HRESULT(WINAPI* SetVertexShader_t)(IDirect3DDevice9*, IDirect3DVertexShader9*);
typedef HRESULT(WINAPI* SetPixelShader_t)(IDirect3DDevice9*, IDirect3DPixelShader9*);

static CreateVertexShader_t  OrgCreateVertexShader = nullptr;
static CreatePixelShader_t   OrgCreatePixelShader  = nullptr;
static SetVertexShader_t     OrgSetVertexShader    = nullptr;
static SetPixelShader_t      OrgSetPixelShader     = nullptr;

// ── Shader bytecode ──────────────────────────────────────────────────────────
// Creation-time bytecode swaps — zero runtime overhead per frame.
//
// Upgrades:
//   0x675DF0  CubeMapFresnel VS     vs_1_1 → vs_2_0  adds oT2=N_view
//   0x6486F8  FresnelHeadlights VS  vs_1_1 → vs_2_0  Blinn-Phong + rim + env
//   0x648B38  projectedmaskcar PS   ps_1_1 → ps_2_0  Blinn-Phong specular + Fresnel + headlight masks
//
// VS/PS pairs:
//   0x675DF0 (VS_675DF0) ↔ 0x676088 (PS_676088) — CubeMapFresnel chrome
//   0x6486F8 (VS_6486F8) ↔ 0x648B38 (PS_648B38) — headlight receiver (sub_512BB0 mode 3)
// ─────────────────────────────────────────────────────────────────────────────

// ── CubeMapFresnel VS — vs_2_0 + oT2=N_view ──────────────────────────────────
// Constants: c0-c3=lightColors/ambient, c4-c7=MVP, c10=ambientFloor, c11=material,
//            c12-c14=lightDirs, c15=fog, c16=FresnelParams, c17-c19=WorldView, c20-c22=NormalToView
// Outputs: oPos, oFog, oD0(diffuse), oD1.w(Fresnel), oT0(baseUV), oT1(reflVec), oT2(N_view)
static const DWORD VS_675DF0[] = {
    0xFFFE0200, 0x007CFFFE, 0x42415443, 0x0000001C,
    0x000001C3, 0xFFFE0200, 0x0000000A, 0x0000001C,
    0x00008100, 0x000001BC, 0x000000E4, 0x00030002,
    0x000E0001, 0x000000F0, 0x00000000, 0x00000100,
    0x000A0002, 0x002A0001, 0x000000F0, 0x00000000,
    0x0000010F, 0x000F0002, 0x003E0001, 0x000000F0,
    0x00000000, 0x0000011B, 0x00000002, 0x00020003,
    0x00000128, 0x00000000, 0x00000138, 0x000C0002,
    0x00320003, 0x00000144, 0x00000000, 0x00000154,
    0x00040002, 0x00120004, 0x0000015C, 0x00000000,
    0x0000016C, 0x000B0002, 0x002E0001, 0x000000F0,
    0x00000000, 0x00000177, 0x00140002, 0x00520003,
    0x00000184, 0x00000000, 0x00000194, 0x00100002,
    0x00420001, 0x000000F0, 0x00000000, 0x000001A2,
    0x00110002, 0x00460003, 0x000001AC, 0x00000000,
    0x6D415F67, 0x6E656962, 0xABAB0074, 0x00030001,
    0x00040001, 0x00000001, 0x00000000, 0x6D415F67,
    0x6E656962, 0x6F6C4674, 0x6700726F, 0x676F465F,
    0x61726150, 0x6700736D, 0x67694C5F, 0x6F437468,
    0x00726F6C, 0x00030001, 0x00040001, 0x00000003,
    0x00000000, 0x694C5F67, 0x44746867, 0xAB007269,
    0x00030001, 0x00040001, 0x00000003, 0x00000000,
    0x564D5F67, 0xABAB0050, 0x00030003, 0x00040004,
    0x00000001, 0x00000000, 0x614D5F67, 0x69726574,
    0x67006C61, 0x726F4E5F, 0x776F526D, 0xABABAB00,
    0x00030001, 0x00040001, 0x00000003, 0x00000000,
    0x68535F67, 0x72656461, 0x736E6F43, 0x5F670074,
    0x77656956, 0x00776F52, 0x00030001, 0x00040001,
    0x00000003, 0x00000000, 0x325F7376, 0x4D00305F,
    0x6F726369, 0x74666F73, 0x29522820, 0x534C4820,
    0x6853204C, 0x72656164, 0x6D6F4320, 0x656C6970,
    0x30312072, 0xAB00312E, 0x05000051, 0xA00F0008,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0200001F, 0x80000000, 0x900F0000, 0x0200001F,
    0x80000003, 0x900F0003, 0x0200001F, 0x80000005,
    0x900F0007, 0x03000009, 0xC0010000, 0x90E40000,
    0xA0E40004, 0x03000009, 0xC0020000, 0x90E40000,
    0xA0E40005, 0x03000009, 0xC0040000, 0x90E40000,
    0xA0E40006, 0x03000009, 0xC0080000, 0x90E40000,
    0xA0E40007, 0x03000009, 0x80040000, 0x90E40000,
    0xA0E40013, 0x04000004, 0xC00F0001, 0x80AA0000,
    0xA055000F, 0xA000000F, 0x03000008, 0x80010001,
    0x90E40003, 0xA0E4000C, 0x03000008, 0x80020001,
    0x90E40003, 0xA0E4000D, 0x03000008, 0x80040001,
    0x90E40003, 0xA0E4000E, 0x0300000B, 0x80070001,
    0x80E40001, 0xA0E4000A, 0x02000001, 0x80070002,
    0xA0E40000, 0x04000004, 0x80070002, 0x80000001,
    0x80E40002, 0xA0E40003, 0x04000004, 0x800B0001,
    0x80550001, 0xA0A40001, 0x80A40002, 0x04000004,
    0x80070001, 0x80AA0001, 0xA0E40002, 0x80F40001,
    0x03000005, 0xD0070000, 0x80E40001, 0xA0E4000B,
    0x03000009, 0x80010000, 0x90E40000, 0xA0E40011,
    0x03000009, 0x80020000, 0x90E40000, 0xA0E40012,
    0x02000024, 0x80070001, 0x80E40000, 0x03000008,
    0x80010000, 0x90E40003, 0xA0E40014, 0x03000008,
    0x80020000, 0x90E40003, 0xA0E40015, 0x03000008,
    0x80040000, 0x90E40003, 0xA0E40016, 0x03000008,
    0x80080000, 0x80E40001, 0x80E40000, 0x03000002,
    0x80080000, 0x80FF0000, 0x80FF0000, 0x04000004,
    0xE0070001, 0x80E40000, 0x81FF0000, 0x80E40001,
    0x02000001, 0xE0070002, 0x80E40000, 0x03000002,
    0x80010000, 0x81AA0000, 0xA0000010, 0x03000005,
    0x80010000, 0x80000000, 0xA0550010, 0x03000005,
    0x80010000, 0x80000000, 0x80000000, 0x0300000B,
    0x80010000, 0x80000000, 0xA0FF0010, 0x0300000A,
    0xD0080001, 0x80000000, 0xA0AA0010, 0x02000001,
    0xD0080000, 0xA0FF000B, 0x02000001, 0xD0070001,
    0xA0000008, 0x02000001, 0xE0030000, 0x90E40007,
    0x02000001, 0xE0080001, 0xA0000010, 0x0000FFFF
};

// ── FresnelPS — ps_2_0 stub (bytecode pending) ───────────────────────────────
static const DWORD PS_676088[] = { 0 };

// ── FresnelHeadlights VS — vs_2_0 ────────────────────────────────────────────
// Constants: c0-c22 inherited; c23-c30 projectors.
// Outputs: oPos, oFog, oD0(diffuse), oD1.w(Fresnel weight), oT0, oT1(reflVec), oT2(projA), oT3(projB)
static const DWORD VS_6486F8[] = {
    0xFFFE0200, 0x00E8FFFE, 0x42415443, 0x0000001C,
    0x00000373, 0xFFFE0200, 0x00000019, 0x0000001C,
    0x00004108, 0x0000036C, 0x00000210, 0x00030002,
    0x000E0001, 0x0000021C, 0x00000000, 0x0000022C,
    0x000A0002, 0x002A0001, 0x0000021C, 0x00000000,
    0x0000023B, 0x000F0002, 0x003E0001, 0x0000021C,
    0x00000000, 0x00000247, 0x00000002, 0x00020001,
    0x0000021C, 0x00000000, 0x00000255, 0x00010002,
    0x00060001, 0x0000021C, 0x00000000, 0x00000263,
    0x00020002, 0x000A0001, 0x0000021C, 0x00000000,
    0x00000271, 0x000C0002, 0x00320001, 0x0000021C,
    0x00000000, 0x0000027D, 0x000D0002, 0x00360001,
    0x0000021C, 0x00000000, 0x00000289, 0x000E0002,
    0x003A0001, 0x0000021C, 0x00000000, 0x00000295,
    0x00040002, 0x00120004, 0x0000029C, 0x00000000,
    0x000002AC, 0x000B0002, 0x002E0001, 0x0000021C,
    0x00000000, 0x000002B7, 0x00140002, 0x00520001,
    0x0000021C, 0x00000000, 0x000002C2, 0x00150002,
    0x00560001, 0x0000021C, 0x00000000, 0x000002CD,
    0x00160002, 0x005A0001, 0x0000021C, 0x00000000,
    0x000002D8, 0x00170002, 0x005E0001, 0x0000021C,
    0x00000000, 0x000002E7, 0x00180002, 0x00620001,
    0x0000021C, 0x00000000, 0x000002F6, 0x001A0002,
    0x006A0001, 0x0000021C, 0x00000000, 0x00000305,
    0x001B0002, 0x006E0001, 0x0000021C, 0x00000000,
    0x00000314, 0x001C0002, 0x00720001, 0x0000021C,
    0x00000000, 0x00000323, 0x001E0002, 0x007A0001,
    0x0000021C, 0x00000000, 0x00000332, 0x00080002,
    0x00220001, 0x0000021C, 0x00000000, 0x0000033D,
    0x00100002, 0x00420001, 0x0000021C, 0x00000000,
    0x0000034B, 0x00110002, 0x00460001, 0x0000021C,
    0x00000000, 0x00000356, 0x00120002, 0x004A0001,
    0x0000021C, 0x00000000, 0x00000361, 0x00130002,
    0x004E0001, 0x0000021C, 0x00000000, 0x6D415F67,
    0x6E656962, 0xABAB0074, 0x00030001, 0x00040001,
    0x00000001, 0x00000000, 0x6D415F67, 0x6E656962,
    0x6F6C4674, 0x6700726F, 0x676F465F, 0x61726150,
    0x6700736D, 0x67694C5F, 0x6F437468, 0x30726F6C,
    0x4C5F6700, 0x74686769, 0x6F6C6F43, 0x67003172,
    0x67694C5F, 0x6F437468, 0x32726F6C, 0x4C5F6700,
    0x74686769, 0x30726944, 0x4C5F6700, 0x74686769,
    0x31726944, 0x4C5F6700, 0x74686769, 0x32726944,
    0x4D5F6700, 0xAB005056, 0x00030002, 0x00040004,
    0x00000001, 0x00000000, 0x614D5F67, 0x69726574,
    0x67006C61, 0x726F4E5F, 0x776F526D, 0x5F670030,
    0x6D726F4E, 0x31776F52, 0x4E5F6700, 0x526D726F,
    0x0032776F, 0x72505F67, 0x65546A6F, 0x6F523178,
    0x67003077, 0x6F72505F, 0x7865546A, 0x776F5231,
    0x5F670031, 0x6A6F7250, 0x31786554, 0x57776F52,
    0x505F6700, 0x546A6F72, 0x52327865, 0x0030776F,
    0x72505F67, 0x65546A6F, 0x6F523278, 0x67003177,
    0x6F72505F, 0x7865546A, 0x776F5232, 0x5F670057,
    0x61505452, 0x736D6172, 0x535F6700, 0x65646168,
    0x6E6F4372, 0x67007473, 0x6569565F, 0x776F5277,
    0x5F670030, 0x77656956, 0x31776F52, 0x565F6700,
    0x52776569, 0x0032776F, 0x325F7376, 0x4D00305F,
    0x6F726369, 0x74666F73, 0x29522820, 0x534C4820,
    0x6853204C, 0x72656461, 0x6D6F4320, 0x656C6970,
    0x30312072, 0xAB00312E, 0x05000051, 0xA00F0009,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0200001F, 0x80000000, 0x900F0000, 0x0200001F,
    0x80000003, 0x900F0001, 0x0200001F, 0x80000005,
    0x900F0002, 0x03000009, 0xC0010000, 0xA0E40004,
    0x90E40000, 0x03000009, 0xC0020000, 0xA0E40005,
    0x90E40000, 0x03000009, 0xC0040000, 0xA0E40006,
    0x90E40000, 0x03000009, 0xC0080000, 0xA0E40007,
    0x90E40000, 0x03000009, 0x80040000, 0x90E40000,
    0xA0E40013, 0x04000004, 0xC00F0001, 0x80AA0000,
    0xA055000F, 0xA000000F, 0x03000008, 0x80010001,
    0x90E40001, 0xA0E4000C, 0x0300000B, 0x80010001,
    0x80000001, 0xA000000A, 0x02000001, 0x80070002,
    0xA0E40000, 0x04000004, 0x80070001, 0x80000001,
    0x80E40002, 0xA0E40003, 0x03000008, 0x80080001,
    0x90E40001, 0xA0E4000D, 0x0300000B, 0x80080001,
    0x80FF0001, 0xA055000A, 0x04000004, 0x80070001,
    0x80FF0001, 0xA0E40001, 0x80E40001, 0x03000008,
    0x80080001, 0x90E40001, 0xA0E4000E, 0x0300000B,
    0x80080001, 0x80FF0001, 0xA0AA000A, 0x04000004,
    0x80070001, 0x80FF0001, 0xA0E40002, 0x80E40001,
    0x03000005, 0xD0070000, 0x80E40001, 0xA0E4000B,
    0x03000008, 0x80010001, 0x90E40001, 0xA0E40014,
    0x03000008, 0x80020001, 0x90E40001, 0xA0E40015,
    0x03000008, 0x80040001, 0x90E40001, 0xA0E40016,
    0x02000024, 0x80070002, 0x80E40001, 0x03000009,
    0x80010000, 0x90E40000, 0xA0E40011, 0x03000009,
    0x80020000, 0x90E40000, 0xA0E40012, 0x02000024,
    0x80070001, 0x80E40000, 0x03000008, 0x80080001,
    0x80E40001, 0x80E40002, 0x03000002, 0x80080001,
    0x80FF0001, 0x80FF0001, 0x04000004, 0xE0070001,
    0x80E40002, 0x81FF0001, 0x80E40001, 0x02000001,
    0x80080000, 0xA0000010, 0x03000009, 0xE0010002,
    0x80E40000, 0xA0E40017, 0x03000009, 0xE0020002,
    0x80E40000, 0xA0E40018, 0x03000009, 0xE0080002,
    0x80E40000, 0xA0E4001A, 0x03000009, 0xE0010003,
    0x80E40000, 0xA0E4001B, 0x03000009, 0xE0020003,
    0x80E40000, 0xA0E4001C, 0x03000009, 0xE0080003,
    0x80E40000, 0xA0E4001E, 0x03000002, 0x80010000,
    0x81AA0002, 0xA0000010, 0x03000005, 0x80010000,
    0x80000000, 0xA0550010, 0x03000005, 0x80010000,
    0x80000000, 0x80000000, 0x0300000B, 0x80010000,
    0x80000000, 0xA0FF0010, 0x0300000A, 0xD0080001,
    0x80000000, 0xA0AA0010, 0x02000001, 0x80070000,
    0xA0E40014, 0x03000008, 0x80010003, 0x80E40000,
    0xA0E4000D, 0x02000001, 0x80070004, 0xA0E40015,
    0x03000008, 0x80020003, 0x80E40004, 0xA0E4000D,
    0x02000001, 0x80070005, 0xA0E40016, 0x03000008,
    0x80040003, 0x80E40005, 0xA0E4000D, 0x02000024,
    0x80070006, 0x80E40003, 0x03000002, 0x80070003,
    0x81E40001, 0x80E40006, 0x02000024, 0x80070006,
    0x80E40003, 0x03000008, 0x80080000, 0x80E40002,
    0x80E40006, 0x0300000B, 0x80080000, 0x80FF0000,
    0xA0000009, 0x03000005, 0x80080000, 0x80FF0000,
    0x80FF0000, 0x03000005, 0x80080000, 0x80FF0000,
    0x80FF0000, 0x03000005, 0x80080000, 0x80FF0000,
    0x80FF0000, 0x03000005, 0x80080000, 0x80FF0000,
    0x80FF0000, 0x03000005, 0x80080000, 0x80FF0000,
    0x80FF0000, 0x03000005, 0x80070003, 0x80FF0000,
    0xA0E40001, 0x03000008, 0x80010006, 0x80E40000,
    0xA0E4000C, 0x03000008, 0x80020006, 0x80E40004,
    0xA0E4000C, 0x03000008, 0x80040006, 0x80E40005,
    0xA0E4000C, 0x02000024, 0x80070007, 0x80E40006,
    0x03000002, 0x80070006, 0x81E40001, 0x80E40007,
    0x02000024, 0x80070007, 0x80E40006, 0x03000008,
    0x80080000, 0x80E40002, 0x80E40007, 0x0300000B,
    0x80080000, 0x80FF0000, 0xA0000009, 0x03000005,
    0x80080000, 0x80FF0000, 0x80FF0000, 0x03000005,
    0x80080000, 0x80FF0000, 0x80FF0000, 0x03000005,
    0x80080000, 0x80FF0000, 0x80FF0000, 0x03000005,
    0x80080000, 0x80FF0000, 0x80FF0000, 0x03000005,
    0x80080000, 0x80FF0000, 0x80FF0000, 0x04000004,
    0x80070003, 0x80FF0000, 0xA0E40000, 0x80E40003,
    0x03000008, 0x80010000, 0x80E40000, 0xA0E4000E,
    0x03000008, 0x80020000, 0x80E40004, 0xA0E4000E,
    0x03000008, 0x80040000, 0x80E40005, 0xA0E4000E,
    0x02000024, 0x80070004, 0x80E40000, 0x03000002,
    0x80070000, 0x81E40001, 0x80E40004, 0x02000001,
    0xE0070005, 0x80E40001, 0x02000024, 0x80070001,
    0x80E40000, 0x03000008, 0x80010000, 0x80E40002,
    0x80E40001, 0x02000001, 0xE0070004, 0x80E40002,
    0x0300000B, 0x80010000, 0x80000000, 0xA0000009,
    0x03000005, 0x80010000, 0x80000000, 0x80000000,
    0x03000005, 0x80010000, 0x80000000, 0x80000000,
    0x03000005, 0x80010000, 0x80000000, 0x80000000,
    0x03000005, 0x80010000, 0x80000000, 0x80000000,
    0x03000005, 0x80010000, 0x80000000, 0x80000000,
    0x04000004, 0x80070000, 0x80000000, 0xA0E40002,
    0x80E40003, 0x03000005, 0xD0070001, 0x80E40000,
    0xA0000008, 0x02000001, 0xD0080000, 0xA0FF000B,
    0x02000001, 0xE0030000, 0x90E40002, 0x02000001,
    0xE0080001, 0xA0000010, 0x02000001, 0xE0040002,
    0xA0000010, 0x02000001, 0xE0040003, 0xA0000010,
    0x0000FFFF,
};


// ── projectedmaskcar PS — ps_2_0 ─────────────────────────────────────────────
// Fresnel weight from oD1.w (shininess-clamped per submesh by sub_5E9CA0).
static const DWORD PS_648B38[] = {
    0xFFFF0201, 0x0054FFFE, 0x42415443, 0x0000001C,
    0x00000123, 0xFFFF0201, 0x00000006, 0x0000001C,
    0x00004108, 0x0000011C, 0x00000094, 0x00000002,
    0x00020001, 0x000000A0, 0x00000000, 0x000000B0,
    0x00010002, 0x00060001, 0x000000A0, 0x00000000,
    0x000000BA, 0x00000003, 0x00020001, 0x000000C4,
    0x00000000, 0x000000D4, 0x00010003, 0x00060001,
    0x000000DC, 0x00000000, 0x000000EC, 0x00020003,
    0x000A0001, 0x000000F4, 0x00000000, 0x00000104,
    0x00030003, 0x000E0001, 0x0000010C, 0x00000000,
    0x61505F67, 0x736D6172, 0xABAB0030, 0x00030001,
    0x00040001, 0x00000001, 0x00000000, 0x61505F67,
    0x736D6172, 0x5F730031, 0x65736142, 0xABABAB00,
    0x000C0004, 0x00010001, 0x00000001, 0x00000000,
    0x75435F73, 0xAB006562, 0x000E0004, 0x00010001,
    0x00000001, 0x00000000, 0x614D5F73, 0x00416B73,
    0x000C0004, 0x00010001, 0x00000001, 0x00000000,
    0x614D5F73, 0x00426B73, 0x000C0004, 0x00010001,
    0x00000001, 0x00000000, 0x325F7370, 0x4D00615F,
    0x6F726369, 0x74666F73, 0x29522820, 0x534C4820,
    0x6853204C, 0x72656461, 0x6D6F4320, 0x656C6970,
    0x30312072, 0xAB00312E, 0x05000051, 0xA00F0002,
    0x3F800000, 0x00000000, 0x3F428F5C, 0x3ECCCCCD,
    0x0200001F, 0x80000000, 0xB0030000, 0x0200001F,
    0x80000000, 0xB00F0002, 0x0200001F, 0x80000000,
    0xB00F0003, 0x0200001F, 0x80000000, 0x90070000,
    0x0200001F, 0x80000000, 0x900F0001, 0x0200001F,
    0x80000000, 0xB0070004, 0x0200001F, 0x80000000,
    0xB0070005, 0x0200001F, 0x90000000, 0xA00F0800,
    0x0200001F, 0x98000000, 0xA00F0801, 0x0200001F,
    0x90000000, 0xA00F0802, 0x0200001F, 0x90000000,
    0xA00F0803, 0x02000024, 0x80070000, 0xB0E40004,
    0x0200005B, 0x80070001, 0x80E40000, 0x03000008,
    0x80080000, 0x80E40001, 0x80E40001, 0x02000007,
    0x80080000, 0x80FF0000, 0x02000006, 0x80080000,
    0x80FF0000, 0x0200005C, 0x80070001, 0x80E40000,
    0x03000008, 0x80010001, 0x80E40001, 0x80E40001,
    0x02000007, 0x80010001, 0x80000001, 0x02000006,
    0x80010001, 0x80000001, 0x03000002, 0x80080000,
    0x80FF0000, 0x80000001, 0x03000005, 0x80180000,
    0x80FF0000, 0xA0550001, 0x03000002, 0x80080000,
    0x81FF0000, 0xA0000002, 0x02000024, 0x80070001,
    0xB0E40005, 0x03000008, 0x80080001, 0x80E40001,
    0x80E40000, 0x03000002, 0x80080001, 0x80FF0001,
    0x80FF0001, 0x04000004, 0x80070000, 0x80E40000,
    0x81FF0001, 0x80E40001, 0x03000042, 0x800F0001,
    0xB0E40000, 0xA0E40800, 0x03000042, 0x800F0002,
    0x80E40000, 0xA0E40801, 0x02000001, 0x80170000,
    0x80E40002, 0x03000005, 0x80080002, 0x90FF0001,
    0xA0000000, 0x03000005, 0x80080003, 0x80FF0002,
    0x80FF0002, 0x03000005, 0x80070002, 0x80E40002,
    0x80FF0002, 0x03000005, 0x80070002, 0x80E40002,
    0xA0550000, 0x03000005, 0x80070000, 0x80E40000,
    0x80FF0003, 0x04000004, 0x80070002, 0x90E40000,
    0x80E40001, 0x80E40002, 0x04000004, 0x80070002,
    0x90E40001, 0x80E40001, 0x80E40002, 0x04000004,
    0x80070000, 0x80E40000, 0xA0AA0000, 0x80E40002,
    0x03010042, 0x800F0002, 0xB0E40002, 0xA0E40802,
    0x03010042, 0x800F0003, 0xB0E40003, 0xA0E40803,
    0x03000005, 0x80080003, 0x80000003, 0x80000003,
    0x03000005, 0x80080003, 0x80000003, 0x80FF0003,
    0x03000005, 0x80080004, 0x80000002, 0x80000002,
    0x04000004, 0x80080003, 0x80FF0004, 0x80000002,
    0x80FF0003, 0x04000004, 0x80070002, 0x80E40003,
    0x80FF0002, 0x80E40002, 0x02000001, 0x800F0004,
    0xA0E40002, 0x04000004, 0x80070003, 0xA0000001,
    0x80C10004, 0x80D40004, 0x03000005, 0x80070003,
    0x80E40003, 0x80FF0003, 0x04000004, 0x80070004,
    0xA0000001, 0x80D40004, 0x80F90004, 0x03000005,
    0x80070003, 0x80E40003, 0x80E40004, 0x04000004,
    0x80070000, 0x80E40000, 0x80FF0000, 0x80E40003,
    0x03000005, 0x80070000, 0x80E40002, 0x80E40000,
    0x03000005, 0x80170001, 0x80E40000, 0xA0FF0000,
    0x02000001, 0x800F0800, 0x80E40001, 0x0000FFFF,
};

// ── Runtime shader constants — uploaded on each bind, loaded from mc2hook.ini ─
// PS_648B38: c0 = fresnelScale | envScale | rimBoost | maskScale
//            c1 = hotspot      | cavityScale
static float g_PS648B38[8] = {
 /* FresnelScale     */ 0.50f,
 /* EnvironmentScale */ 1.00f,
 /* RimBoost         */ 2.00f,
 /* MaskScale        */ 3.75f,
 /* HeadlightHotspot */ 0.15f,
 /* CavityScale      */ 0.10f,
                        0.00f, 0.00f
};

// VS_6486F8 — SetVertexShaderConstantF(8, ..., 1)
//   c8.x = specIntensity (Blinn-Phong multiplier)
static float g_VS6486F8[4] = {
 /* SpecularIntensity */ 1.00f, 0.00f, 0.00f, 0.00f
};

static IDirect3DPixelShader9*  sPS_648B38_handle = nullptr;
static IDirect3DVertexShader9* sVS_6486F8_handle = nullptr;

// Returns false if the bytecode pointer is null or its first DWORD is zero (stub/unset).
static bool HasBytecode(const DWORD* p) { return p && *p; }

static HRESULT WINAPI Hook_CreateVertexShader(IDirect3DDevice9* pDevice, const DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
    const DWORD* finalBytecode = pFunction;
    const char*  label         = nullptr;

    if      (pFunction == reinterpret_cast<const DWORD*>(0x006486F8) && HasBytecode(VS_6486F8))
    { finalBytecode = VS_6486F8; label = "FresnelHeadlights VS (0x6486F8) [vs_2_0 fix]"; }
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
        if (finalBytecode == VS_6486F8) sVS_6486F8_handle = *ppShader;
    }

    return hr;
}

static HRESULT WINAPI Hook_CreatePixelShader(IDirect3DDevice9* pDevice, const DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
    const DWORD* finalBytecode = pFunction;
    const char*  label         = nullptr;

    if      (pFunction == reinterpret_cast<const DWORD*>(0x00676088) && HasBytecode(PS_676088))
    { finalBytecode = PS_676088; label = "FresnelPS (0x676088) [ps_2_0]"; }
    else if (pFunction == reinterpret_cast<const DWORD*>(0x00648B38) && HasBytecode(PS_648B38))
    { finalBytecode = PS_648B38; label = "projectedmaskcar PS (0x648B38) [ps_2_0 fix]"; }
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
    }

    return hr;
}

static HRESULT WINAPI Hook_SetVertexShader(IDirect3DDevice9* pDevice, IDirect3DVertexShader9* pShader)
{
    HRESULT hr = OrgSetVertexShader(pDevice, pShader);
    if (SUCCEEDED(hr) && pShader && pShader == sVS_6486F8_handle)
        pDevice->SetVertexShaderConstantF(8, g_VS6486F8, 1);
    return hr;
}

static HRESULT WINAPI Hook_SetPixelShader(IDirect3DDevice9* pDevice, IDirect3DPixelShader9* pShader)
{
    HRESULT hr = OrgSetPixelShader(pDevice, pShader);
    if (SUCCEEDED(hr) && pShader && pShader == sPS_648B38_handle)
        pDevice->SetPixelShaderConstantF(0, g_PS648B38, 2);
    return hr;
}

static void __cdecl Hook_ShaderInitializationStage()
{
    IDirect3DDevice9* device = DevicePtr.get();
    if (device)
    {
        void** vtable = *(void***)device;

        OrgCreateVertexShader = (CreateVertexShader_t)vtable[91];
        OrgSetVertexShader    = (SetVertexShader_t)vtable[92];
        OrgCreatePixelShader  = (CreatePixelShader_t)vtable[106];
        OrgSetPixelShader     = (SetPixelShader_t)vtable[107];

        DWORD old;
        VirtualProtect(&vtable[91],  sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[91]  = &Hook_CreateVertexShader; VirtualProtect(&vtable[91],  sizeof(void*), old, &old);
        VirtualProtect(&vtable[92],  sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[92]  = &Hook_SetVertexShader;    VirtualProtect(&vtable[92],  sizeof(void*), old, &old);
        VirtualProtect(&vtable[106], sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[106] = &Hook_CreatePixelShader;  VirtualProtect(&vtable[106], sizeof(void*), old, &old);
        VirtualProtect(&vtable[107], sizeof(void*), PAGE_EXECUTE_READWRITE, &old); vtable[107] = &Hook_SetPixelShader;     VirtualProtect(&vtable[107], sizeof(void*), old, &old);

        hook_output("[Shader] VTable hooks installed (Create/Set VS 91/92, Create/Set PS 106/107)");
    }

    typedef void(__cdecl* InitAllShaders_t)();
    ((InitAllShaders_t)0x5FC170)();
}

void ShaderHandler::Install()
{
    if (!HookConfig::GetBool("Shader", "ShaderEnable", false))
    {
        hook_output("[Shader] Disabled in config");
        return;
    }

    g_PS648B38[0] = HookConfig::GetFloat("Shader", "FresnelScale",     0.50f);
    g_PS648B38[1] = HookConfig::GetFloat("Shader", "EnvironmentScale", 1.00f);
    g_PS648B38[2] = HookConfig::GetFloat("Shader", "RimBoost",         2.00f);
    g_PS648B38[3] = HookConfig::GetFloat("Shader", "MaskScale",        3.75f);
    g_PS648B38[4] = HookConfig::GetFloat("Shader", "HeadlightHotspot", 0.15f);
    g_PS648B38[5] = HookConfig::GetFloat("Shader", "CavityScale",      0.10f);

    g_VS6486F8[0] = HookConfig::GetFloat("Shader", "SpecularIntensity", 1.00f);

    hook_output("[Shader] fresnel=%.2f env=%.2f rim=%.2f mask=%.2f hotspot=%.2f cavity=%.2f spec=%.2f",
        g_PS648B38[0], g_PS648B38[1], g_PS648B38[2], g_PS648B38[3], g_PS648B38[4], g_PS648B38[5], g_VS6486F8[0]);

    InstallCallback("ShaderHandler::EarlyInit", "Intercept early engine shader setup",
        &Hook_ShaderInitializationStage, { cb::call(0x5F15EC) });

    hook_output("[Shader] Installed");
}
