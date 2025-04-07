
void DeleteItems(UPrimalInventoryComponent* inv_comp, TArray<UPrimalItem*> for_delete)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	for (UPrimalItem* eDelItem : for_delete)
	{
		inv_comp->RemoveItem(&eDelItem->ItemIDField(), false, false, true, false);
	}
}



void RetrieveBag(AShooterCharacter* shooter_character)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	AShooterPlayerController* spc = static_cast<AShooterPlayerController*>(shooter_character->GetOwnerController());

	

	//if (!spc) return;

	//if (!spc->LastDeathPrimalCharacterField().Get())
	//{
	//	Log::GetLog()->info("LastDeathPrimalCharacterField is invalid!");
	//	return;
	//}

	//// Player Corpse still available
	//if (spc->LastDeathPrimalCharacterField().Get())
	//{
	//	Log::GetLog()->info("Corpse Exists!");
	//	FindPlayerCorpse(spc);
	//}
	//// Item Cache Bag 
	//else
	//{
	//	Log::GetLog()->info("Corpse Doesnt Exists!");
	//	FindItemCacheBag(shooter_character);
	//}


}


void CheckCallback(AShooterPlayerController* pc, FString* param, int, int)
{

	if (!pc)
	{
		Log::GetLog()->info("AShooterPlayerController is invalid!");
		return;
	}

	if (!pc->LastDeathPrimalCharacterField())
	{
		Log::GetLog()->info("LastDeathPrimalCharacterField is invalid!");
		return;
	}
	else
	{
		Log::GetLog()->info("LastDeathPrimalCharacterField is valid!");
	}
}


void KillMeCallBack(AShooterPlayerController* player_controller, FString* param, int, int)
{
	player_controller->GetPlayerCharacter()->Suicide();
	player_controller->CopyCoordsToClipboard();
}


void TeleportCallBack(AShooterPlayerController* player_controller, FString* param, int, int)
{

	ACharacter* character = static_cast<ACharacter*>(player_controller->CharacterField().Get());

	UE::Math::TVector<double> Dest = UE::Math::TVector<double>(-81866.1, 168309, -16171);
	FRotator Rot = UE::Math::TRotator<double>(0, 0, 0);
	character->TeleportTo(&Dest, &Rot, false, false);
	//UPrimalGlobals::SimpleTeleportTo(player_controller->LastControlledPlayerCharacterField().Get()->OwnerField(), Dest, Rot);
}
