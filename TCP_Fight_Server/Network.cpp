#include "Network.h"
#include "Define.h"
#include "Debug.h"
#include "PacketDefine.h"
#include "Client.h"

SOCKET listen_sock;
DWORD g_dwKey_UserNumber = 1;
std::unordered_map<DWORD, stSession*> g_SessionInfo;

bool InitServer()
{
	int retval;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	//SOCKET 생성
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		return false;

	//Bind
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(dfNETWORK_PORT);
	retval = bind(listen_sock, (SOCKADDR*)& serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
		return false;

	return true;
}

bool OpenServer()
{
	int retval;

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)	return false;

	return true;
}

void NetWorkProcess()
{
	DWORD userNumber[FD_SETSIZE];
	SOCKET userSocket[FD_SETSIZE];
	int sockCount = 0;

	std::unordered_map < DWORD, stSession*>::iterator sessionIter;

	FD_SET readSet;
	FD_SET writeSet;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	memset(userNumber, 0, sizeof(DWORD) * FD_SETSIZE);
	memset(userSocket, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);


	FD_SET(listen_sock, &readSet);


	userNumber[sockCount] = sockCount;
	userSocket[sockCount] = listen_sock;

	sockCount++;

	for (sessionIter = g_SessionInfo.begin(); sessionIter != g_SessionInfo.end(); )
	{
		if (sessionIter->second->socket != INVALID_SOCKET)
		{
			userNumber[sockCount] = sessionIter->second->dwSeesionID;
			userSocket[sockCount] = sessionIter->second->socket;

			if (sessionIter->second->cSendQ.GetUseSize() > 0)
			{
				FD_SET(sessionIter->second->socket, &writeSet);
			}

			FD_SET(sessionIter->second->socket, &readSet);

			sessionIter++;
			sockCount++;


			if (FD_SETSIZE <= sockCount)
			{
				SelectSocket(userNumber, userSocket, &readSet, &writeSet);
				FD_ZERO(&readSet);
				FD_ZERO(&writeSet);
				memset(userNumber, 0, sizeof(DWORD) * FD_SETSIZE);
				memset(userSocket, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

				FD_SET(listen_sock, &readSet);
				userNumber[0] = 0;
				userSocket[0] = listen_sock;

				sockCount = 1;
			}
		}
	}

	if (sockCount > 0)
	{
		SelectSocket(userNumber, userSocket, &readSet, &writeSet);
	}
}

void SelectSocket(DWORD* dwpUserNumber, SOCKET* pUserSocket, FD_SET* pReadSet, FD_SET* pWriteSet)
{
	timeval Time;
	int iResult;
	bool bProcFlag;

	Time.tv_sec = 0;
	Time.tv_usec = 0;

	iResult = select(0, pReadSet, pWriteSet, 0, &Time);

	if (0 < iResult)
	{
		for (int i = 0; i < FD_SETSIZE; i++)
		{
			bProcFlag = true;
			if (pUserSocket[i] == INVALID_SOCKET)
				continue;

			if (FD_ISSET(pUserSocket[i], pWriteSet))
			{
				--iResult;
				//전송
				NetSend_Proc(dwpUserNumber[i]);
			}

			if (FD_ISSET(pUserSocket[i], pReadSet))
			{
				--iResult;
				if (bProcFlag == true)
				{
					//리슨소켓
					if (dwpUserNumber[i] == 0)
					{
						//Accept
						NetAccept_Proc();
					}
					else
					{
						//printf("전송\n");
						//Recv
						NetRecv_Proc(dwpUserNumber[i]);
					}

				}
				

			}
		}
	}
	else if (iResult == SOCKET_ERROR)
	{
		//에러처리?
	}
}

void NetAccept_Proc()
{
	int addrlen = sizeof(SOCKADDR_IN);
	SOCKADDR_IN sockaddr;

	SOCKET socket = accept(listen_sock, (SOCKADDR*)& sockaddr, &addrlen);
	if (socket == INVALID_SOCKET)
	{
		wprintf(L"Accept Error\n");
		return;
	}

	stSession* pSession = CreateSession(g_dwKey_UserNumber, socket, sockaddr);
	g_SessionInfo.insert(std::make_pair(pSession->dwSeesionID, pSession));


	WCHAR wcAddrs[20];
	InetNtop(AF_INET, &pSession->sockAddr.sin_addr, wcAddrs, sizeof(wcAddrs));

	_LOG(dfLOG_LEVEL_DEBUG, L"[Accept] - %s:%d [UserNo:%d][SocketNo:%d]\n", wcAddrs, ntohs(pSession->sockAddr.sin_port), pSession->dwSeesionID, pSession->socket);


	stClient* pClient = CreateClient(pSession->dwSeesionID);
	g_ClientInfo.insert(std::make_pair(pClient->dwSeesionID, pClient));
	Sector_AddCharacter(pClient);

	Send_ResCharactor(pSession, pClient->dwSeesionID, pClient->byDir, pClient->shX, pClient->shY, pClient->cHP);

	//주변캐릭터 전송.
	
	//CharacterSectorUpdatePacket(pClient);

	st_SECOTR_AROUND stSector;
	GetSectorAround(pClient->stCurSector.iX, pClient->stCurSector.iY, &stSector);

	CPacket cPacket;
	stSession* pSessionOther;
	std::list<stClient*>::iterator sectorIter;
	for (int i = 0; i < stSector.iCount; i++)
	{
		for (sectorIter = g_sector[stSector.stAround[i].iY][stSector.stAround[i].iX].begin(); sectorIter != g_sector[stSector.stAround[i].iY][stSector.stAround[i].iX].end(); ++sectorIter)
		{
			if ((*sectorIter)->dwSeesionID != pClient->dwSeesionID)
			{				
				MakePacket_OtherCharctor(&cPacket, (*sectorIter)->dwSeesionID, (*sectorIter)->byDir, (*sectorIter)->shX, (*sectorIter)->shY, (*sectorIter)->cHP);
				_LOG(dfLOG_LEVEL_DEBUG, L"MakeOtherCharactor[ID:%d][X:%d][Y:%d][PacketSize:%d]\n", (*sectorIter)->dwSeesionID, (*sectorIter)->shX, (*sectorIter)->shY, cPacket.GetDataSize());
				SendUnicast(pSession, &cPacket);

				MakePacket_OtherCharctor(&cPacket, pClient->dwSeesionID, pClient->byDir, pClient->shX, pClient->shY, pClient->cHP);
				pSessionOther = FindSession((*sectorIter)->dwSeesionID);
				SendUnicast(pSessionOther, &cPacket);
			}

			
		}
	}
}

void NetRecv_Proc(DWORD dwUserNumber)
{
	stSession* pSession = FindSession(dwUserNumber);
	if (pSession == NULL)
		return;

	pSession->dwLastRecvTime = timeGetTime();

	int iResult = recv(pSession->socket, pSession->cRecvQ.GetRearBufferPtr(), pSession->cRecvQ.DirectEnqueueSize(), 0);


	if (iResult == SOCKET_ERROR || iResult == 0)
	{
		DisConnect(dwUserNumber);
		return;
	}


	pSession->cRecvQ.MoveRear(iResult);

	if (0 < iResult)
	{
		while (1)
		{
			iResult = CompleteRecvPacket(pSession);

			if (iResult == 1)
				break;

			if (iResult == -1)
			{
				_LOG(dfLOG_LEVEL_ERROR, L" Packet Error User No : %d\n", dwUserNumber);
				return;
			}
		}
	}
}

void NetSend_Proc(DWORD dwUserNumber)
{
	stSession* pClient = FindSession(dwUserNumber);
	if (pClient == NULL)
		return;


	int iSendSize = pClient->cSendQ.DirectDequeueSize();

	if (iSendSize <= 0)
		return;


	int iResult = send(pClient->socket, pClient->cSendQ.GetFrontBufferPtr(), iSendSize, 0);
	//_LOG(dfLOG_LEVEL_DEBUG, L"Socket Send [UserNo:%d][PacketSize:%d]\n", dwUserNumber, iResult);
	if (iResult == SOCKET_ERROR)
	{
		DWORD dwError = WSAGetLastError();
		if (dwError == WSAEWOULDBLOCK)
		{
			_LOG(dfLOG_LEVEL_DEBUG, L"Socket WOULDBLOCK user Number : %d\n", dwUserNumber);
			return;
		}

		DisConnect(dwUserNumber);
		return;
	}
	else
	{
		pClient->cSendQ.MoveFront(iResult);
	}

	return;
}

int CompleteRecvPacket(stSession* pSession)
{
	st_PACKET_HEADER stHeader;

	int qSize = pSession->cRecvQ.GetUseSize();

	if (sizeof(st_PACKET_HEADER) > qSize)
		return 1;


	pSession->cRecvQ.Peek((char*)& stHeader, sizeof(st_PACKET_HEADER));


	if (stHeader.byCode != dfPACKET_CODE)
		return 0xff;

	if (stHeader.bySize + sizeof(st_PACKET_HEADER) > (WORD)qSize)
	{
		//아직 패킷을 다  받지 못하였다
		return 1;
	}


	pSession->cRecvQ.MoveFront(sizeof(st_PACKET_HEADER));

	CPacket cPacket;

	if (stHeader.bySize != pSession->cRecvQ.Dequeue(cPacket.GetBufferPtr(), stHeader.bySize))
	{
		return -1;
	}


	cPacket.MoveWritePos(stHeader.bySize);

	//패킷처리
	if (PacketProc(pSession, stHeader.byType, &cPacket) == false)
		return -1;

	return 0;

}


stSession* CreateSession(DWORD dwSessionID, SOCKET socket, SOCKADDR_IN sockaddr)
{
	stSession* pSession = new stSession;

	pSession->dwSeesionID = dwSessionID;
	pSession->socket = socket;
	pSession->sockAddr = sockaddr;

	pSession->cSendQ.ClearBuffer();
	pSession->cRecvQ.ClearBuffer();

	pSession->dwLastRecvTime = timeGetTime();

	g_dwKey_UserNumber++;

	return pSession;
}

stSession* FindSession(DWORD dwSessionID)
{
	std::unordered_map < DWORD, stSession*>::iterator sessionIter = g_SessionInfo.find(dwSessionID);

	if (sessionIter == g_SessionInfo.end())
		return NULL;

	return sessionIter->second;
}

bool DeleteSession(DWORD dwSessionID)
{
	std::unordered_map < DWORD, stSession*>::iterator sessionIter = g_SessionInfo.find(dwSessionID);

	if (sessionIter == g_SessionInfo.end())
		return false;;


	g_SessionInfo.erase(sessionIter);

	return true;
}


void Send_ResCharactor(stSession* pSession, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP)
{
	CPacket packet;

	MakePacket_Charctor(&packet, dwSessionID, byDir, shX, shY, cHP);
	_LOG(dfLOG_LEVEL_DEBUG, L"MakeCharactor[X:%d][Y:%d]\n", shX, shY);
	SendUnicast(pSession, &packet);
}

void MakePacket_Charctor(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	stHeader.bySize = 10;

	cpPacket->Clear();
	cpPacket->PutData((char*)& stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2
	*cpPacket << cHP;//1
}

void MakePacket_OtherCharctor(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char cHP)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	stHeader.bySize = 10;

	cpPacket->Clear();
	cpPacket->PutData((char*)& stHeader, sizeof(stHeader));

	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2
	*cpPacket << cHP;//1
}

void MakePacket_DeleteCharacter(CPacket* cpPacket, DWORD dwSessionID)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_DELETE_CHARACTER;
	stHeader.bySize = 4;

	cpPacket->Clear();
	cpPacket->PutData((char*)& stHeader, sizeof(stHeader));

	*cpPacket << dwSessionID;//4
}

bool PacketProc(stSession* pSession, BYTE byType, CPacket* pPacket)
{
	//_LOG(dfLOG_LEVEL_DEBUG, L"Recv Packet[ID:%d][TYPE:%d]\n", pSession->dwSeesionID, byType);	

	//stClient* pClient = FindClient(dwUserNumber);
	switch (byType)
	{
	case dfPACKET_CS_MOVE_START:
		return Recv_ReqMoveStart(pSession, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		return Recv_ReqMoveStop(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK1:
		return Recv_ReqAttack1(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK2:
		return Recv_ReqAttack2(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK3:
		return Recv_ReqAttack3(pSession, pPacket);
		break;
	case dfPACKET_CS_ECHO:
		return Recv_ReqEcho(pSession, pPacket);
		break;
	default:
		break;
	}

	return false;
}

bool Recv_ReqMoveStart(stSession* pSession, CPacket* cpPacket)
{
	BYTE byDir;
	short shX, shY;

	*cpPacket >> byDir;
	*cpPacket >> shX;
	*cpPacket >> shY;


	//_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Move Start[ID:%d][X:%d][Y:%d]\n", pSession->dwSeesionID, shX, shY);

	stClient* pClient = FindClient(pSession->dwSeesionID);

	if (pClient == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Move Start NotFind[ID:%d]\n", pSession->dwSeesionID);
		return false;
	}

	if (abs(pClient->shX - shX) > dfERROR_RANGE || abs(pClient->shY - shY) > dfERROR_RANGE)
	{
		//싱크전송
		_LOG(dfLOG_LEVEL_ERROR, L"SynkError[CX:%d][CY:%d][SX:%d][SY:%d]\n", pClient->shX, pClient->shY, shX, shY);
		MakePacket_Synk(cpPacket, pClient->dwSeesionID, pClient->shX, pClient->shY);
		SendPacket_Around(pSession, cpPacket, true);

		shX = pClient->shX;
		shY = pClient->shY;
	}

	pClient->dwAction = byDir;
	pClient->byMoveDir = byDir;


	switch (byDir)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pClient->byDir = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pClient->byDir = dfPACKET_MOVE_DIR_LL;
		break;

		//default:
		//	_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Move Start SwitchDirError [ID:%d][%d]\n", pSession->dwSeesionID, byDir);
		//	break;
	}

	pClient->shX = shX;
	pClient->shY = shY;

	if (Sector_UpdateCharacter(pClient))
	{
		//섹터업데이트패킷.
		CharacterSectorUpdatePacket(pClient);
	}

	MakePacket_MoveStart(cpPacket, pSession->dwSeesionID, byDir, pClient->shX, pClient->shY);
	SendPacket_Around(pSession, cpPacket);


	return true;
}
void MakePacket_MoveStart(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_MOVE_START;
	stHeader.bySize = 9;

	cpPacket->Clear();
	cpPacket->PutData((char*)& stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2
}

bool Recv_ReqMoveStop(stSession* pSession, CPacket* pPacket)
{
	BYTE byDir;
	short shX, shY;

	*pPacket >> byDir;
	*pPacket >> shX;
	*pPacket >> shY;

	

	stClient* pClient = FindClient(pSession->dwSeesionID);

	if (pClient == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Move Stop NotFind[ID:%d]\n", pSession->dwSeesionID);
		return false;
	}

	//_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Move Stop[ID:%d][SX:%d][SY:%d]\n", pSession->dwSeesionID, pClient->stCurSector.iX, pClient->stCurSector.iY);

	//_LOG(dfLOG_LEVEL_DEBUG, L"Test[CX:%d][CY:%d][SX:%d][SY:%d]\n", pClient->shX, pClient->shY, shX, shY);

	if (abs(pClient->shX - shX) > dfERROR_RANGE || abs(pClient->shY - shY) > dfERROR_RANGE)
	{
		//싱크전송
		_LOG(dfLOG_LEVEL_ERROR, L"SynkError[CX:%d][CY:%d][SX:%d][SY:%d]\n", pClient->shX, pClient->shY,shX,shY);
		MakePacket_Synk(pPacket, pClient->dwSeesionID, pClient->shX, pClient->shY);
		SendPacket_Around(pSession, pPacket, true);

		shX = pClient->shX;
		shY = pClient->shY;
	}

	pClient->dwAction = dfACTION_STAND;

	pClient->byMoveDir = byDir;


	switch (byDir)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pClient->byDir = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pClient->byDir = dfPACKET_MOVE_DIR_LL;
		break;

		//default:
		//	_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Move Stop SwitchDirError [ID:%d][%d]\n", pSession->dwSeesionID, byDir);
		//	break;
	}

	pClient->shX = shX;
	pClient->shY = shY;

	if (Sector_UpdateCharacter(pClient))
	{
		//섹터업데이트패킷.
		CharacterSectorUpdatePacket(pClient);
	}

	MakePacket_MoveStop(pPacket, pSession->dwSeesionID, byDir, pClient->shX, pClient->shY);
	SendPacket_Around(pSession, pPacket);


	return true;
}

void MakePacket_MoveStop(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_MOVE_STOP;
	stHeader.bySize = 9;

	cpPacket->Clear();
	cpPacket->PutData((char*)& stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2
}

bool Recv_ReqAttack1(stSession* pSession, CPacket* pPacket)
{
	BYTE byDir;
	short shX, shY;

	*pPacket >> byDir;
	*pPacket >> shX;
	*pPacket >> shY;


	stClient* pClient = FindClient(pSession->dwSeesionID);

	if (pClient == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Attack1 NotFind[ID:%d]\n", pSession->dwSeesionID);
		return false;
	}

	_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Attack1[ID:%d][X:%d][Y:%d]\n", pSession->dwSeesionID,shX,shY);
	pClient->dwAction = dfACTION_ATTACK1;

	switch (byDir)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pClient->byDir = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pClient->byDir = dfPACKET_MOVE_DIR_LL;
		break;
	}

	CPacket cPacket;
	MakePacket_Attack1(&cPacket, pClient->dwSeesionID, pClient->byDir, pClient->shX, pClient->shY);
	SendPacket_Around(pSession, &cPacket);

	st_SECOTR_AROUND stSector;
	GetSectorAround(pClient->stCurSector.iX, pClient->stCurSector.iY, &stSector);


	std::list<stClient*>::iterator sectorIter;
	std::list<stClient*>* pSectorList;
	for (int i = 0; i < stSector.iCount; i++)
	{
		pSectorList = &g_sector[stSector.stAround[i].iY][stSector.stAround[i].iX];
		for (sectorIter = pSectorList->begin(); sectorIter != pSectorList->end(); ++sectorIter)			
		{
			if ((*sectorIter) != pClient)
			{
				if (AttackHit(pClient, (*sectorIter), dfATTACK1_RANGE_X, dfATTACK1_RANGE_Y) == true)
				{
					(*sectorIter)->cHP -= dfATTACK1_DAMAGE;

					MakePacket_Damge(&cPacket, pClient->dwSeesionID, (*sectorIter)->dwSeesionID, (*sectorIter)->cHP);
					SendPacket_Around(pSession, &cPacket, true);
				}
			}
		}
	}

	return true;

}

void MakePacket_Attack1(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_ATTACK1;
	stHeader.bySize = 9;

	cpPacket->Clear();
	cpPacket->PutData((char*)&stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2	
}


bool Recv_ReqAttack2(stSession* pSession, CPacket* pPacket)
{
	BYTE byDir;
	short shX, shY;


	*pPacket >> byDir;
	*pPacket >> shX;
	*pPacket >> shY;


	stClient* pClient = FindClient(pSession->dwSeesionID);

	if (pClient == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Attack2 NotFind[ID:%d]\n", pSession->dwSeesionID);
		return false;
	}
	_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Attack2[ID:%d][X:%d][Y:%d]\n", pSession->dwSeesionID, shX, shY);
	pClient->dwAction = dfACTION_ATTACK2;

	switch (byDir)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pClient->byDir = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pClient->byDir = dfPACKET_MOVE_DIR_LL;
		break;
	}

	CPacket cPacket;
	MakePacket_Attack2(&cPacket, pClient->dwSeesionID, pClient->byDir, pClient->shX, pClient->shY);
	SendPacket_Around(pSession, &cPacket);

	st_SECOTR_AROUND stSector;
	GetSectorAround(pClient->stCurSector.iX, pClient->stCurSector.iY, &stSector);


	std::list<stClient*>::iterator sectorIter;
	std::list<stClient*>* pSectorList;
	for (int i = 0; i < stSector.iCount; i++)
	{
		pSectorList = &g_sector[stSector.stAround[i].iY][stSector.stAround[i].iX];
		for (sectorIter = pSectorList->begin(); sectorIter != pSectorList->end(); ++sectorIter)
		{
			if ((*sectorIter) != pClient)
			{
				if (AttackHit(pClient, (*sectorIter),dfATTACK2_RANGE_X, dfATTACK2_RANGE_Y) == true)
				{
					(*sectorIter)->cHP -= dfATTACK2_DAMAGE;

					MakePacket_Damge(&cPacket, pClient->dwSeesionID, (*sectorIter)->dwSeesionID, (*sectorIter)->cHP);
					SendPacket_Around(pSession, &cPacket, true);
				}
			}
		}
	}

	return true;

}

void MakePacket_Attack2(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_ATTACK2;
	stHeader.bySize = 9;

	cpPacket->Clear();
	cpPacket->PutData((char*)&stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2	
}

bool Recv_ReqAttack3(stSession* pSession, CPacket* pPacket)
{
	BYTE byDir;
	short shX, shY;

	*pPacket >> byDir;
	*pPacket >> shX;
	*pPacket >> shY;


	stClient* pClient = FindClient(pSession->dwSeesionID);

	if (pClient == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Attack3 NotFind[ID:%d]\n", pSession->dwSeesionID);
		return false;
	}
	_LOG(dfLOG_LEVEL_DEBUG, L"Charactor Attack3[ID:%d][X:%d][Y:%d]\n", pSession->dwSeesionID, shX, shY);
	pClient->dwAction = dfACTION_ATTACK3;

	switch (byDir)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pClient->byDir = dfPACKET_MOVE_DIR_RR;
		break;

	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		pClient->byDir = dfPACKET_MOVE_DIR_LL;
		break;
	}

	CPacket cPacket;
	MakePacket_Attack3(&cPacket, pClient->dwSeesionID, pClient->byDir, pClient->shX, pClient->shY);
	SendPacket_Around(pSession, &cPacket);

	st_SECOTR_AROUND stSector;
	GetSectorAround(pClient->stCurSector.iX, pClient->stCurSector.iY, &stSector);


	std::list<stClient*>::iterator sectorIter;
	std::list<stClient*>* pSectorList;
	for (int i = 0; i < stSector.iCount; i++)
	{
		pSectorList = &g_sector[stSector.stAround[i].iY][stSector.stAround[i].iX];
		for (sectorIter = pSectorList->begin(); sectorIter != pSectorList->end(); ++sectorIter)
		{
			if ((*sectorIter) != pClient)
			{
				if (AttackHit(pClient, (*sectorIter), dfATTACK3_RANGE_X, dfATTACK3_RANGE_Y) == true)
				{
					(*sectorIter)->cHP -= dfATTACK3_DAMAGE;

					MakePacket_Damge(&cPacket, pClient->dwSeesionID, (*sectorIter)->dwSeesionID, (*sectorIter)->cHP);
					SendPacket_Around(pSession, &cPacket, true);
				}
			}
		}
	}

	return true;

}

void MakePacket_Attack3(CPacket* cpPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_ATTACK3;
	stHeader.bySize = 9;

	cpPacket->Clear();
	cpPacket->PutData((char*)&stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << byDir;//1
	*cpPacket << shX;//2
	*cpPacket << shY;//2	
}

void MakePacket_Damge(CPacket* cpPacket, DWORD dwAttackID, DWORD dwOtherID, char chOtherHP)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_DAMAGE;
	stHeader.bySize = 9;

	cpPacket->Clear();
	cpPacket->PutData((char*)&stHeader, sizeof(stHeader));


	*cpPacket << dwAttackID;//4
	*cpPacket << dwOtherID;//4
	*cpPacket << chOtherHP;//1
}

bool AttackHit(stClient* pAtkClient, stClient* pOtherClient,short shRangeX, short shRangeY)
{
	short shX = pAtkClient->shX - pOtherClient->shX;
	short shY = pAtkClient->shY - pOtherClient->shY;

	if (pAtkClient->byDir == dfPACKET_MOVE_DIR_LL)
	{
		if (shX < 0 || shY < 0)
		{
			return false;
		}

		if (shX < shRangeX && shY < shRangeY)
		{
			return true;
		}
	}
	else if (pAtkClient->byDir == dfPACKET_MOVE_DIR_RR)
	{
		if (shX > 0 || shY > 0)
		{
			return false;
		}

		if (shX > -shRangeX && shY > -shRangeY)
		{
			return true;
		}
	}

	return false;
}
bool Recv_ReqEcho(stSession* pSession, CPacket* pPacket)
{
	DWORD time;
	*pPacket >> time;

	CPacket cPacket;
	MakePacket_Echo(&cPacket, time);

	SendUnicast(pSession, &cPacket);

	return true;
}

void MakePacket_Echo(CPacket* cpPacket,DWORD dwTime)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_ECHO;
	stHeader.bySize = 4;

	cpPacket->Clear();
	cpPacket->PutData((char*)&stHeader, sizeof(stHeader));

	*cpPacket << dwTime;//4

}

void MakePacket_Synk(CPacket* cpPacket, DWORD dwSessionID, short shX, short shY)
{
	st_PACKET_HEADER stHeader;

	stHeader.byCode = dfPACKET_CODE;
	stHeader.byType = dfPACKET_SC_SYNC;
	stHeader.bySize = 7;

	cpPacket->Clear();
	cpPacket->PutData((char*)&stHeader, sizeof(stHeader));


	*cpPacket << dwSessionID;//4
	*cpPacket << shX;//2
	*cpPacket << shY;//2	
}

void SendUnicast(stSession* pSession, CPacket* pPakcet)
{
	//_LOG(dfLOG_LEVEL_DEBUG, L"SendUnicast[ID:%d][PacketSize:%d]\n", pSession->dwSeesionID, pPakcet->GetDataSize());
	pSession->cSendQ.Enqueue((char*)pPakcet->GetBufferPtr(), pPakcet->GetDataSize());
}

void SendPacket_SectorOne(int iSectorX, int iSectorY, CPacket* pPacket, stSession* pSession)
{
	std::list<stClient*>::iterator sectorIter;
	for (sectorIter = g_sector[iSectorY][iSectorX].begin(); sectorIter != g_sector[iSectorY][iSectorX].end(); ++sectorIter)
	{
		if (pSession != NULL && (*sectorIter)->dwSeesionID == pSession->dwSeesionID)
			continue;

		SendUnicast(FindSession((*sectorIter)->dwSeesionID), pPacket);

	}
}


void SendPacket_Around(stSession* pSession, CPacket* pPacket, bool bSendMe)
{
	stClient* pClient = FindClient(pSession->dwSeesionID);

	st_SECOTR_AROUND stSector;
	GetSectorAround(pClient->stCurSector.iX, pClient->stCurSector.iY, &stSector);

	for (int i = 0; i < stSector.iCount; i++)
	{
		if (!bSendMe)
			SendPacket_SectorOne(stSector.stAround[i].iX, stSector.stAround[i].iY, pPacket, pSession);
		else
			SendPacket_SectorOne(stSector.stAround[i].iX, stSector.stAround[i].iY, pPacket, NULL);
	}
}

void DisConnectClient(DWORD dwSessionID)
{
	stClient* pClient = FindClient(dwSessionID);

	if (pClient == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"DisConnect pClient NULL[ID:%d]\n", dwSessionID);
		return;
	}

	if (DeleteClient(dwSessionID) == true)
	{
		Send_ResDeleteCharacter(pClient);
		delete pClient;
	}
}
void DisConnect(DWORD dwSessionID)
{
	stSession* pSession = FindSession(dwSessionID);
	
	if (pSession == NULL)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"DisConnect Session NULL[ID:%d]\n", dwSessionID);
		return;
	}

	closesocket(pSession->socket);

	DisConnectClient(dwSessionID);

	if (DeleteSession(dwSessionID) == true)
	{
		delete pSession;
	}


	_LOG(dfLOG_LEVEL_DEBUG, L"DisConnect Player[ID:%d]\n", dwSessionID);

}

void Send_ResDeleteCharacter(stClient* pClient)
{
	stSession* pSession = FindSession(pClient->dwSeesionID);

	CPacket packet;
	MakePacket_DeleteCharacter(&packet, pSession->dwSeesionID);
	_LOG(dfLOG_LEVEL_DEBUG, L"Remove Character[ID:%d]\n", pSession->dwSeesionID);

	int iSectorX = pClient->shX / dfSECTOR_SIZE_X;
	int iSectorY = pClient->shY / dfSECTOR_SIZE_Y;
	st_SECOTR_AROUND stSector;
	GetSectorAround(iSectorX, iSectorY, &stSector);
	Sector_RemoveCharacter(pClient);

	for (int i = 0; i < stSector.iCount; i++)
	{
		SendPacket_SectorOne(stSector.stAround[i].iX, stSector.stAround[i].iY, &packet, pSession);
	}
}