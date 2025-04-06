#include "API/ARK/Ark.h"

#include "DeathBagRetriever.h"

#include "Utils.h"

#include "Retrieve.h"

//#include "RepairItems.h"

#include "Hooks.h"

#include "Timers.h"

//#include "Commands.h"

#include "Reload.h"


#pragma comment(lib, "AsaApi.lib")

/*
Player Automatic retrieval
	- When player dies it all/specified items will be transfer to chosen storage
		- slash command to a storage and save to database
	- When player dies it will equip and retrieve the items in inventory

Player Manual retrieve
	- slash commands

Capture player initial items before dying

Another (way less cheating)
- When player do slash command it will capture initial inventory items and save all the list in the db (capture id)
- perform slash retrieve
*/



void OnServerReady()
{
	Log::GetLog()->info("DeathBagRetriever Initialized");

	ReadConfig();
	//LoadDatabase();
	//AddOrRemoveCommands();
	AddReloadCommands();
	SetTimers();
	SetHooks();
}

DECLARE_HOOK(AShooterGameMode_BeginPlay, void, AShooterGameMode*);
void Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this)
{
	AShooterGameMode_BeginPlay_original(_this);

	OnServerReady();
}

extern "C" __declspec(dllexport) void Plugin_Init()
{
	Log::Get().Init(PROJECT_NAME);

	AsaApi::GetHooks().SetHook("AShooterGameMode.BeginPlay()", Hook_AShooterGameMode_BeginPlay, &AShooterGameMode_BeginPlay_original);

	if (AsaApi::GetApiUtils().GetStatus() == AsaApi::ServerStatus::Ready)
		OnServerReady();
}

extern "C" __declspec(dllexport) void Plugin_Unload()
{
	AsaApi::GetHooks().DisableHook("AShooterGameMode.BeginPlay()", Hook_AShooterGameMode_BeginPlay);

	//AddOrRemoveCommands(false);
	AddReloadCommands(false);
	SetTimers(false);
	SetHooks(false);
}