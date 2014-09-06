#include "wowhacks.h"

/*
	wowhacks by Teq 10.2011 - 06.2014

	Передаю привет всем читателям ][, заглянувшим в мои исходники!
       Этот проект стал результатом кропотливого исследования и вложенных в него трудов, он многократно протестирован и полностью работоспособен
       Очень надеюсь, что вы подчерпнете отсюда что-то полезное для себя
*/

HANDLE hConIn, hConOut;

class ConsoleLogger
{
public:
	void OnLogMsg( void *caller, LogData *ld) // caller is undefined
	{
		DWORD written;
		WriteConsole( hConOut, ld->msg, strlen(ld->msg), &written, NULL ); 
	}
};

// ------------------------------------------------------

void ConsoleDbgCreate( )
{
	AllocConsole();
	SetConsoleTitle("Teq's wowhacks: debug console");
	hConIn = GetStdHandle( STD_INPUT_HANDLE );
	hConOut = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleMode(hConIn, 0);
}

// ------------------------------------------------------

void ConsoleProc()
{
	CHAR	byte;
	DWORD	read;
	while ( ReadConsole( hConIn, &byte, 1, &read, NULL) )
	{
		if ( byte == 'C' || byte == 'c' )
		{
			COORD coordScreen = { 0, 0 };
			FillConsoleOutputCharacter(hConOut,' ', 0xFFFFFFFF, coordScreen, &read);
			SetConsoleCursorPosition(hConOut, coordScreen);
		}
		else if ( byte == 'W' || byte == 'w' )
		{
			Scanner::TPattern WardenPattern ("\x56\x57\xFC\x8B\x54\x24\x14\x8B\x74\x24\x10\x8B\x44\x24\x0C\x8B\xCA\x8B\xF8\xC1\xE9\x02\x74\x02\xF3\xA5" \
								  "\xB1\x03\x23\xCA\x74\x02\xF3\xA4\x5F\x5E\xC3", "x37");
			DWORD WardenProc = (DWORD) Scanner::ScanMem( &WardenPattern );
			if ( WardenProc )
			{
				Logger::OutLog("Warden::Scan proc:0x%.8x\r\n", WardenProc);
			}
			else 
				Logger::OutLog("Warden::Scan proc not found\r\n");
		}
	}
}

// ------------------------------------------------------

void VA_hook_internal( DWORD dwCallAddr, DWORD dwMemBlock, DWORD dwSize)
{
	if ( dwMemBlock && dwSize > 0x2000 )
	{
		//if ( *(PDWORD)dwMemBlock == '2LLB' )
		Logger::OutLog("Allocated block:%.8X - %.8X, called from:%.8X\r\n", dwMemBlock, dwMemBlock+dwSize, dwCallAddr );
	}
}

void VP_hook_internal( DWORD dwCallAddr, DWORD dwMemBlock, DWORD dwSize, DWORD flNewProtect)
{
	if ( dwMemBlock && flNewProtect==PAGE_EXECUTE_READ )
	{
		MEMORY_BASIC_INFORMATION Mem;
		if  ( VirtualQuery( (PVOID) dwMemBlock, &Mem, sizeof (MEMORY_BASIC_INFORMATION)) )
		{
			if ( *(PDWORD)Mem.AllocationBase == '2LLB' )
			Logger::OutLog("Warden image found at:%.8X, code section:%.8X, called from:%.8X\r\n", Mem.AllocationBase, dwMemBlock, dwCallAddr);
			
		}
		
	}
}

__declspec(naked) void VA_hook( LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect )
{
	__asm 
	{
		pop		eax	// org func
		push	ebp
		mov		ebp, esp // to access parameters via names

		// call orig-func
		push	[flProtect]
		push	[flAllocationType]
		push	[dwSize]
		push	[lpAddress]
		call	VA_prologue

		pushad
		push	dwSize
		push	eax // mem block
		push	[ebp+4] // ret addr
		call	VA_hook_internal
		add esp, 0xC
		popad	

		leave
		ret 0x10
VA_prologue:
		push    ebp
		mov     ebp, esp
		jmp		eax // to VirtualAlloc
	}
}

__declspec(naked) void VP_hook( LPVOID lpAddress, SIZE_T dwSize, DWORD flProtect, DWORD pOldProtect )
{
	__asm 
	{
		pop		eax	// org func
		push	ebp
		mov		ebp, esp // to access parameters via names

		// call orig-func
		push	[pOldProtect]
		push	[flProtect]
		push	[dwSize]
		push	[lpAddress]
		call	VP_prologue

		pushad
		push	[flProtect]
		push	[dwSize]
		push	[lpAddress]
		push	[ebp+4] // ret addr
		call	VP_hook_internal
		add esp, 0x10
		popad	

		leave
		ret 0x10
VP_prologue:
		push    ebp
		mov     ebp, esp
		jmp		eax // to VirtualAlloc
	}
}

bool PatchVA()
{
	bool bRetval = false;
	PBYTE bCode = (PBYTE) "\xE8\x90\x90\x90\x90\x90"; // call rel32
	DWORD pProc = (DWORD) GetProcAddress( GetModuleHandleA( "KernelBase.DLL"), "VirtualAlloc" );
	
	*((PDWORD)(bCode+1)) = (DWORD)&VA_hook - ((DWORD)pProc+5);
	
	if ( Patcher::Instance()->MakePatch( (PBYTE)pProc, bCode, 5 ) ) 
	{
		Logger::OutLog( "VirtualAlloc patched at: %x\r\n", pProc );
		bRetval = true;
	}
	else Logger::OutLog( "VirtualAlloc patch failed\r\n" );
	return bRetval;
}

bool PatchVP()
{
	bool bRetval = false;
	PBYTE bCode = (PBYTE) "\xE8\x90\x90\x90\x90\x90"; // call rel32
	DWORD pProc = (DWORD) GetProcAddress( GetModuleHandleA( "KernelBase.DLL"), "VirtualProtect" );
	
	*((PDWORD)(bCode+1)) = (DWORD)&VP_hook - ((DWORD)pProc+5);
	
	if ( Patcher::Instance()->MakePatch( (PBYTE)pProc, bCode, 5 ) ) 
	{
		Logger::OutLog( "VirtualProtect patched at: %x\r\n", pProc );
		bRetval = true;
	}
	else Logger::OutLog( "VirtualProtect patch failed\r\n" );
	return bRetval;
}



// ------------------------------------------------------

void BuddyThread( LPVOID param )
{
	//PatchVP();
	ConsoleDbgCreate();

	// logger
	ConsoleLogger *log = new ConsoleLogger();
	Logger::LogMessageHandler = EventHandler<pLogData>(); // static-field initialize
	Logger::LogMessageHandler.Assign( EventHandler<pLogData>::CreateHandler<ConsoleLogger>(log, &ConsoleLogger::OnLogMsg) );

	HacksController * hc = new HacksController();
	hc->Run();
	
	ConsoleProc();
}

 // ------------------------------------------------------

void RemoteProc( )
{
	// prologue
	DWORD oeax, oeip, dwHistance;
	__asm mov oeax, eax
	__asm mov oeip, ecx	      // original eip in ecx
	__asm mov dwHistance, edx // new base

	//__asm int 3;
	ProcessImport(dwHistance, true);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) BuddyThread, NULL, 0, NULL );

	// epilogue
	__asm mov eax, oeax
	__asm jmp oeip
 }

// ------------------------------------------------------

void StartWow()
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	CreateProcessA( NULL, (PCHAR) "D:\\Games\\World of Warcraft\\wow.exe -noautolaunch64bit", NULL, NULL, false, CREATE_SUSPENDED, NULL, NULL, &si, &pi );
	Inject( NULL, pi.dwProcessId, (PTHREAD_START_ROUTINE)&RemoteProc, 0, pi.hThread); // using context-hijacking method
	ResumeThread( pi.hThread );
}

// ------------------------------------------------------

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	StartWow();
	return 0;
}
