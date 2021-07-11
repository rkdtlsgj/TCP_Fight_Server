#include "Client.h"
#include "Network.h"

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

int	g_dwUpdateTick;
int	g_dwOneFrameTick;
int	g_dwTick;

void Update()
{
	if (g_dwUpdateTick == 0)
		g_dwUpdateTick = timeGetTime();

	g_dwOneFrameTick = timeGetTime() - g_dwUpdateTick;

	if ((g_dwOneFrameTick + g_dwTick) < 1000 / 50)
		return;

	else
	{
		g_dwTick += g_dwOneFrameTick - 1000 / 50;
		g_dwUpdateTick = timeGetTime();
	}


	stClient* pClient;
	std::unordered_map<DWORD, stClient*>::iterator clientIter;
	for (clientIter = g_ClientInfo.begin(); clientIter != g_ClientInfo.end();)
	{
		pClient = clientIter->second;
		clientIter++;


		if (0 >= pClient->cHP)
		{
			//사망!
		}
		else
		{
			stSession* pSession = FindSession(pClient->dwSeesionID);
			if (timeGetTime() - pSession->dwLastRecvTime > dfNETWORK_PACKET_RECV_TIMEOUT)
			{
				printf("타임아웃\n");
				//타임아웃!
				continue;
			}

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

			printf("루프\n");
			if (pClient->dwAction >= dfACTION_MOVE_LL && pClient->dwAction <= dfACTION_MOVE_LD)
			{
				if (Sector_UpdateCharacter(pClient))
				{
					//섹터 변경시 클라에게 정보 쏨
					//CharacterSectorUpdatePacket(pCharacter);
				}
			}
		}
	}
}

BOOL FrameSkip(int deltaTime)
{
	if (deltaTime >= 40)
		return TRUE;

	return FALSE;
}


void Sector_AddCharacter(stClient* pClient)
{
	int iSectorX = pClient->shX / dfSECTOR_SIZE_X;
	int iSectorY = pClient->shY / dfSECTOR_SIZE_Y;

	pClient->stOldSector.iX  = pClient->stCurSector.iX;
	pClient->stOldSector.iY = pClient->stCurSector.iY;

	pClient->stCurSector.iX = iSectorX;
	pClient->stCurSector.iY = iSectorY;

	g_sector[iSectorY][iSectorX].push_front(pClient);

}

void Sector_RemoveCharacter(stClient* pClient)
{
	int iSectorX = pClient->shX / dfSECTOR_SIZE_X;
	int iSectorY = pClient->shY / dfSECTOR_SIZE_Y;

	pClient->stOldSector.iX = pClient->stCurSector.iX;
	pClient->stOldSector.iY = pClient->stCurSector.iY;

	pClient->stCurSector.iX = -1;
	pClient->stCurSector.iY = -1;

	std::list<stClient*>::iterator sectorIter;
	for (sectorIter = g_sector[iSectorY][iSectorX].begin(); sectorIter != g_sector[iSectorY][iSectorX].end();)
	{
		if (pClient == *sectorIter)
		{
			g_sector[iSectorY][iSectorX].remove(pClient);
			break;
		}
		++sectorIter;
	}
	
}

bool Sector_UpdateCharacter(stClient* pClient)
{
	Sector_RemoveCharacter(pClient);
	Sector_AddCharacter(pClient);

	return true;
}


void GetSectorAround(int iSectorX, int iSectorY, st_SECOTR_AROUND* pSectorAround)
{
	pSectorAround->iCount = 0;

	iSectorX--;
	iSectorY--;

	for (int y = 0; y < 3; y++)
	{
		if (iSectorY + y < 0 || iSectorY + y > dfSECTOR_MAX_Y)
			continue;

		for (int x = 0; x < 3; x++)
		{
			if (iSectorX + x < 0 || iSectorX + x > dfSECTOR_MAX_X)
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
			pAddSector->stAround[pRemoveSector->iCount] = curSectorAround.stAround[iCntOld];
			pAddSector->iCount++;
		}
	}

}