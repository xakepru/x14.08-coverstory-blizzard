#ifndef __HACKSCTRL
#define __HACKSCTRL

#include "PB_lib\HackBase.h"
#include "PB_lib\_internal.h"

class HacksController
{
	class __list
	{
		pIHackBase * hacks; // array
		int dwHacks;
	public:
		__list()
		{
			hacks=NULL;
			dwHacks=0;
		}
		inline int Count() { return dwHacks; }
		void Add ( IHackBase *newHack )
		{
			pIHackBase *oldlist = hacks;
			hacks = new pIHackBase[dwHacks +1]; // MEMORY LEAK =)
			memcpy_s(hacks, sizeof(pIHackBase) * (dwHacks +1), oldlist, sizeof(pIHackBase) *dwHacks);
			hacks[dwHacks] = newHack;
			dwHacks++;
		}

		pIHackBase operator [] (int idx)
		{
			if (idx <dwHacks)
				return hacks[idx];
			else 
				return NULL;
		}

	} HacksList;

	void OnLogMsg( void *caller, LogData *ld);
public:
	HacksController(/* view? =) */);
	void Run();
};

#endif