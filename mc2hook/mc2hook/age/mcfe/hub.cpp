#include "hub.h"
#include <mc2hook\mc2hook.h>
#include <discord-rpc/discord_rpc.h>

void mcFeHub::sub_419E60()
{
	hook::Thunk<0x419E60>::Call<void>(this);
	Discord_UpdateForState(RPCState::Frontend); // Leaving from LAN lobby to Frontend
}
