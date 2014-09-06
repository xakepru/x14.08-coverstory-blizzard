#include "WardenLoaderHack.h"
#include "..\PB_lib\Warden.h"

bool WardenLoaderHack::MakePatches()
{
	if (PatchWardenLoader() != 0)return true;
	else return false;
}
bool WardenLoaderHack::RemovePatches()
{
	/*not implemented*/
	return true;
}

WardenLoaderHack::WardenLoaderHack(unsigned int ID): IHackBase(ID){ }

// ------------------------------------------------------

void WardenModulePatch( LPVOID dwMemBlock, DWORD dwSize)
{
#ifdef _DEBUG
			Logger::OutLog("WARDEN: code section at 0x%.8X (%x bytes)\r\n", dwMemBlock, dwSize);
#endif
	Warden::PatchWardenScan( dwMemBlock, dwSize);
}

__declspec(naked) void WardenLoader_hook( LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD pdwOldProtect ) // executes for every section (VirtualProtect)
{
	__asm 
	{
		push	ebp
		mov		ebp, esp
		pushad
	}
	if ( flNewProtect==PAGE_EXECUTE_READ ) // for code-section
		WardenModulePatch(lpAddress, dwSize);
	__asm
	{
		popad
		pop ebp
		jmp dword ptr[VirtualProtect]
	}
}

DWORD WardenLoaderHack::PatchWardenLoader()
{
	// LoadWardenModule thunk: (можно найти по референсу на "Warden.cpp"/"WardenLoader.cpp")
	// 51            |PUSH ECX                                ; /pOldProtect
	// FF76 08       |PUSH DWORD PTR DS:[ESI+8]               ; |NewProtect
	// 50            |PUSH EAX                                ; |Size
	// 53            |PUSH EBX                                ; |Address
	// FF15 BC329A01 |CALL DWORD PTR DS:[<&KERNEL32.VirtualPr ; \KERNEL32.VirtualProtect <<-- change( call dword ptr[vp_addr], call imm32 wa_hook )
	// F646 08 F0    |TEST BYTE PTR DS:[ESI+8],F0
	// 74 11         |JZ SHORT 016DBE9C
	// FF75 08       |PUSH DWORD PTR SS:[EBP+8]               ; /Size
	// 53            |PUSH EBX                                ; |Base
	// FF15 EC349A01 |CALL DWORD PTR DS:[<&KERNEL32.GetCurren ; |[KERNEL32.GetCurrentProcess
	// 50            |PUSH EAX                                ; |hProcess
	// FF15 B8329A01 |CALL DWORD PTR DS:[<&KERNEL32.FlushInst ; \KERNEL32.FlushInstructionCache
	// FF45 D4       |INC DWORD PTR SS:[EBP-2C]
	// 8B55 E4       |MOV EDX,DWORD PTR SS:[EBP-1C]
	// EB BA         \JMP SHORT 016DBE5E


	PBYTE	bCode = (PBYTE) "\xE8\x90\x90\x90\x90\x90\x90"; // call
	Scanner::TPattern Pattern( "\x51\xFF\x76\x08\x50\x53\xFF\x15\x00\x00\x00\x00\xF6\x46\x08\xF0\x74\x11\xFF\x75\x08\x53\xFF\x15\x00\x00\x00\x00\x50\xFF\x15",
		"x8?4x12?4x3");

	DWORD dwProc = (DWORD) Scanner::ScanMem( &Pattern );
	if ( dwProc )
	{
#ifdef _DEBUG
		Logger::OutLog("WARDEN: LoadWardenModule thunk:0x%.8X, patching...\r\n", dwProc );
#endif
		DWORD dwProcChangeOffset = dwProc+6;
		*((PDWORD)(bCode+1)) = (DWORD)&WardenLoader_hook - ((DWORD)dwProcChangeOffset+5);
		if ( Patcher::Instance()->MakePatch( (PBYTE)dwProcChangeOffset, bCode, 6 ) ) 
		{
#ifdef _DEBUG
			Logger::OutLog("WARDEN: LoadWardenModule patched at 0x%.8X (6 bytes)\r\n", dwProcChangeOffset);
#endif
			return dwProc;
		}
	}
	else
	{
#ifdef _DEBUG
		Logger::OutLog("WARDEN: LoadWardenModule thunk not found\r\n");
#endif
	}
	return NULL;

}

// ------------------------------------------------------

/*
CPU Disasm
Address   Hex dump          Command                                  Comments
016DBF4D  /$  55            PUSH EBP
016DBF4E  |.  8BEC          MOV EBP,ESP
016DBF50  |.  56            PUSH ESI
016DBF51  |.  33F6          XOR ESI,ESI
016DBF53  |.  56            PUSH ESI                                 ; /Arg4
016DBF54  |.  68 85000000   PUSH 85                                  ; |Arg3 = 85
016DBF59  |.  68 F43CA401   PUSH OFFSET 01A43CF4                     ; |Arg2 = ASCII "W32\WardenLoader.cpp"
016DBF5E  |.  6A 04         PUSH 4                                   ; |Arg1 = 4
016DBF60  |.  E8 6E01B8FF   CALL 0125C0D3                            ; \Wow.0125C0D3
016DBF65  |.  3BC6          CMP EAX,ESI
016DBF67  |.^ 74 09         JE SHORT 016DBF72
016DBF69  |.  8BC8          MOV ECX,EAX
016DBF6B  |.  E8 EDFCFFFF   CALL 016DBC5D                            ; [Wow.016DBC5D
016DBF70  |.  8BF0          MOV ESI,EAX
016DBF72  |>  FF75 0C       PUSH DWORD PTR SS:[EBP+0C]               ; /Arg2
016DBF75  |.  8B0E          MOV ECX,DWORD PTR DS:[ESI]               ; |
016DBF77  |.  FF75 08       PUSH DWORD PTR SS:[EBP+8]                ; |Arg1
016DBF7A  |.  E8 19FDFFFF   CALL 016DBC98                            ; \Wow.016DBC98 ; <<LOAD_MODULE>>
016DBF7F  |.  84C0          TEST AL,AL
016DBF81  |.^ 75 12         JNZ SHORT 016DBF95
016DBF83  |.  8BCE          MOV ECX,ESI
016DBF85  |.  E8 97FFFFFF   CALL 016DBF21                            ; [Wow.016DBF21
016DBF8A  |.  56            PUSH ESI                                 ; /Arg1
016DBF8B  |.  E8 7F509AFF   CALL 0108100F                            ; \Wow.0108100F
016DBF90  |.  59            POP ECX
016DBF91  |.  33C0          XOR EAX,EAX
016DBF93  |.^ EB 02         JMP SHORT 016DBF97
016DBF95  |>  8BC6          MOV EAX,ESI
016DBF97  |>  5E            POP ESI
016DBF98  |.  5D            POP EBP
016DBF99  \.  C3            RETN

*/