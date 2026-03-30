#include "token.h"
#include <mc2hook\mc2hook.h>

float datAsciiTokenizer::GetFloat()
{
	return hook::Thunk<0x6157F0>::Call<float>(this);
}
