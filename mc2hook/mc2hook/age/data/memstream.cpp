#include "memstream.h"
#include <age/data/replay.h> //
#include <age/core/output.h> //

uint32_t datMemStream::GetCh(int stream)
{
	return hook::StaticThunk<0x617A20>::Call<uint32_t>(stream);
}

int datMemStream::Write(const void* data, unsigned int size)
{
	//Printf(
	//	"write=%u used=%u cursor=%p\n",
	//	size,
	//	*(this->dword_1c + 8),
	//	this->dword_20
	//);

	return hook::Thunk<0x6179D0>::Call<int>(this, data, size); // Call original
}

// TODO:
// Try to print, then write to file in real time
// if that works somewhat, try to write it all at once in the end