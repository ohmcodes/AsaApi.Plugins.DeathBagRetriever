

DECLARE_HOOK(HandleRespawned_Implementation, void, AShooterPlayerController*, APawn*, bool);

void Hook_HandleRespawned_Implementation(AShooterPlayerController* player_controller, APawn* NewPawn, bool IsFirstSpawn)
{
	HandleRespawned_Implementation_original(player_controller, NewPawn, IsFirstSpawn);

	FindPlayerCorpse(player_controller);
}


DECLARE_HOOK(AShooterCharacter_Die, bool, AShooterCharacter*, float, FDamageEvent*, AController*, AActor*);

bool Hook_AShooterCharacter_Die(AShooterCharacter* shooter_character, float KillingDamage, FDamageEvent* DamageEvent, AController* Killer, AActor* DamageCauser)
{
	PlayerDeathData playerDeathData = ConstructPlayerDeathData(shooter_character);

	DeathBagRetriever::playerCorpses.Add(playerDeathData);

	return AShooterCharacter_Die_original(shooter_character, KillingDamage, DamageEvent, Killer, DamageCauser);
}

void SetHooks(bool addHooks = true)
{
	if (addHooks)
	{
		AsaApi::GetHooks().SetHook("AShooterPlayerController.HandleRespawned_Implementation(APawn*,bool)", &Hook_HandleRespawned_Implementation, &HandleRespawned_Implementation_original);

		AsaApi::GetHooks().SetHook("AShooterCharacter.Die(float,FDamageEvent&,AController*,AActor*)", &Hook_AShooterCharacter_Die, &AShooterCharacter_Die_original);
	}
	else
	{
		AsaApi::GetHooks().DisableHook("AShooterPlayerController.HandleRespawned_Implementation(APawn*,bool)", &Hook_HandleRespawned_Implementation);

		AsaApi::GetHooks().DisableHook("AShooterCharacter.Die(float,FDamageEvent&,AController*,AActor*)", &Hook_AShooterCharacter_Die);
	}
}