#include "lobbyderived.h"
#include <mc2hook\mc2hook.h>
#include <discord-rpc/discord_rpc.h>

declfield(mcLobbyDerived::Instance)(0x695710);

void mcLobbyDerived::sub_5395E0()
{
	hook::Thunk<0x5395E0>::Call<void>(this);
	Discord_UpdateForState(RPCState::Frontend); // Leaving from Online lobby to Frontend
}
