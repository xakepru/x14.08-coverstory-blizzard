#ifndef __SCANNER
#define __SCANNER

namespace Scanner
{
	typedef class _Pattern 
	{
		public:
		PBYTE pByteSequence;
		DWORD dwSequenceLen;
		PCHAR pPatternString;

		_Pattern(PCHAR b, DWORD l, PCHAR p){
			pByteSequence = (PBYTE)b;
			dwSequenceLen = l;
			pPatternString = p;
		};
		_Pattern(PCHAR b, PCHAR p){ 
			pByteSequence = (PBYTE)b;
			dwSequenceLen = 0;
			pPatternString = p;

			// calc actual lenght
			for (DWORD i=0; pPatternString[i] ; i++ )
			{
				DWORD len = 0;
				switch (pPatternString[i])
				{
					case 'x':
					case '?':
						i++;
						len = atoi( pPatternString + i);
				}
				dwSequenceLen += len;
			}
		};
	} *pTPattern, TPattern;

	PVOID	ScanMem( pTPattern pszPattern );
	PVOID	FindPattern ( PVOID pMemBase, DWORD dwSize, pTPattern pPattern);
}

#endif
