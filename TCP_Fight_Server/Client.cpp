#include "Client.h"

std::unordered_map<DWORD, stClient*> g_ClientInfo;
std::list<stClient*> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];


stClient* CreateClient(DWORD dwSessionID)
{
	stClient* pClient = new stClient;

	pClient->dwSeesionID = dwSessionID;
	pClient->dwAction = dfACTION_STAND;

	pClient->byDir = dfACTION_MOVE_RR;
	pClient->byMoveDir = dfACTION_STAND;

	pClient->shX = rand() % dfMAP_WIDTH;
	pClient->shY = rand() % dfMAP_HEIGHT;

	pClient->stCurSector.iX = pClient->shX / dfSECTOR_SIZE_X;
	pClient->stCurSector.iY = pClient->shY / dfSECTOR_SIZE_Y;

	pClient->stOldSector.iX = pClient->stCurSector.iX;
	pClient->stOldSector.iY = pClient->stCurSector.iY;


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


void Sector_AddCharacter(stClient* pClient)
{
	int iSectorX = pClient->shX / dfSECTOR_SIZE_X;
	int iSectorY = pClient->shY / dfSECTOR_SIZE_Y;

	pClient->stOldSector.iX  = pClient->stCurSector.iX;
	pClient->stOldSector.iY = pClient->stCurSector.iY;

	pClient->stOldSector.iX = iSectorX;
	pClient->stOldSector.iY = iSectorY;

}

void Sector_RemoveCharacter(stClient* pClient)
{

}

bool Sector_UpdateCharacter(stClient* pClient)
{
	return true;
}