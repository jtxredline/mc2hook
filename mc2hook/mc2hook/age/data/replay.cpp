#include "replay.h"

declfield(datReplay::sm_FrameSize)(0x860670);
declfield(datReplay::sm_ReplayFile)(0x86066C);

uint8_t datReplay::GetByte()
{
	return hook::StaticThunk<0x614590>::Call<uint8_t>();
}

int datReplay::GetInt()
{
	return hook::StaticThunk<0x6145E0>::Call<int>();
}

uint16_t datReplay::ReadFrameUInt16()
{
	return hook::StaticThunk<0x6145C0>::Call<uint16_t>();
}
