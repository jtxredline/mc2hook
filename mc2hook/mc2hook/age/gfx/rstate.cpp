#include "rstate.h"

declfield(gfxState::sm_Camera) = 0x858548;

void gfxState::SetCamera(Matrix44 const& mtx) {
    hook::StaticThunk<0x5EE8D0>::Call<void>(&mtx); 
}

void gfxState::SetCamera(Matrix34 const& mtx) {
    hook::StaticThunk<0x5EE900>::Call<void>(&mtx); 
}

Matrix44 gfxState::GetCameraMatrix() {
    return gfxState::sm_Camera;
}

void gfxState::SetWorld(Matrix44 const& mtx) {
    hook::StaticThunk<0x5EDB00>::Call<void>(&mtx);
}

void gfxState::SetWorld(Matrix34 const& mtx) {
    hook::StaticThunk<0x5EDB20>::Call<void>(&mtx);
}

/*void gfxState::SetColor(Vector4 const& color) {
    hook::StaticThunk<0x5EE630>::Call<void>(&color);
}*/

//void gfxRenderState::Convert(Matrix44 const& dest, Matrix34 const& src) {
//    hook::StaticThunk<0x5De230>::Call<void>(&dest, &src);
//}
