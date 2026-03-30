#include "RenderDistanceHandler.h"
#include <age/core/output.h>

static float ccMultiplier = 1.0f;

float RenderDistanceHandler::GetFloatHook()
{
	float val = hook::Thunk<0x6157F0>::Call<float>(this); // datAsciiTokenizer::GetFloat();
	val *= ccMultiplier;
	return val;
}

void RenderDistanceHandler::Install()
{
    ccMultiplier = HookConfig::GetFloat("Graphics", "CCRadiusMultiplier", 1.0f);

    InstallCallback("CC Multiplier Handler", "GetFloat Hook",
        &GetFloatHook, {
            cb::call(0x5274E4),
            cb::call(0x526337),
        });
}
