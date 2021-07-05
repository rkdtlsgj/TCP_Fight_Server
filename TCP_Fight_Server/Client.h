#pragma once

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <unordered_map>
#include "Define.h"

struct st_SECTOR_POS
{
	int iX;
	int iY;
};

struct st_SECOTR_AROUND
{
	int iCount;
	st_SECTOR_POS stAround[9];
};

struct  stClient
{
	DWORD dwSeesionID;
	DWORD dwAction;

	BYTE byDir;
	BYTE byMoveDir;

	short shX;
	short shY;

	//세션추가
	st_SECTOR_POS stOldSector;
	st_SECTOR_POS stCurSector;

	char cHP;
};

extern std::unordered_map<DWORD, stClient*> g_ClientInfo;
extern std::list<stClient*> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

stClient* FindClient(DWORD dwSessionID);
stClient* CreateClient(DWORD dwSessionID);





//캐릭터의 현재 좌표로 섹터위치를 계산하여 해당위치 섹터에 넣음
void Sector_AddCharacter(stClient* pClient);

//캐릭터의 현재 좌표로 섹터를 계산해서 해당 섹터에서 삭제
void Sector_RemoveCharacter(stClient* pClient);

//현재 위치한 섹터에서 삭제 후 현재의 좌표로 섹터를 새롭게 계산하여 섹터에 넣음
bool Sector_UpdateCharacter(stClient* pClient);
