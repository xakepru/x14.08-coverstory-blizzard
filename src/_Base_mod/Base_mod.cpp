#include <windows.h>
#include <stdio.h>
#include "Base_mod.h"

#ifdef MEM_DEBUG
DWORD dwMemAlloc = 0;
#endif

// ---------------------------------------------------
// RDTSC
// ---------------------------------------------------



// ---------------------------------------------------
// Memory managment functions
// ---------------------------------------------------

LPVOID	xalloc( DWORD dwSize)
{
	//LPVOID lpRet = VirtualAlloc( 0, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE ); ,,,,,, HEAP_ZERO_MEMORY
	LPVOID lpRet = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize );
	//if ( lpRet ) memset( lpRet, 0, dwSize );
	#ifdef MEM_DEBUG
	dwMemAlloc += xsize( lpRet );
	#endif
	
	return lpRet;
}
// ---------------------------------------------------

LPVOID	xrealloc( LPVOID lpAddr, DWORD dwSize )
{
	LPVOID lpNewAddr;

	if ( lpAddr) 
	{
		#ifdef MEM_DEBUG
			dwMemAlloc -= xsize( lpAddr );
		#endif

		lpNewAddr = HeapReAlloc( GetProcessHeap(), 0, lpAddr, dwSize );
		if ( !lpNewAddr ) 
		{
			xfree( lpAddr );
		}
		else
		{
			//memset( lpNewAddr, 0, dwSize );
			#ifdef MEM_DEBUG
				dwMemAlloc += dwSize;
			#endif
		}
	}
	else
		lpNewAddr = xalloc ( dwSize );

	return lpNewAddr;
}
// ---------------------------------------------------

DWORD	xsize( LPVOID lpAddr )
{
	int iSize = 0;
	if ( lpAddr)
	{
		iSize = HeapSize( GetProcessHeap(), 0, lpAddr);
		//MEMORY_BASIC_INFORMATION MemInfo;
		//VirtualQuery(lpAddr, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION));
		//return MemInfo.RegionSize;
		if (iSize == -1)iSize = 0;
	}
	return iSize;
}
// ---------------------------------------------------
VOID 	xfree( LPVOID lpAddr )
{
	#ifdef MEM_DEBUG
	dwMemAlloc -= xsize( lpAddr );
	#endif

	HeapFree( GetProcessHeap(), 0, lpAddr);
	//VirtualFree(lpAddr, 0, MEM_RELEASE);
}
// ---------------------------------------------------

DWORD	xmem( )
{
	#ifdef MEM_DEBUG
		return dwMemAlloc;
	#else
		return 0;
	#endif
		
}
// ---------------------------------------------------

// ---------------------------------------------------
// Dynamic buffered formatted output
// ---------------------------------------------------

PCHAR xvprintf( PCHAR szFmtStr, va_list arg_list )
{
	PCHAR buff = 0;
	DWORD size = 0;
	BOOL  flag = false;

	while ( !flag )
	{
		if ( size ) 
		{
			buff = (char*) xalloc( ++size );
			flag = true;
		}
		size = _vsnprintf(buff, size, szFmtStr, arg_list);
	}

	return buff;
}
// ---------------------------------------------------

PCHAR xprintf( PCHAR szFmtStr, ... )
{
	va_list arg_list;
	va_start( arg_list, szFmtStr );
		
	return xvprintf( szFmtStr, arg_list );
}
// ---------------------------------------------------