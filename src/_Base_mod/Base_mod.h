#ifndef BASE_MOD
#define BASE_MOD

#define MEM_DEBUG
// memory managment
LPVOID 	xalloc( DWORD dwSize );
LPVOID 	xrealloc( LPVOID lpAddr, DWORD dwSize );
DWORD 	xsize( LPVOID lpAddr );
VOID	xfree( LPVOID lpAddr );

// i/o
PCHAR	xvprintf( PCHAR szFmtStr, va_list arg_list );
PCHAR	xprintf( PCHAR szFmtStr, ... );

#endif