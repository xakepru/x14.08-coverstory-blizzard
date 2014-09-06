#include "HackBase.h"

IHackBase::IHackBase(unsigned int ID)
{
	id = ID;
	state = notinstalled;
}

void IHackBase::pulseState()
{
	HackStateChangedHandler( this, id );
}

unsigned int IHackBase::GetID()
{
	return id;
}

void IHackBase::Install()
{
	if ( state!=installed) 
	{
		if (MakePatches())
			state = installed;
		else
			state = error;
		pulseState();
	}
}

void IHackBase::Uninstall()
{
	if (state==installed)
	{
		RemovePatches();
	}
	state = notinstalled;
	pulseState();
}
