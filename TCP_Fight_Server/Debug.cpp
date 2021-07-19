#pragma once
#include <Windows.h>
#include <wchar.h>
#include <time.h>
#include "Debug.h"

int g_iLogLevel = dfLOG_LEVEL_DEBUG;
WCHAR g_szLogBuff[1024];

void Log(const WCHAR* szString, int iLogLevel)
{
	time_t t = time(NULL);
	struct tm tm;
	localtime_s(&tm, &t);

	wprintf(L"%04d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	wprintf(L"%s", szString);
}
