#include "opponent.h"
//#include <core/output.h>

bool aiOpponent::Init(int index, const char* carName, const aiOpponentDesc* desc, int a5, bool a6)
{	
	return hook::Thunk<0x4B0110>::Call<bool>(this, index, carName, desc, a5, a6); // Call original

	//Displayf("aiOpponent::Init, %d %s", index, carName);

	//// Load AI-specific resources
	//char boundName[128];
	//snprintf(boundName, sizeof(boundName), "%s_bound", carName);
}
