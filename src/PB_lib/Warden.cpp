#include "Warden.h"
#include "Patcher.h"

// ------------------------------------------------------

 void __cdecl	Warden::ProcessScan( PBYTE pOutBuff, DWORD dwAddr, DWORD dwSize ) // zomg-hekk-code
{
#ifdef _DEBUG
	Logger::OutLog("WARDEN: Scan at: 0x%.8X (%d bytes)\r\n", dwAddr, dwSize );
#endif

	pPatcher p = Patcher::Instance();
	pPatch PatchList, pCurrentPatch = NULL;
	DWORD	dwPatches = p->GetAllPatches( PatchList );
	
	/* Forbid `warden-scanner` read patched data

	   .. - original bytes
	   ,, - patched	
	   `` - scanned

	   ....,,,,,,,,...................  (a)
	     ````````````````
	   ....,,,,,,,,...................  (b)
	      ```````
	   ....,,,,,,,,...................  (c)
	           ````
	   ....,,,,,,,,...................  (d)
	           ````````
	*/
	for ( unsigned int i=0; i< dwPatches; i++)
		// is here attempt to read a patched data?
		if ( (PatchList[i].addr - dwAddr < dwSize) /* (a) + (b) */ || 
			( dwAddr - PatchList[i].addr < PatchList[i].len ) /* (b) + (c) */ )
		{
			pCurrentPatch = &(PatchList[i]);
			break;
		}

	// copy
	if (!pCurrentPatch)
		memcpy(pOutBuff, (PVOID)dwAddr, dwSize);
	else
	{
		for ( unsigned int i=0; i< dwSize; i++)
		{
			unsigned int delta = dwAddr+i - pCurrentPatch->addr;
			byte* pCurrent;
			if( delta < pCurrentPatch->len ) // patched byte?
			{
				pCurrent = pCurrentPatch->org + delta;
			}
			else
				pCurrent = (PBYTE)(dwAddr+i);

			pOutBuff[i] = *pCurrent; 
		}
	
#ifdef _DEBUG
		Logger::OutLog("WARDEN: PREVENTED Scan at: 0x%.8X (%d bytes) with Patch at 0x%.8X\r\n", dwAddr, dwSize, pCurrentPatch->addr );
#endif
	}
}

// ------------------------------------------------------

bool Warden::PatchWardenScan( PVOID pMemBase, DWORD dwSize )
{	// WARDEN SCAN PROC
	// 56                     push    esi
	// 57                     push    edi
	// FC                     cld
	// 8B5424 14              mov     edx, dword ptr ss:[esp+14]
	// 8B7424 10              mov     esi, dword ptr ss:[esp+10]
	// 8B4424 0C              mov     eax, dword ptr ss:[esp+C]
	// 8BCA                   mov     ecx, edx
	// 8BF8                   mov     edi, eax
	// C1E9 02                shr     ecx, 2
	// 74 02                  je      short 
	// F3:A5                  rep     movs dword ptr es:[edi], dword ptr ds:[esi]
	// B1 03                  mov     cl, 3
	// 23CA                   and     ecx, edx
	// 74 02                  je      short 
	// F3:A4                  rep     movs byte ptr es:[edi], byte ptr ds:[esi]
	// 5F                     pop     edi
	// 5E                     pop     esi
	// C3                     ret
	bool	fResult = false;
	PBYTE	bCode = (PBYTE) "\x68\xAA\xAA\xAA\xAA\xC3"; // push 0xAAAAAAAA; retn
	Scanner::TPattern WardenPattern ("\x56\x57\xFC\x8B\x54\x24\x14\x8B\x74\x24\x10\x8B\x44\x24\x0C\x8B\xCA\x8B\xF8\xC1\xE9\x02\x74\x02\xF3\xA5" \
								  "\xB1\x03\x23\xCA\x74\x02\xF3\xA4\x5F\x5E\xC3", "x37");
	DWORD WardenProc;
	if (pMemBase)
		WardenProc = (DWORD) Scanner::FindPattern( pMemBase, dwSize, &WardenPattern );
	else
		WardenProc = (DWORD) Scanner::ScanMem( &WardenPattern );
	if ( WardenProc )
	{
#ifdef _DEBUG
		Logger::OutLog("Warden::Scan proc:0x%.8x, patching...\r\n", WardenProc);
#endif
		*((PDWORD)(bCode+1)) = (DWORD)&ProcessScan;
		if ( Patcher::Instance()->MakePatch( (PBYTE)WardenProc, bCode, 6 ) ) 
		{
#ifdef _DEBUG
		Logger::OutLog("Warden::Scan patched at 0x%.8x (6 bytes)\r\n", WardenProc);
#endif
			fResult = true;
		}
	}
	else
	{
#ifdef _DEBUG
		Logger::OutLog("Warden::Scan proc not found\r\n");
#endif
	}

	return fResult;
}

// ------------------------------------------------------