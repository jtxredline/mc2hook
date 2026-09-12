#include "manager.h"
#include <core/output.h>
#include <mcrace/racemgr.h>
#include <mcdata/raceconfig.h>
#include <mcrace/racebase.h>
#include <mcai/opponent.h>
#include <age/globals.h>
#include <memory/memory.h>

#include <veh_base/entity.h> //
#include <mcplayer/factory.h> //

#include <handlers/CustomVehicleHandler.h>

declfield(aiOpponentManager::Instance)(0x698624);
declfield(aiOpponentManager::byte_6C342B)(0x6C342B);

bool aiOpponentManager::Init(bool loadResources)
{
    Displayf("aiOpponentManager::Init");

    if (!byte_6C342B)
        return false;

    mcRaceBase* raceBase = mcRaceManager::Instance->m_CurrentRace;

    m_NumOpponents = raceBase->m_NumOpponents;
    m_NumCompetitors = 0;

    if (m_NumOpponents > 0)
    {
        int numPlayers = mcRaceConfig::g_NumPlayers;

        int* allocation = (int*)age_allocate(sizeof(aiOpponent) * m_NumOpponents + sizeof(int));

        if (allocation)
        {
            *allocation = m_NumOpponents;

            m_Opponents = (aiOpponent*)(allocation + 1);

            for (int i = 0; i < m_NumOpponents; i++) // Original calls ConstructArray_401750
            {
                new (&m_Opponents[i]) aiOpponent();
            }
        }
        else
        {
            m_Opponents = nullptr;
        }

        static bool addonVehicles = HookConfig::GetBool("Experimental", "AddonVehicles", false);

        for (int i = 0; i < m_NumOpponents; ++i)
        {
            aiOpponent* opponent = &m_Opponents[i];

            // One entry per opponent in the race definition.
            aiOpponentDesc* raceOpponentDesc = &raceBase->m_OpponentDescs[i];

            if (!addonVehicles)
                opponent->Init(i, g_VehicleNamesDev[raceOpponentDesc->m_CarIndex], raceOpponentDesc, i, loadResources);
            else
                opponent->Init(i, VEHICLE_BASENAMES_DYN[raceOpponentDesc->m_CarIndex], raceOpponentDesc, i, loadResources);

            opponent->dword_60 = 0;

            // Multiplayer team assignment
            if (mcRaceConfig::g_MultiplayerTeamMode)
            {
                if (mcRaceConfig::g_MultiplayerTeamMode == 1)
                {
                    opponent->m_Team = ((i + numPlayers) < ((numPlayers + m_NumOpponents) / 2)) ? 2 : 1;
                }
                else
                {
                    opponent->m_Team = ((i + numPlayers) & 1) ? 1 : 2;
                }
            }
            else
            {
                opponent->m_Team = 1;
            }

            if (raceOpponentDesc->m_IsCompetitor) ++m_NumCompetitors;
        }
    }

    m_CurrentOpponentIdx = 0;

    //// Test car spawn
    //vehFactory factory;
    //factory.Build("vp_saleen", 0, nullptr); // Setting idx to higher than 0 enables collisions again
    //vehEntity* entity = factory.Create();
    //mcCar* car = &entity->m_Car;
    //Printf("GHOST mcCar: %p\n\n\n\n\n\n", car);

    return true;
}
