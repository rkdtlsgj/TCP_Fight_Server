#pragma once
#include "stdafx.h"

int g_iLogLevel = dfLOG_LEVEL_ERROR;

WCHAR g_szLogFileName[1024];
WCHAR g_szLogBuff[1024];
WCHAR g_szLogFileSave[1024];

bool bFileSave = false;

void Log(const WCHAR* szString, int iLogLevel)
{
	time_t t = time(NULL);
	struct tm tm;
	localtime_s(&tm, &t);



	wsprintf(g_szLogFileName, L"%04d-%02d-%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	FILE* fp;
	_wfopen_s(&fp, g_szLogFileName, L"ab");
	fseek(fp, 0, SEEK_END);

	wsprintf(g_szLogFileSave, L"%04d_%02d_%02d %02d:%02d:%02d %s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, szString);
	fwprintf(fp, L"%s", g_szLogFileSave);
	
	fclose(fp);

	wprintf(L"%s", g_szLogFileSave);
}
