#include "HacksController.h"
#include "Hacks\WardenLoaderHack.h"
#include "Hacks\RunScriptHack.h"
HacksController::HacksController()
{
	HacksList.Add( new WardenLoaderHack(-1) );
	HacksList.Add( new RunScriptHack(0) );
	//HacksList.Add( new ForceInsecureHack(1) );
	//HacksList.Add( new GlobalCooldownHack(2) );
	//HacksList.Add( new LoadAddonHack(3) );
}

void HacksController::Run()
{
	for (int i=0; i<HacksList.Count(); i++ )
		HacksList[i]->Install();
}