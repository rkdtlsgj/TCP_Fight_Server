#pragma once
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <unordered_map>
#include "CPacket.h"
#include "CRingBuffer.h"
#include "Client.h"


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

extern std::unordered_map<DWORD, stSession*> g_SessionInfo;


stSession* FindSession(DWORD dwSessionID);
stSession* CreateSession(DWORD dwSessionID,SOCKET socket, SOCKADDR_IN sockaddr);
bool DeleteSession(DWORD dwSessionID);

void DisConnect(DWORD dwSessionID);
void DisConnectClient(DWORD dwSessionID);


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
void MakePacket_Charctor(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP);

void Send_ResOtherCharactor(stSession* pSession, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP);
void MakePacket_OtherCharctor(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP);


void Send_ResDeleteCharacter(stClient* pClient);
void MakePacket_DeleteCharacter(CPacket* cpPacket, DWORD dwSessionID);

bool Recv_ReqMoveStart(stSession* pSession, CPacket* cpPacket);
void MakePacket_MoveStart(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

bool Recv_ReqMoveStop(stSession* pSession, CPacket* pPacket);
void MakePacket_MoveStop(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

bool Recv_ReqAttack1(stSession* pSession, CPacket* pPacket);
void MakePacket_Attack1(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

bool Recv_ReqAttack2(stSession* pSession, CPacket* pPacket);
void MakePacket_Attack2(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

bool Recv_ReqAttack3(stSession* pSession, CPacket* pPacket);
void MakePacket_Attack3(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void MakePacket_Damge(CPacket* cpPacket, DWORD dwAttackID, DWORD dwOtherID, char chOtherHP);


bool Recv_ReqEcho(stSession* pSession, CPacket* pPacket);
void MakePacket_Echo(CPacket* cpPacket, DWORD dwTime);

void MakePacket_Synk(CPacket* cpPacket, DWORD dwSessionID, short shX, short shY);

bool AttackHit(stClient* pAtkClient, stClient* pOtherClient, short shRangeX, short shRangeY);

void SendUnicast(stSession* pSession, CPacket* pPakcet);
void SendPacket_SectorOne(int iSectorX, int iSectorY, CPacket* pPacket,stSession *pSession,bool test = false);
void SendPacket_Around(stSession* pSession, CPacket* pPacket, bool bSendMe = false);