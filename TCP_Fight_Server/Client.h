#pragma once

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <unordered_map>


struct  stClient
{
	DWORD dwSeesionID;
	DWORD dwAction;

	BYTE byDir;
	BYTE byMoveDir;

	short shX;
	short shY;

	//�����߰�

	char cHP;
};



stClient* FindClient(DWORD dwSessionID);
stClient* CreateClient(DWORD dwSessionID);