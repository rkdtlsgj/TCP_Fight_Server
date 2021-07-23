#include "stdafx.h"

std::unordered_map<DWORD, stClient*> g_ClientInfo;
std::list<stClient*> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];


int TestY = 64;

stClient* CreateClient(DWORD dwSessionID)
{
	stClient* pClient = new stClient;

	pClient->dwSeesionID = dwSessionID;
	pClient->dwAction = dfACTION_STAND;

	pClient->byDir = dfACTION_MOVE_RR;
	pClient->byMoveDir = dfACTION_STAND;

	pClient->shX = rand() % dfMAP_WIDTH;
	pClient->shY = rand() % dfMAP_HEIGHT;

	//pClient->shX = 0;
	//pClient->shY = 0;


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

bool DeleteClient(DWORD dwSessionID)
{
	std::unordered_map < DWORD, stClient*>::iterator clientIter = g_ClientInfo.find(dwSessionID);

	if (clientIter == g_ClientInfo.end())
		return false;

	g_ClientInfo.erase(clientIter);
	return true;
}



int waitTime = 1000 / 50;
int lastTime = timeGetTime();
int nowTime;
int delayTIme;

int timeCheck = 0;
int logicCount = 0;
int frameCheck = 0;

int iAcceptsTPS = 0;

void Update()
{
	nowTime = timeGetTime();
	delayTIme = nowTime - lastTime;
	logicCount++;

	if (nowTime - timeCheck > 1000)
	{
		if (frameCheck != 50)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"[Frame:%d][Logic:%d][Accept:%d]\n", frameCheck, logicCount, iAcceptsTPS);
		}
		else
		{
			_LOG(dfLOG_LEVEL_ERROR, L"[Frame:%d][Logic:%d][Accept:%d]\n", frameCheck,logicCount, iAcceptsTPS);
		}


		iAcceptsTPS = 0;
		logicCount = 0;
		frameCheck = 0;
		timeCheck = nowTime;
	}

	if (delayTIme < waitTime)
	{		
		return;
	}

	lastTime = nowTime - (delayTIme - waitTime);
	frameCheck++;
	

	stClient* pClient;
	std::unordered_map<DWORD, stClient*>::iterator clientIter;
	for (clientIter = g_ClientInfo.begin(); clientIter != g_ClientInfo.end();)
	{
		pClient = clientIter->second;
		clientIter++;

		if (0 >= pClient->cHP)
		{
			DisConnect(pClient->dwSeesionID);
			//사망!
		}
		else
		{
			//stSession* pSession = FindSession(pClient->dwSeesionID);
			//if (timeGetTime() - pSession->dwLastRecvTime > dfNETWORK_PACKET_RECV_TIMEOUT)
			//{
			//	//타임아웃!
			//	_LOG(dfLOG_LEVEL_ERROR, L"TimeOut\n");
			//	continue;
			//}

			switch (pClient->dwAction)
			{
			case dfACTION_MOVE_LL:
				if (pClient->shX - dfSPEED_PLAYER_X > dfRANGE_MOVE_LEFT)
				{
					pClient->shX -= dfSPEED_PLAYER_X;
				}
				break;

			case dfACTION_MOVE_LU:
				if ((pClient->shX - dfSPEED_PLAYER_X > dfRANGE_MOVE_LEFT) &&
					(pClient->shY - dfSPEED_PLAYER_Y > dfRANGE_MOVE_TOP))
				{
					pClient->shX -= dfSPEED_PLAYER_X;
					pClient->shY -= dfSPEED_PLAYER_Y;
				}
				break;

			case dfACTION_MOVE_LD:
				if ((pClient->shX - dfSPEED_PLAYER_X > dfRANGE_MOVE_LEFT) &&
					(pClient->shY + dfSPEED_PLAYER_Y < dfRANGE_MOVE_BOTTOM))
				{
					pClient->shX -= dfSPEED_PLAYER_X;
					pClient->shY += dfSPEED_PLAYER_Y;
				}
				break;

			case dfACTION_MOVE_RR:
				if (pClient->shX + dfSPEED_PLAYER_X < dfRANGE_MOVE_RIGHT)
				{
					pClient->shX += dfSPEED_PLAYER_X;
				}
				break;

			case dfACTION_MOVE_RU:
				if ((pClient->shX + dfSPEED_PLAYER_X < dfRANGE_MOVE_RIGHT) &&
					(pClient->shY - dfSPEED_PLAYER_Y > dfRANGE_MOVE_TOP))
				{
					pClient->shX += dfSPEED_PLAYER_X;
					pClient->shY -= dfSPEED_PLAYER_Y;
				}
				break;

			case dfACTION_MOVE_RD:
				if ((pClient->shX + dfSPEED_PLAYER_X < dfRANGE_MOVE_RIGHT) &&
					(pClient->shY + dfSPEED_PLAYER_Y < dfRANGE_MOVE_BOTTOM))
				{
					pClient->shX += dfSPEED_PLAYER_X;
					pClient->shY += dfSPEED_PLAYER_Y;
				}
				break;

			case dfACTION_MOVE_UU:
				if (pClient->shY - dfSPEED_PLAYER_Y > dfRANGE_MOVE_TOP)
				{
					pClient->shY -= dfSPEED_PLAYER_Y;
				}
				break;

			case dfACTION_MOVE_DD:
				if (pClient->shY + dfSPEED_PLAYER_Y < dfRANGE_MOVE_BOTTOM)
				{
					pClient->shY += dfSPEED_PLAYER_Y;
				}
				break;
			}
			
			if (pClient->dwAction >= dfACTION_MOVE_LL && pClient->dwAction <= dfACTION_MOVE_LD)
			{
				if (Sector_UpdateCharacter(pClient))
				{
					//섹터 변경시 클라에게 정보 쏨
					CharacterSectorUpdatePacket(pClient);
				}
			}
		}
	}
}



void Sector_AddCharacter(stClient* pClient)
{
	int iSectorX = pClient->shX / dfSECTOR_SIZE_X;
	int iSectorY = pClient->shY / dfSECTOR_SIZE_Y;

	//_LOG(dfLOG_LEVEL_DEBUG, L"OldSector[X:%d][Y:%d]\n", pClient->stCurSector.iX, pClient->stCurSector.iY);

	g_sector[iSectorY][iSectorX].push_front(pClient);

	pClient->stOldSector.iX  = pClient->stCurSector.iX;
	pClient->stOldSector.iY = pClient->stCurSector.iY;

	pClient->stCurSector.iX = iSectorX;
	pClient->stCurSector.iY = iSectorY;	
	
	//_LOG(dfLOG_LEVEL_ERROR, L"AddSector[X:%d][Y:%d]\n", iSectorX, iSectorY);

}

void Sector_RemoveCharacter(stClient* pClient)
{
	int iSectorX = pClient->stCurSector.iX;
	int iSectorY = pClient->stCurSector.iY;

	std::list<stClient*>::iterator sectorIter;
	for (sectorIter = g_sector[iSectorY][iSectorX].begin(); sectorIter != g_sector[iSectorY][iSectorX].end();	++sectorIter)
	{
		if (pClient == *sectorIter)
		{			
			g_sector[iSectorY][iSectorX].erase(sectorIter);			
			break;
		}	
	}	
}

bool Sector_UpdateCharacter(stClient* pClient)
{
	int iNowSectorX = pClient->stCurSector.iX;
	int iNowSectorY = pClient->stCurSector.iY;

	int iNewSectorX = pClient->shX / dfSECTOR_SIZE_X;
	int iNewSectorY = pClient->shY / dfSECTOR_SIZE_Y;

	if (iNewSectorX == iNowSectorX && iNewSectorY == iNowSectorY)
		return false;

	Sector_RemoveCharacter(pClient);
	Sector_AddCharacter(pClient);

	pClient->stCurSector.iX = iNewSectorX;
	pClient->stCurSector.iY = iNewSectorY;
	pClient->stOldSector.iX = iNowSectorX;
	pClient->stOldSector.iY = iNowSectorY;

	return true;
}


void GetSectorAround(int iSectorX, int iSectorY, st_SECOTR_AROUND* pSectorAround)
{
	pSectorAround->iCount = 0;

	iSectorX--;
	iSectorY--;

	for (int y = 0; y < 3; y++)
	{
		if (iSectorY + y < 0 || iSectorY + y >= dfSECTOR_MAX_Y)
			continue;

		for (int x = 0; x < 3; x++)
		{
			if (iSectorX + x < 0 || iSectorX + x >= dfSECTOR_MAX_X)
				continue;

			pSectorAround->stAround[pSectorAround->iCount].iX = iSectorX + x;
			pSectorAround->stAround[pSectorAround->iCount].iY = iSectorY + y;

			pSectorAround->iCount++;
		}
	}
}

void GetUpdateSectorAround(stClient* pClient, st_SECOTR_AROUND* pRemoveSector, st_SECOTR_AROUND* pAddSector)
{
	int iCntOld, iCntCur;

	bool bFind;
	st_SECOTR_AROUND oldSectorAround, curSectorAround;

	oldSectorAround.iCount = 0;
	curSectorAround.iCount = 0;

	pRemoveSector->iCount = 0;
	pAddSector->iCount = 0;

	GetSectorAround(pClient->stOldSector.iX, pClient->stOldSector.iY, &oldSectorAround);
	GetSectorAround(pClient->stCurSector.iX, pClient->stCurSector.iY, &curSectorAround);


	for (iCntOld = 0; iCntOld < oldSectorAround.iCount; iCntOld++)
	{
		bFind = false;

		for (iCntCur = 0; iCntCur < curSectorAround.iCount; iCntCur++)
		{
			if (oldSectorAround.stAround[iCntOld].iX == curSectorAround.stAround[iCntCur].iX &&
				oldSectorAround.stAround[iCntOld].iY == curSectorAround.stAround[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}

		if (bFind == false)
		{
			pRemoveSector->stAround[pRemoveSector->iCount] = oldSectorAround.stAround[iCntOld];
			pRemoveSector->iCount++;
		}
	}

	for (iCntCur = 0; iCntCur < curSectorAround.iCount; iCntCur++)
	{
		bFind = false;

		for (iCntOld = 0; iCntOld < oldSectorAround.iCount; iCntOld++)
		{
			if (oldSectorAround.stAround[iCntOld].iX == curSectorAround.stAround[iCntCur].iX &&
				oldSectorAround.stAround[iCntOld].iY == curSectorAround.stAround[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}

		if (bFind == false)
		{
			pAddSector->stAround[pAddSector->iCount] = curSectorAround.stAround[iCntCur];
			pAddSector->iCount++;
		}
	}

}

//이전섹터 캐릭터 삭제(남은사람)
//이전섹터 캐릭터 삭제(나한테)
//새로운섹터 캐릭터 추가
//새로운섹터 다른캐릭터 받기
void CharacterSectorUpdatePacket(stClient* pClient)
{
	st_SECOTR_AROUND stRemoveSector;
	st_SECOTR_AROUND stAddSector;
	CPacket cPacket;
	std::list<stClient*>* pSectorList;

	GetUpdateSectorAround(pClient, &stRemoveSector, &stAddSector);


	MakePacket_DeleteCharacter(&cPacket, pClient->dwSeesionID);

	
	//이전섹터에서 나의 캐릭터삭제
	for (int i = 0; i < stRemoveSector.iCount; i++)
	{
		SendPacket_SectorOne(stRemoveSector.stAround[i].iX, stRemoveSector.stAround[i].iY, &cPacket,NULL,true);				
		//여기서 나한테 삭제를 보내면 MakePacket_DeleteCharacter를 여러번 호출해야됨.
	}	

	//이전섹터에 있는 다른 캐릭터삭제
	std::list<stClient*>::iterator sectorIter;	
	//stSession* stSession = FindSession(pClient->dwSeesionID);
	for (int i = 0; i < stRemoveSector.iCount; i++)
	{
		for (sectorIter = g_sector[stRemoveSector.stAround[i].iY][stRemoveSector.stAround[i].iX].begin(); sectorIter != g_sector[stRemoveSector.stAround[i].iY][stRemoveSector.stAround[i].iX].end();			++sectorIter)
		{
			MakePacket_DeleteCharacter(&cPacket, (*sectorIter)->dwSeesionID);
			SendUnicast(pClient->pSession, &cPacket);
		}
	}


	//새로운색터 캐릭터 추가(내캐릭터들 다른사람들한테)
	MakePacket_OtherCharctor(&cPacket, pClient->dwSeesionID, pClient->byDir, pClient->shX, pClient->shY, pClient->cHP);

	for (int i = 0; i < stAddSector.iCount; i++)
	{
		SendPacket_SectorOne(stAddSector.stAround[i].iX, stAddSector.stAround[i].iY, &cPacket, NULL);		
	}

	MakePacket_MoveStart(&cPacket, pClient->dwSeesionID, pClient->byMoveDir, pClient->shX, pClient->shY);

	for (int i = 0; i < stAddSector.iCount; i++)
	{
		SendPacket_SectorOne(stAddSector.stAround[i].iX, stAddSector.stAround[i].iY, &cPacket, NULL);
	}

	//새로운섹터 캐릭터 추가(다른사람 캐릭터를 나한테)
	for (int i = 0; i < stAddSector.iCount; i++)
	{
		pSectorList = &g_sector[stAddSector.stAround[i].iY][stAddSector.stAround[i].iX];

		for (sectorIter = pSectorList->begin(); sectorIter != pSectorList->end(); ++sectorIter)
		{
			if ((*sectorIter) != pClient)
			{
				MakePacket_OtherCharctor(&cPacket, (*sectorIter)->dwSeesionID, (*sectorIter)->byDir, (*sectorIter)->shX, (*sectorIter)->shY, (*sectorIter)->cHP);
				SendUnicast(pClient->pSession, &cPacket);

				switch ((*sectorIter)->dwAction)
				{
				case dfACTION_MOVE_DD:
				case dfACTION_MOVE_LD:
				case dfACTION_MOVE_LL:
				case dfACTION_MOVE_UU:
				case dfACTION_MOVE_LU:
				case dfACTION_MOVE_RU:
				case dfACTION_MOVE_RR:
					MakePacket_MoveStart(&cPacket, (*sectorIter)->dwSeesionID, (*sectorIter)->byMoveDir, (*sectorIter)->shX, (*sectorIter)->shY);
					//stSession = FindSession((*sectorIter)->dwSeesionID);
					//SendPacket_Around(stSession, &cPacket);
					SendUnicast(pClient->pSession, &cPacket);
					break;
				}
			}
		}
	}
}

bool AttackHit(stClient* pAtkClient, stClient* pOtherClient, short shRangeX, short shRangeY)
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

void DebugSector()
{
	int count = 0;
	for (int y = 0; y < dfSECTOR_MAX_Y; y++)
	{
		for (int x = 0; x < dfSECTOR_MAX_X; x++)
		{
			count += g_sector[y][x].size();
			wprintf(L"%zd ", g_sector[y][x].size());
		}
		wprintf(L"\n");		
	}
	wprintf(L"%d\n", count);
}