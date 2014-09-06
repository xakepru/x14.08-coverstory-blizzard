#ifndef __HACKBASE
#define __HACKBASE

#include "_internal.h"

typedef enum {installed, notinstalled, error} enHackState;

typedef class IHackBase abstract
{
protected:
	unsigned int id;
	enHackState state;
	virtual bool MakePatches() =0;
	virtual bool RemovePatches() =0;

	void pulseState();

public:
	IHackBase(unsigned int ID);
	EventHandler<unsigned int> HackStateChangedHandler; // StateChanged(this, this.ID)

	unsigned int GetID();
	enHackState GetState() {return state;}
	void Install(); // patches
	void Uninstall();
} *pIHackBase;

#endif