#include <mc2hook/mc2hook.h>
#include "simulator.h"

declfield(phSimulator::Instance)(0x6C8EEC);

bool phSimulator::ApplyForce(int bodyId, Vector3* force, Vector3* position, float mass)
{
    return hook::Thunk<0x585E80>::Call<bool>(this, bodyId, force, position, mass);
}
