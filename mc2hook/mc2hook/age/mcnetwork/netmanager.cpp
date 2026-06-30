#include "netmanager.h"
#include "age/state/gamestate.h"
#include "age/types.h"
#include "discord-rpc/discord_rpc.h"
#include <age/core/output.h>
#include <age/string/string.h>

declfield(mcNetManager::Instance)(0x6C38B4);
declfield(mcNetManager::IsNetworkMode)(0x6C3421);
declfield(mcNetManager::LocalPlayerID)(0x6C3424);

int mcNetManager::GetPlayerCount()
{
    unsigned int mask = m_ActivePlayersBitfield;
    int count = 0;

    while (mask)
    {
        mask &= (mask - 1);
        count++;
    }
    return count;
}

void mcNetManager::NotifyJoin(int peerID)
{
    if (m_ConnectedState == peerID)
    {
        Displayf("I joined successfully as peer %d", peerID);
        Discord_UpdateForState(RPCState::Loading); // RPC refresh
        Discord_UpdateForState(RPCState::Host); // Update RPC player count
    }    
    else
    {
        Displayf("Peer %d joined successfully", peerID);
        Discord_UpdateForState(RPCState::Loading); // RPC refresh
        Discord_UpdateForState(RPCState::Host); // Update RPC player count
    }
}

void mcNetManager::NotifyQuit(int peerID, void* a2, wchar_t* reason, char wasHost)
{
    char* nickname = nullptr;
    char buffer[256]; // safe temp buffer

    if (reason)
    {
        int len = (int)wcslen(reason) + 1;
        if (len > sizeof(buffer))
            len = sizeof(buffer);

        NarrowCopy(buffer, reason, len);
        nickname = buffer;
    }

    if (wasHost)
    {
        Displayf("I (%s) was host and I left successfully - Reason: %s", nickname, nullptr);
    }
    else if (peerID)
    {
        Displayf("Peer %d (%s) quit successfully - Reason: %s", peerID, nickname, nullptr);
        Discord_UpdateForState(RPCState::Loading); // RPC refresh
        if (mcGameState::Instance->m_CurrentState == GameState::Game) Discord_UpdateForState(RPCState::Race); // Update RPC when a peer leaves mid-game
        if (mcGameState::Instance->m_CurrentState == GameState::Frontend) Discord_UpdateForState(RPCState::Host); // Update RPC when a peer leaves a host
    }
    else
    {
        Displayf("Host (%s) quit, I got ejected, or I quit successfully - Reason: %s", nickname, nullptr);
        Discord_UpdateForState(RPCState::Loading); // RPC refresh
        if (mcGameState::Instance->m_CurrentState == GameState::Game) Discord_UpdateForState(RPCState::Race); // Update RPC when a host leaves mid-game
        if (mcGameState::Instance->m_CurrentState == GameState::Frontend) Discord_UpdateForState(RPCState::Frontend); // Update RPC when a host leaves the lobby
    }
}

void mcNetManager::DisplayConnectedState()
{
    Displayf("Connected State: %d", mcNetManager::Instance->m_ConnectedState);

    if (mcNetManager::Instance->m_ConnectedState == -1) Discord_UpdateForState(RPCState::LobbyLAN);
    if (mcNetManager::Instance->m_ConnectedState >= 0)  Discord_UpdateForState(RPCState::Host);
}
