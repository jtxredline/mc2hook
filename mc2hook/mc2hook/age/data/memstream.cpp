#include "memstream.h"

uint32_t datMemStream::GetCh(int stream)
{
	return hook::StaticThunk<0x617A20>::Call<uint32_t>(stream);
}
