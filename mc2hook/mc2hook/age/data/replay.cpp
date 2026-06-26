#include "replay.h"
#include <age/core/output.h> //

declfield(datReplay::sm_FrameSize)(0x860670);
declfield(datReplay::sm_ReplayFile)(0x86066C);
declfield(datReplay::sm_Playback)(0x860620);

uint8_t datReplay::GetByte()
{
	return hook::StaticThunk<0x614590>::Call<uint8_t>(); // Call original
}

int datReplay::GetInt()
{
	return hook::StaticThunk<0x6145E0>::Call<int>(); // Call original
}

void datReplay::RecordInt(int a1)
{
	hook::StaticThunk<0x614700>::Call<void>(a1); // Call original
}

uint16_t datReplay::ReadFrameUInt16()
{
	return hook::StaticThunk<0x6145C0>::Call<uint16_t>(); // Call original
}

bool datReplay::BeginRecording(datMemStream* stream)
{
	return hook::StaticThunk<0x6148C0>::Call<bool>(stream); // Call original
}

void datReplay::Reset()
{
	hook::StaticThunk<0x614740>::Call<void>(); // Call original
}

void datReplay::datReplay_614770()
{
	hook::StaticThunk<0x614770>::Call<void>(); // Call original
}