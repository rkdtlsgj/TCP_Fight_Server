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

	//�����߰�
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


//ĳ������ ���� ��ǥ�� ������ġ�� ����Ͽ� �ش���ġ ���Ϳ� ����
void Sector_AddCharacter(stClient* pClient);

//ĳ������ ���� ��ǥ�� ���͸� ����ؼ� �ش� ���Ϳ��� ����
void Sector_RemoveCharacter(stClient* pClient);

//���� ��ġ�� ���Ϳ��� ���� �� ������ ��ǥ�� ���͸� ���Ӱ� ����Ͽ� ���Ϳ� ����
bool Sector_UpdateCharacter(stClient* pClient);

//Ư�� ������ǥ���� �ֹ������ ���� ��������
void GetSectorAround(int iSectorX, int iSectorY, st_SECOTR_AROUND* pSectorAround);

//���Ϳ��� ���͸� �̵��Ͽ��� �� ���� ����ǿ��� ���� ����, �����߰��� ���� ������ ���ϴ� �Լ�
void GetUpdateSectorAround(stClient* pClient, st_SECOTR_AROUND* pRemoveSector, st_SECOTR_AROUND* pAddSector);

//ĳ������ �����̵��� ������Ʈ��Ŷ
void CharacterSectorUpdatePacket(stClient* pClient);



void DebugSector();