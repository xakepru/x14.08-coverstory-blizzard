#include "Logger.h"

// dark legacy =/
extern PCHAR xvprintf( PCHAR szFmtStr, va_list arg_list );
extern VOID	xfree( LPVOID lpAddr );

// ------------------------------------------------------

LogData::LogData( char* szMsg, bool isEx): msg(szMsg), isException(isEx) {}

// ------------------------------------------------------

EventHandler<pLogData> Logger::LogMessageHandler;

// ------------------------------------------------------

void Logger::OutLog( char* szFmtStr, ... )
{
	va_list arg_list;
	va_start( arg_list, szFmtStr );
		
	PCHAR szMsg = xvprintf( szFmtStr, arg_list );
	Out( szMsg, false );
	xfree(szMsg);
}
// ------------------------------------------------------

void Logger::OutEx( char* szFmtStr, ... )
{
	va_list arg_list;
	va_start( arg_list, szFmtStr );
		
	PCHAR szMsg = xvprintf( szFmtStr, arg_list );
	Out( szMsg, true );
	xfree(szMsg);
}
// ------------------------------------------------------

void Logger::Out( char* msg, bool ex )
{
	pLogData tempdata = new LogData( msg, ex );
	LogMessageHandler( NULL, tempdata );
	delete tempdata;
}