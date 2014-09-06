#include "RunScriptHack.h"


bool RunScriptHack::MakePatches()
{
	if (PatchRunScript()!=0) return true;
	else return false;
}
bool RunScriptHack::RemovePatches()
{
	/*do i really need this?*/
	return true;
}

RunScriptHack::RunScriptHack(unsigned int ID): IHackBase(ID)
{
}

// ------------------------------------------------------

DWORD RunScriptHack::PatchRunScript()
{
	/*
		33C9          XOR ECX,ECX
		56            PUSH ESI
		390D A8BA5F01 CMP DWORD PTR DS:[15FBAA8],ECX
		74 44         JE SHORT 01309A84
		83FA 22       CMP EDX,22
	*/

	PBYTE	bCode = (PBYTE) "\xEB"; // JMP SHORT
	Scanner::TPattern Pattern( "\x33\xC9\x56\x39\x0D\xFF\xFF\xFF\xFF\x74\x44\x83\xFA\x22", "x5?4x5");

	DWORD dwProc = (DWORD) Scanner::ScanMem( &Pattern );
	if ( dwProc )
	{
		DWORD dwProcChangeOffset = dwProc+9;

#ifdef _DEBUG
		Logger::OutLog("Script_RunScript proc:0x%.8X, patching...\r\n", dwProc );
#endif
		if ( Patcher::Instance()->MakePatch( (PBYTE)dwProcChangeOffset, bCode, 1 ) ) 
		{
#ifdef _DEBUG
			Logger::OutLog("Script_RunScript patched at 0x%.8X (1 bytes)\r\n", dwProcChangeOffset);
#endif
			return dwProcChangeOffset;
		}
	}
	else
	{
#ifdef _DEBUG
		Logger::OutLog("Script_RunScript proc not found\r\n");
#endif
	}
	return NULL;

}

// ------------------------------------------------------

/*
Поиск по референсу на строку "Usage: JoinBattlefield(id, joinAsGroup)"
00F51992      E8 93803B00   CALL 01309A2A
первый же вызов - функция проверки прав

CPU Disasm
Address   Hex dump          Command                                  Comments
00F5198D  /.  55            PUSH EBP
00F5198E  |.  8BEC          MOV EBP,ESP
00F51990      6A 14         PUSH 14
00F51992      E8 93803B00   CALL 01309A2A
00F51997  |.  59            POP ECX
00F51998  |.  85C0          TEST EAX,EAX
00F5199A  |.  74 4E         JZ SHORT 00F519EA
00F5199C  |.  56            PUSH ESI
00F5199D  |.  8B75 08       MOV ESI,DWORD PTR SS:[EBP+8]
00F519A0  |.  6A 01         PUSH 1                                   ; /Arg2 = 1
00F519A2  |.  56            PUSH ESI                                 ; |Arg1
00F519A3  |.  E8 C848BCFF   CALL 00B16270                            ; \wow.00B16270
00F519A8  |.  59            POP ECX
00F519A9  |.  59            POP ECX
00F519AA  |.  85C0          TEST EAX,EAX
00F519AC  |.  74 2E         JZ SHORT 00F519DC
00F519AE  |.  57            PUSH EDI
00F519AF  |.  6A 01         PUSH 1                                   ; /Arg2 = 1
00F519B1  |.  56            PUSH ESI                                 ; |Arg1
00F519B2  |.  E8 AF49BCFF   CALL 00B16366                            ; \wow.00B16366
00F519B7  |.  6A 00         PUSH 0                                   ; /Arg3 = 0
00F519B9  |.  6A 02         PUSH 2                                   ; |Arg2 = 2
00F519BB  |.  56            PUSH ESI                                 ; |Arg1
00F519BC  |.  8BF8          MOV EDI,EAX                              ; |
00F519BE  |.  E8 3E98B4FF   CALL 00A9B201                            ; \wow.00A9B201
00F519C3  |.  0FB6C8        MOVZX ECX,AL
00F519C6  |.  E8 DBF2FFFF   CALL 00F50CA6                            ; [wow.00F50CA6
00F519CB  |.  0FB6C0        MOVZX EAX,AL
00F519CE  |.  50            PUSH EAX
00F519CF  |.  51            PUSH ECX
00F519D0  |.  57            PUSH EDI
00F519D1  |.  E8 BBF1FFFF   CALL 00F50B91
00F519D6  |.  83C4 20       ADD ESP,20
00F519D9  |.  5F            POP EDI
00F519DA  |.  EB 0D         JMP SHORT 00F519E9
00F519DC  |>  68 88393E01   PUSH OFFSET 013E3988                     ; /Format = "Usage: JoinBattlefield(id, joinAsGroup)"
00F519E1  |.  56            PUSH ESI                                 ; |Arg1
00F519E2  |.  E8 2B5ABCFF   CALL 00B17412                            ; \wow.00B17412

*/