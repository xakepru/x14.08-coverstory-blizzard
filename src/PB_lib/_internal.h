#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501
#pragma warning( disable : 4005 ) // fuck it
#include <Windows.h>
#include <Ntsecapi.h>
#include "..\ntdll.h"
#include "Delegates.h"

#include "Patcher.h"
#include "Scanner.h"
#include "Logger.h"

PVOID extern QueryInformation ( SYSTEMINFOCLASS dwClass );
