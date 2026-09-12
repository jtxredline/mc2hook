#include "ghostmgr.h"
#include <core/output.h>

mcGhostManager mcGhostManager::Instance;

void mcGhostManager::SpawnGhost(const char* carName)
{
    // Already have a ghost?
    if (m_Ghost.m_Entity)
        return;

    if (!m_Ghost.Spawn(carName))
    {
        Printf("Failed to spawn ghost car '%s'\n", carName); 
        return;
    }

    Printf("Spawned ghost car: %s\n", carName);
}

void mcGhostManager::DestroyGhost()
{
    m_Ghost.Destroy();// (true);
}

void mcGhostManager::Update()
{
    if (m_Ghost.m_Entity)
        m_Ghost.Update();
}
