#include "GravityHandler.h"

static float gravity;

void GravityHandler::Install()
{
    gravity = HookConfig::GetFloat("Physics", "Gravity", -9.8f);

    mem::write(0x6449BC, gravity);
}
