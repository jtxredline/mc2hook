#include "mcaudiomgr.h"

declfield(mcAudioManager::Instance)(0x6CE2E4);

void mcAudioManager::sub_52B3C0()
{
	hook::Thunk<0x52B3C0>::Call<void>(this); // Call original
}

void mcAudioManager::sub_52B3D0()
{
	hook::Thunk<0x52B3D0>::Call<void>(this); // Call original
}
