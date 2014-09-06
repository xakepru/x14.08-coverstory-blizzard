#include "Patcher.h"

extern LPVOID 	xalloc( DWORD dwSize );
extern LPVOID 	xrealloc( LPVOID lpAddr, DWORD dwSize );
extern VOID	xfree( LPVOID lpAddr );
 
Patcher* Patcher::instance = NULL;

// ------------------------------------------------------

Patcher::_Patcher() 
{
	dwPatches = 0;
	Patches = NULL;
}

// ------------------------------------------------------

_Patcher* Patcher::Instance()
{
	if (instance==NULL) instance = new Patcher();
	return instance;
}

// ------------------------------------------------------

void Patcher::ResetInstance() // for new process
{
	instance=NULL;
}

// ------------------------------------------------------

DWORD Patcher::NewPatch ( DWORD dwAddr, DWORD dwSize )
{
	pPatch pPatchStruc = NULL;
	BOOL flAdded = false;

	CheckPatches();

	// already patched?
	int idx = FindPatch(dwAddr);
	if (idx)
		RemovePatchByIndex( idx ); // restore orig data before patching
	else
		Patches = (pPatch) xrealloc( Patches, (dwPatches+1) * sizeof(Patch));	

	pPatchStruc  = &Patches[dwPatches]; // last element
	pPatchStruc->addr = dwAddr;
	pPatchStruc->len = dwSize;
	memcpy( pPatchStruc->org , (PVOID) dwAddr, dwSize );
	dwPatches++;
	return dwPatches;
}

// ------------------------------------------------------

DWORD Patcher::GetAllPatches( pPatch & pPatches )
{
	if (  dwPatches )
	{
		pPatches = Patches; // direct access
	}
	return dwPatches;
}

// ------------------------------------------------------

int	Patcher::FindPatch( DWORD dwAddr )
{
	for (unsigned int idx=0; idx< dwPatches; idx++)
	{
		if ( Patches[idx].addr == dwAddr ) // found
		{
			return idx;
		}
	}
	return 0;
}

// ------------------------------------------------------

BOOL	Patcher::RemovePatch(DWORD dwAddr)
{
	bool found = false;
	int idx = FindPatch(dwAddr);
	 
	if (idx)
	{
		RemovePatchByIndex(idx);
		found = true;
	}
	return found;
}

// ------------------------------------------------------

void	Patcher::RemovePatchByIndex(DWORD idx)
{
	if (idx>=0 && idx <dwPatches) // in range of array
	{
		DWORD addr = Patches[idx].addr;
		DWORD len = Patches[idx].len;
		if (!IsBadWritePtr((PVOID)addr, len) )
		{
			memcpy( (PVOID)addr, Patches[idx].org, len);
		}

		// shift array left, but dont reallocate (small mem-leak) -->> this trick is used in 'AddPatch' 
		for ( DWORD i = idx+1; i < dwPatches; i++) 
			memcpy( (PVOID)&Patches[i-1], (PVOID)&Patches[i], sizeof(Patch));

		dwPatches--;
	}
}

// ------------------------------------------------------

VOID Patcher::CheckPatches() // removes unaccessible patches
{
	if ( dwPatches )
	{
		DWORD idx = 0;
		while ( idx < dwPatches )
		{
			if ( IsBadWritePtr((PVOID)Patches[idx].addr, Patches[idx].len) )
			{
				RemovePatchByIndex(idx);
			}
			else
				idx++;
		}
	}
}

// ------------------------------------------------------

VOID Patcher::RemoveAllPatches()
{
	if ( dwPatches )
	{
		for (DWORD i =dwPatches; i>0; i-- ) // backward
		{
			RemovePatchByIndex(i-1);
		}

		xfree(Patches);
		Patches = NULL;
	}
}

// ------------------------------------------------------

void Patcher::SafeSuspendAll(PSYSTEM_THREAD pThreads, DWORD dwThreads, DWORD dwBlockAddr, DWORD dwBlockSize)
{
	if ( pThreads == NULL ) return;
	DWORD dwCurrentTID = GetCurrentThreadId();
	
	for (unsigned int i = 0; i<dwThreads ; i++)
	{
		pThreads[i].ClientId.UniqueProcess = 0;	// use this field to handle the only threads are to resume
		if ( pThreads[i].ClientId.UniqueThread != dwCurrentTID )
		{
			if ( HANDLE hThrd = OpenThread(THREAD_SUSPEND_RESUME, 0, pThreads[i].ClientId.UniqueThread) )
			{
				while ( SuspendThread( hThrd ) != -1 )
				{
					pThreads[i].ClientId.UniqueProcess = (DWORD) hThrd;// dont care

					CONTEXT ctx;
					if ( GetThreadContext( hThrd, &ctx) )
					if ( ctx.Eip >= dwBlockAddr && ctx.Eip <= dwBlockSize )
					{
						ResumeThread( hThrd );
						Sleep(0); // switch
						continue; // try again
					}
					
					break;
				}
			}
		}
	}
}

// ------------------------------------------------------

void Patcher::WakeUP(PSYSTEM_THREAD pThreads, DWORD dwThreads)
{
	if ( pThreads )
	{
		for (unsigned int i = 0; i<dwThreads ; i++)
		{
			if ( pThreads[i].ClientId.UniqueProcess )
			{
				ResumeThread( (HANDLE) pThreads[i].ClientId.UniqueProcess );
				CloseHandle( (HANDLE) pThreads[i].ClientId.UniqueProcess );
			}
		}
	}
}

// ------------------------------------------------------

DWORD GetThreadsInfo (PSYSTEM_THREAD &pThreads, PSYSTEM_PROCESS_INFORMATION &pProcInfo)
{
	DWORD dwThreads = 0;
	pProcInfo = (PSYSTEM_PROCESS_INFORMATION) 
												QueryInformation( SystemProcessInformation );
	DWORD dwCurrentPID = GetCurrentProcessId();

	if ( pProcInfo )
	{
		for ( PSYSTEM_PROCESS_INFORMATION pCurrentProc = pProcInfo;;)
		{
			if (pCurrentProc->uUniqueProcessId == dwCurrentPID)
			{
				dwThreads = pCurrentProc->uThreadCount;
				pThreads = pCurrentProc->Threads;
			}

			if (pCurrentProc->uNext == 0)
				break;

			// find the address of the next process structure
			pCurrentProc = (PSYSTEM_PROCESS_INFORMATION)(((LPBYTE)pCurrentProc) + pCurrentProc->uNext);
		}
	}
	return dwThreads;
}

// ------------------------------------------------------

BOOL Patcher::MakePatch( PBYTE pAddr, PBYTE pData, DWORD dwDataSize )
{
	BOOL fRes = false;
	PSYSTEM_PROCESS_INFORMATION pProcInfo;
	PSYSTEM_THREAD pThreads;
	DWORD dwThreads;

	dwThreads = GetThreadsInfo(pThreads, pProcInfo);
	SafeSuspendAll( pThreads, dwThreads, (DWORD)pAddr, dwDataSize);

	DWORD dwOldp;
	if ( VirtualProtect( pAddr, dwDataSize, PAGE_EXECUTE_READWRITE, &dwOldp) && NewPatch( (DWORD)pAddr, dwDataSize ) )
	{
		memcpy( pAddr, pData, dwDataSize );
		fRes = true;
	}

	WakeUP( pThreads, dwThreads);
	xfree ( pProcInfo );
	return fRes;
}

// ------------------------------------------------------