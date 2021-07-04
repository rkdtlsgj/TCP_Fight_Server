#pragma once
#include <Windows.h>
#include <wchar.h>

#include "Debug.h"

int g_iLogLevel = dfLOG_LEVEL_DEBUG;
WCHAR g_szLogBuff[1024];

void Log(const WCHAR* szString, int iLogLevel)
{
	wprintf(L"%s \n", szString);
}
