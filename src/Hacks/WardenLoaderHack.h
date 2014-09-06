#ifndef __WARDENLOADERHACK
#define __WARDENLOADERHACK

#include "..\PB_lib\HackBase.h"

class WardenLoaderHack: public IHackBase
{
	DWORD PatchWardenLoader();
	bool MakePatches();
	bool RemovePatches();
	
public:
	WardenLoaderHack(unsigned int ID);
};

#endif