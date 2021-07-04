#pragma once

#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_WARRING 1
#define dfLOG_LEVEL_ERROR 2

extern	int g_iLogLevel;
extern	WCHAR g_szLogBuff[1024];

void Log(const WCHAR* szString, int iLogLevel);

#define _LOG(LogLevel, fmt, ...)												\
do																				\
{																				\
	if (g_iLogLevel <= LogLevel)												\
	{																			\
		wsprintf(g_szLogBuff,fmt, ##__VA_ARGS__);								\
		Log(g_szLogBuff,LogLevel);												\
	}																			\
}while(0)																		\
