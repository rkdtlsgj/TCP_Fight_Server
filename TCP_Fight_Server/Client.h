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

	//Test
	//stSession* pSession;
};

extern std::unordered_map<DWORD, stClient*> g_ClientInfo;
extern std::list<stClient*> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

stClient* FindClient(DWORD dwSessionID);
stClient* CreateClient(DWORD dwSessionID);
bool DeleteClient(DWORD dwSessionID);

void Update();


//캐릭터의 현재 좌표로 섹터위치를 계산하여 해당위치 섹터에 넣음
void Sector_AddCharacter(stClient* pClient);

//캐릭터의 현재 좌표로 섹터를 계산해서 해당 섹터에서 삭제
void Sector_RemoveCharacter(stClient* pClient);

//현재 위치한 섹터에서 삭제 후 현재의 좌표로 섹터를 새롭게 계산하여 섹터에 넣음
bool Sector_UpdateCharacter(stClient* pClient);

//특정 섹터좌표기준 주번영향권 섹터 가져오기
void GetSectorAround(int iSectorX, int iSectorY, st_SECOTR_AROUND* pSectorAround);

//섹터에서 섹터를 이동하였을 때 섹터 영향권에서 빠진 섹터, 새로추가된 섹터 정보를 구하는 함수
void GetUpdateSectorAround(stClient* pClient, st_SECOTR_AROUND* pRemoveSector, st_SECOTR_AROUND* pAddSector);

//캐릭터의 섹터이동시 업데이트패킷
void CharacterSectorUpdatePacket(stClient* pClient);



void DebugSector();