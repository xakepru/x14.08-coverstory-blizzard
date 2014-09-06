#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include "Debug_mod.h"
#include "../_Base_mod/Base_mod.h"

#if DBG_OUT == DBG_FILE 
#define DEBUG_FILE_LOG "e:\\dbg_log.txt"
	CRITICAL_SECTION dbg_logfile;
	BOOL dbg_section_initialized = false;
#endif

#if DBG_OUT == DBG_BUFF 
	PCHAR dbg_log_buff = NULL;
#endif

// ---------------------------------------------------
// Debug Logging function
// ---------------------------------------------------

void DbgOut( char * msg, ... )
{
#ifdef ALLOW_DEBUG_OUTPUT

	char *buff, *obuff;
	SYSTEMTIME SystemTime;
	va_list mylist;

	GetSystemTime( &SystemTime );

	va_start( mylist, msg );
 	buff = xvprintf(msg, mylist);

	obuff = xprintf( "%02d:%02d:%02d %03d, Thrd:%05x, %s\r\n", 
			SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds, 
			GetCurrentThreadId(), buff );

	#if DBG_OUT == DBG_STRINGS // debug output into debug strings

		OutputDebugStringA( obuff );

	#elif DBG_OUT == DBG_FILE // debug output into file log

		HANDLE hDbgFile;
		DWORD dwWritten;

		if (!dbg_section_initialized) 
		{
			InitializeCriticalSection( &dbg_logfile );
			dbg_section_initialized = true;
		}

		EnterCriticalSection ( &dbg_logfile );		
		hDbgFile = CreateFile( DEBUG_FILE_LOG, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
									OPEN_ALWAYS, 0, NULL );
		SetFilePointer( hDbgFile, 0, 0, FILE_END );
		WriteFile( hDbgFile, obuff, lstrlen(obuff), &dwWritten, 0 );
		CloseHandle( hDbgFile );
		LeaveCriticalSection( &dbg_logfile );

	#elif DBG_OUT == DBG_BUFF // debug output into global buffer
		
		DWORD dwMsgLen = lstrlen(obuff);

		if ( !dbg_log_buff )
			dbg_log_buff = (PCHAR) xalloc ( dwMsgLen + 1 );
		else
			dbg_log_buff = (PCHAR) xrealloc( dbg_log_buff, lstrlen(dbg_log_buff) + dwMsgLen + 1 );

		lstrcat( dbg_log_buff, obuff);
		
	#endif

	xfree( buff );
	xfree( obuff );

#endif
}

// ---------------------------------------------------