#define _WIN32_WINNT 0x0501
#define _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4005 ) // fuck it
#include <Windows.h>
#include <stdio.h>
#include <Ntsecapi.h>
#include "ntdll.h"

#include "_Debug_mod/Debug_mod.h"
#include "_Base_mod/Base_mod.h"
#include "_Inject_mod/Inject_mod.h"

#include "PB_lib\Logger.h"
#include "PB_lib\Warden.h"
#include "HacksController.h"