#ifndef __PATCHER
#define __PATCHER

#include "_internal.h"

typedef struct _Patch
{
	DWORD addr;
	DWORD len;
	BYTE  org[32];
} Patch, *pPatch ;

typedef class _Patcher
{
	DWORD dwPatches;
	pPatch Patches;

	DWORD	NewPatch ( DWORD dwAddr, DWORD dwSize );
	void	RemovePatchByIndex(DWORD idx);
	void	SafeSuspendAll(PSYSTEM_THREAD pThreads, DWORD dwThreads, DWORD dwBlockAddr, DWORD dwBlockSize);
	void	WakeUP(PSYSTEM_THREAD pThreads, DWORD dwThreads);

	static _Patcher*instance;
	_Patcher();
public:
	static  _Patcher* Instance();
	static  void ResetInstance();
	
	DWORD	GetAllPatches ( pPatch & pPatches );
	int		FindPatch( DWORD dwAddr );
	BOOL	RemovePatch(DWORD dwAddr);
	VOID	CheckPatches(); // base check for 'readable' address pointed by dwAddr
	VOID	RemoveAllPatches();

	BOOL	MakePatch( PBYTE pAddr, PBYTE pData, DWORD dwDataSize );

} Patcher, *pPatcher;

#endif