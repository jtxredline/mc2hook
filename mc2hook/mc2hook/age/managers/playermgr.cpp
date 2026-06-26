#include "playermgr.h"

declfield(mcPlayerManager::Instance)(0x69585C);

mcPlayer* mcPlayerManager::GetPlayer(int idx)
{
	return Instance->m_Players[idx];
}
