#include <mc2hook/mc2hook.h>
#include "simulator.h"

phSimulator* g_phSimulator = (phSimulator*)0x006C8EEC;

bool phSimulator::ApplyForce(int bodyId, Vector3* force, Vector3* position, float mass)
{
    return hook::Thunk<0x585E80>::Call<bool>(this, bodyId, force, position, mass);
}
