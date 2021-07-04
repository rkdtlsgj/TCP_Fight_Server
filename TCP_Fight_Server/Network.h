#pragma once
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <unordered_map>
#include "CPacket.h"
#include "CRingBuffer.h"


struct  stSession
{
	SOCKET socket;
	DWORD dwSeesionID;
	DWORD dwLastRecvTime;

	CRingBuffer cSendQ;
	CRingBuffer cRecvQ;

	SOCKADDR_IN sockAddr;
};


struct st_PACKET_HEADER
{
	BYTE	byCode;			// 패킷코드 0x89 고정.
	BYTE	bySize;			// 패킷 사이즈.
	BYTE	byType;			// 패킷타입.
};


stSession* FindSession(DWORD dwSessionID);
stSession* CreateSession(DWORD dwSessionID,SOCKET socket, SOCKADDR_IN sockaddr);

void DisConnect(DWORD dwSessionID);

void NetWorkProcess();
void NetAccept_Proc();
void NetRecv_Proc(DWORD dwSessionID);
void NetSend_Proc(DWORD dwSessionID);

void SelectSocket(DWORD* dwSessionID, SOCKET* pUserSocket, FD_SET* pReadSet, FD_SET* pWriteSet);

bool PacketProc(stSession* pSession, BYTE byType, CPacket* pPacket);
int CompleteRecvPacket(stSession* pSession);

bool InitServer();
bool OpenServer();



void Send_ResCharactor(stSession* pSession, DWORD dwSessionID,BYTE byDir, short shX, short shY,char cHP);
void MakePacket_Charctor(st_PACKET_HEADER* stHeader, CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP);


void SendUnicast(stSession* pSession, st_PACKET_HEADER* pHeader, CPacket* pPakcet);