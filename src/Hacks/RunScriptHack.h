#ifndef __RUNSCRIPTHACK
#define __RUNSCRIPTHACK

#include "..\PB_lib\HackBase.h"

class RunScriptHack: public IHackBase
{
	DWORD PatchRunScript();
	bool MakePatches();
	bool RemovePatches();
	
public:
	RunScriptHack(unsigned int ID);
};

#endif