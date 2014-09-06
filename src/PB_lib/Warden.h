#ifndef __WARDEN
#define __WARDEN

#include "HackBase.h"

class Warden
{
public:
	static	void __cdecl	ProcessScan( PBYTE pOut, DWORD dwAddr, DWORD dwSize );
	static	bool	PatchWardenScan( PVOID pMemBase=NULL, DWORD dwSize=0 );
};

#endif