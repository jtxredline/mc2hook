#pragma once
#include <mc2hook\mc2hook.h>

class gfxViewport
{
public:
    static void Perspective(float const& fov, float const& aspect, float const& nearClip, float const& farClip);
};
