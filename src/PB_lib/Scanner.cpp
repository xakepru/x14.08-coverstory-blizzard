#include "_internal.h"
#include "Scanner.h"

// ------------------------------------------------------

PVOID Scanner::FindPattern ( PVOID pMemBase, DWORD dwSize, pTPattern pPattern)
{
	DWORD dwRemain = dwSize;
	PBYTE pMem = (PBYTE)pMemBase, pMemCmp;
	BOOL fFound = false;

	while ( dwRemain >= pPattern->dwSequenceLen )
	{
		PBYTE pBytesToCompare = pPattern->pByteSequence;
		pMemCmp = pMem;
		
		for (DWORD i=0; pPattern->pPatternString[i] ; i++ )
		{
			bool skip = false;
			DWORD len = 0;
			switch (pPattern->pPatternString[i])
			{
				case '?':
					skip = true;
				case 'x':
					i++; // little optimization, may cause bug when pattern looks like "xx1"
					len = atoi( pPattern->pPatternString + i);
			}
			if (len)
			{
				if (!skip)
				fFound = !memcmp( pMemCmp, pBytesToCompare, len );

				pBytesToCompare += len;
				pMemCmp += len;
			}
			if (!fFound) break;
		}
		if (fFound) return pMem;

		pMem++;
		dwRemain--;
	}
	return 0;
}

// ------------------------------------------------------

PVOID Scanner::ScanMem( pTPattern pPattern )
{
     DWORD dwBase = 0x10000, dwProcessing;
     MEMORY_BASIC_INFORMATION Mem;
     PVOID pResult = 0;

	 do {
		if  ( dwProcessing = VirtualQuery( (PVOID) dwBase, &Mem, sizeof (MEMORY_BASIC_INFORMATION)) )
		{
			/*DbgOut("dwBase:%x, Alloc:%x, Base:%x, Size:%x, Aprotect:%x, Protect:%x, State:%x", 
				dwBase, (DWORD)Mem.AllocationBase, (DWORD) Mem.BaseAddress, Mem.RegionSize, 
				Mem.AllocationProtect, Mem.Protect, Mem.State);
*/
			// Only Executable&Readable memory ( 0x20 <= m <= 0x80 ) PAGE_GUARD
			if ( Mem.State == MEM_COMMIT && Mem.Protect<=0x80 && Mem.Protect>=0x20 ) 
				pResult = FindPattern( Mem.BaseAddress, Mem.RegionSize, pPattern );
			dwBase = Mem.RegionSize + (DWORD) Mem.BaseAddress ;
		}
	 } while ( dwProcessing && dwBase < 0x7C900000 && (!pResult || /* pattern found itself */ pResult==pPattern->pByteSequence) );

     return pResult;
}

// ------------------------------------------------------