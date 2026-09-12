#include "viewport.h"

void gfxViewport::Perspective(float const& fov, float const& aspect, float const& nearClip, float const& farClip)
{
    hook::StaticThunk<0x5FA880>::Call<void>(&fov, &aspect, &nearClip, &farClip);
}
