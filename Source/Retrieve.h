
void FindItemCacheBag(AShooterPlayerController* player_controller, PlayerDeathData pdd)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	if (!player_controller) return;

	AShooterCharacter* shooter_character = player_controller->GetPlayerCharacter();

	if (!shooter_character) return;

	UPrimalInventoryComponent* playerInvComp = shooter_character->MyInventoryComponentField();

	if (!playerInvComp) return;

	if (!DeathBagRetriever::DeathCacheClass)
		DeathBagRetriever::DeathCacheClass = UVictoryCore::BPLoadClass("Blueprint'/Game/PrimalEarth/Structures/DeathItemCache_PlayerDeath.DeathItemCache_PlayerDeath'");

	TArray<AActor*> out_bags;
	UVictoryCore::ServerOctreeOverlapActors(&out_bags, player_controller->GetWorld(), &pdd.Location, 500, EServerOctreeGroup::Type::STRUCTURES, true);

	for (AActor* actor : out_bags)
	{
		if (!actor) continue;

		if (!actor->IsA(DeathBagRetriever::DeathCacheClass)) continue;

		APrimalStructureItemContainer* cacheBag = static_cast<APrimalStructureItemContainer*>(actor);

		if (!cacheBag) continue;

		if (!cacheBag->DescriptiveNameField().Equals("Item Cache")) continue;

		if (cacheBag->DeathCacheCharacterIDField() == 0) continue;

		if (cacheBag->DeathCacheCharacterIDField() != pdd.PlayerID) continue;

		UPrimalInventoryComponent* cacheBagInvComp = cacheBag->MyInventoryComponentField();

		if (!cacheBagInvComp) continue;

		if (cacheBagInvComp->InventoryItemsField().Num() <= 0) continue;

		TArray<UPrimalItem*> cacheBagInvItems = cacheBagInvComp->InventoryItemsField();

		for (UPrimalItem* item : cacheBagInvItems)
		{
			if (item->bIsEngram().Get()) continue;
			if (item->IsItemSkin(true)) continue;

			// TODO expose this in config
			// Resources
			if (item->MyItemTypeField().GetIntValue() == 5) continue;

			// Artifact
			if (item->MyItemTypeField().GetIntValue() == 8) continue;

			// Fertile egg
			if (item->bIsEgg().Get() && item->UsesDurability()) continue;

			cacheBagInvComp->RemoveItem(&item->ItemIDField(), false, true, true, false);

			// finaly transfer
			bool addToSlot = item->SlotIndexField() > -1;
			item->AddToInventory(playerInvComp, false, addToSlot, &item->ItemIDField(), true, true, false, false, false);
		}
	}

	// Remove Corpse
	//DeathBagRetriever::playerCorpses.RemoveSingle(pdd);
}


void FindPlayerCorpse(AShooterPlayerController* player_controller)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);
	if (!player_controller)
	{
		Log::GetLog()->info("AShooterPlayerController is invalid!");
		return;
	}

	UPrimalInventoryComponent* newInvComp = player_controller->GetPlayerInventoryComponent();

	if (!newInvComp)
	{
		Log::GetLog()->error("newInvComp is invalid!");
		return;
	}

	TArray<PlayerDeathData> pdd = DeathBagRetriever::playerCorpses.FilterByPredicate([&](const PlayerDeathData& _pdd)
		{
			return _pdd.EosID.Equals(player_controller->GetEOSId());
		});

	for (PlayerDeathData _pdd : pdd)
	{
		AShooterCharacter* corpse = _pdd.Corpse;

		UPrimalInventoryComponent* corpseInvComp = corpse->MyInventoryComponentField();

		if (!corpseInvComp)
		{
			// Find cache bag instead
			FindItemCacheBag(player_controller, _pdd);

			Log::GetLog()->error("corpseInvComp is invalid!");
			continue;
		}

		// Handle Equipped items
		TArray<UPrimalItem*> equippedItems = corpseInvComp->EquippedItemsField();

		Log::GetLog()->info("equippedItems: {}", equippedItems.Num());

		for (UPrimalItem* eItem : equippedItems)
		{
			if (eItem->bIsEngram().Get()) continue;
			if (eItem->IsItemSkin(true)) continue;

			bool isEquipped;
			if (!eItem->IsBroken())
			{
				isEquipped = true;
			}

			corpseInvComp->RemoveItem(&eItem->ItemIDField(), false, false, true, false);

			bool addToSlot = eItem->SlotIndexField() > -1;
			eItem->AddToInventory(newInvComp, isEquipped, addToSlot, &eItem->ItemIDField(), false, false, false, false, false);

			Log::GetLog()->info("item eq transferred: {}", eItem->DescriptiveNameBaseField().ToString());
		}

		// Handle Inventory items
		TArray<UPrimalItem*> invItems = corpseInvComp->InventoryItemsField();

		Log::GetLog()->info("invItems: {}", invItems.Num());

		for (UPrimalItem* iItem : invItems)
		{
			if (iItem->bIsEngram().Get()) continue;
			if (iItem->IsItemSkin(true)) continue;

			// TODO expose this in config
			// Resources
			if (iItem->MyItemTypeField().GetIntValue() == 5) continue;

			// Artifact
			if (iItem->MyItemTypeField().GetIntValue() == 8) continue;

			// Fertile egg
			if (iItem->bIsEgg().Get() && iItem->UsesDurability()) continue;

			corpseInvComp->RemoveItem(&iItem->ItemIDField(), false, true, true, false);

			// finaly transfer
			bool addToSlot = iItem->SlotIndexField() > -1;
			iItem->AddToInventory(newInvComp, false, addToSlot, &iItem->ItemIDField(), true, true, false, false, false);

			Log::GetLog()->info("item inv transferred: {}", iItem->DescriptiveNameBaseField().ToString());
		}

		// Remove Corpse
		//DeathBagRetriever::playerCorpses.RemoveSingle(_pdd);
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
