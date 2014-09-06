#ifndef __LOGGER
#define __LOGGER

#include <Windows.h>
#include "Delegates.h"

typedef class LogData
{
public:
	const char * msg;
	const bool isException;

	LogData( char*, bool );
} *pLogData;

class Logger
{
public:
	static EventHandler<pLogData> LogMessageHandler; // [Handler <= HacksManager.Log], for example
	static void OutLog( char* szFmtStr, ... );
	static void OutEx( char* szFmtStr, ... );
private:
	static void Out( char* msg, bool ex );
};

#endif
