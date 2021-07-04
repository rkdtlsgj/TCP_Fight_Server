#include "Client.h"
#include "Define.h"

std::unordered_map<DWORD, stClient*> g_ClientInfo;

stClient* CreateClient(DWORD dwSessionID)
{
	stClient* pClient = new stClient;

	pClient->dwSeesionID = dwSessionID;
	pClient->dwAction = dfACTION_STAND;

	pClient->byDir = dfACTION_MOVE_LL;
	pClient->byMoveDir = dfACTION_STAND;

	pClient->shX = rand() % dfMAP_WIDTH;
	pClient->shY = rand() % dfMAP_HEIGHT;


	pClient->cHP = 100;

	return pClient;
}

stClient* FindClient(DWORD dwSessionID)
{
	std::unordered_map < DWORD, stClient*>::iterator clientIter = g_ClientInfo.find(dwSessionID);

	if (clientIter == g_ClientInfo.end())
		return NULL;

	return clientIter->second;
}