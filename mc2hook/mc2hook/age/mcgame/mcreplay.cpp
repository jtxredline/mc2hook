#include "mcreplay.h"
#include <mc2hook\mc2hook.h>
#include <age/core/output.h> //
#include <age/mcgame/raceconfig.h>
#include <age/data/replay.h>
#include <age/mcgame/playermgr.h>
#include <age/mcgame/player.h>
#include <age/vehicle/entity.h>
#include <age/vehicle/carsim.h>
#include <age/physics/phcollider.h>
#include <age/physics/phinertia.h>
#include <age/mcnetwork/netmanager.h>

// TODO: Put all mc classes into a mcgame folder

// Loading .rpy
#include <fstream>
#include <vector>
#include <cstdint>

void mcReplay::StartPlayback()
{
	// Save .rpy
	//int size = this->m_Size;
	//const void* adr = this->m_Stream; //this->dword_14; // MIGHT NOT BE datMemStream!!!
	//std::ofstream file("replay_dump.rpy", std::ios::binary);
	//file.write((char*)adr, size);
	//Printf("SAVED REPLAY\n");

	// Load .rpy
	//std::ifstream file("replay_dump.rpy", std::ios::binary);
	//file.seekg(0, std::ios::end);
	//size_t size = file.tellg();
	//file.seekg(0);
	//file.read((char*)this->m_Stream, size);
	//file.read((char*)this->dword_18, size); //
	//this->m_Size = size;

	hook::Thunk<0x404C80>::Call<void>(this); // Call original
}

bool mcReplay::Update()
{
	//return hook::Thunk<0x404D50>::Call<bool>(this); // Call original

    if (mcRaceConfig::g_NumPlayers == 0)
        return true;

    datReplay::datReplay_614770();

    // Frame counter (cycles every 0x1E = 30 ticks)
    this->dword_10++;
    if (this->dword_10 < 0x1E)
        return true;
    this->dword_10 = 0;

    mcPlayer* player = mcPlayerManager::GetPlayer(0); //(mcNetManager::IsNetworkMode ? mcNetManager::LocalPlayerID : 0);
    phInertialCS* ics = player->m_Entity->m_Car.m_CarSim->m_Collider->m_ICS;

    float x = ics->m_WorldTransform.m30;
    float y = ics->m_WorldTransform.m31;
    float z = ics->m_WorldTransform.m32;

    if (!datReplay::sm_ReplayFile)
        return true;

    if (datReplay::sm_Playback)
    {
        // Retreive recorded xyz
        
        float replayX = *(float*)(datReplay::GetInt());
        float replayY = *(float*)(datReplay::GetInt());
        float replayZ = *(float*)(datReplay::GetInt());

        // Divergence check: compare recorded pos vs actual pos
        if (replayX != x || replayY != y || replayZ != z)
        {
            Warningf("Replay diverged! Was (%f, %f, %f), but is instead (%f, %f, %f)!",
                     replayX, replayY, replayZ,
                     x, y, z);

            return false;
        }
    }
    else
    {
        // Record pos
        datReplay::RecordInt(*(int*)&x);
        datReplay::RecordInt(*(int*)&y);
        datReplay::RecordInt(*(int*)&z);
    }

    return true;
}
