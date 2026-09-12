#include "racebase.h"
#include <age/mcgame/ghostmgr.h>

void mcRaceBase::Destructor()
{
	// DestroyGhost here
	mcGhostManager::Instance.DestroyGhost();

	hook::Thunk<0x489540>::Call<void>(this); // Call original
}
