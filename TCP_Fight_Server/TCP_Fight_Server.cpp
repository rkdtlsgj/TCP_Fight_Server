#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include "stdafx.h"
#include "CPacket.h"
using namespace std;


bool g_bShutdown;

void ServerControl();

int main()
{
	timeBeginPeriod(1);

	if (InitServer() == false)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"Init Error\n");		
		return -1;
	}

	if (OpenServer() == false)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"Listen Error\n");		
		return -1;
	}


	while (g_bShutdown == false)
	{
		NetWorkProcess();
		Update();

		ServerControl();
	}
		
}


void ServerControl()
{
	static bool bControlMode = false;

	if (_kbhit())
	{
		WCHAR ControlKey = _getwch();

		if (L'u' == ControlKey || L'U' == ControlKey)
		{
			bControlMode = true;

			wprintf(L"Control Mode - Pres Q - Quit\n");
			wprintf(L"Control Mode - Pres L - KeyLock\n");
			wprintf(L"Control Mode - Pres I - DebugSector\n");
		}

		if ((L'l' == ControlKey || L'L' == ControlKey) && bControlMode)
		{
			bControlMode = false;

			wprintf(L"Control Lock ! Pres U - Control UnLock\n");
		}

		if ((L'i' == ControlKey || L'I' == ControlKey) && bControlMode)
		{
			DebugSector();
		}

		if ((L'q' == ControlKey || L'Q' == ControlKey) && bControlMode)
		{
			g_bShutdown = true;
		}
	}

}
