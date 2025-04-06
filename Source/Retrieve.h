
void DeleteItems(UPrimalInventoryComponent* inv_comp, TArray<UPrimalItem*> for_delete)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	for (UPrimalItem* eDelItem : for_delete)
	{
		inv_comp->RemoveItem(&eDelItem->ItemIDField(), false, false, true, false);
	}
}

void FindPlayerCorpse(AShooterPlayerController* player_controller)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	UPrimalInventoryComponent* corpseInvComp = player_controller->LastDeathPrimalCharacterField().Get()->MyInventoryComponentField();

	if (!corpseInvComp) return;

	UPrimalInventoryComponent* newInvComp = player_controller->GetPlayerInventoryComponent();

	if (!newInvComp) return;

	// Handle Equipped items
	TArray<UPrimalItem*> equippedItems = corpseInvComp->EquippedItemsField();
	TArray<UPrimalItem*> forDelete;

	for (UPrimalItem* eItem : equippedItems)
	{
		bool isEquipped;
		if (!eItem->IsBroken())
		{
			isEquipped = true;
		}

		forDelete.Add(eItem);

		eItem->AddToInventory(newInvComp, isEquipped, true, &eItem->ItemIDField(), false ,false, false, false, false);
	}

	// Handle Inventory items
	TArray<UPrimalItem*> invItems = corpseInvComp->InventoryItemsField();

	for (UPrimalItem* iItem : invItems)
	{
		if (iItem->bIsEngram().Get()) continue;

		// TODO expose this in config
		// Resources
		if (iItem->MyItemTypeField().GetIntValue() == 5) continue;

		// Artifact
		if (iItem->MyItemTypeField().GetIntValue() == 8) continue;

		// Fertile egg
		if (iItem->bIsEgg().Get() && iItem->UsesDurability()) continue;

		// remove items
		forDelete.Add(iItem);

		// finaly transfer
		bool addToSlot = iItem->SlotIndexField() > -1;
		iItem->AddToInventory(newInvComp, false, addToSlot, &iItem->ItemIDField(), true, true, false, false, false);
	}

	DeleteItems(corpseInvComp, forDelete);
}

void FindItemCacheBag(AShooterCharacter* shooter_character)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	UPrimalInventoryComponent* playerInvComp = shooter_character->MyInventoryComponentField();

	if (!playerInvComp) return;

	if(!DeathBagRetriever::DeathCacheClass)
		DeathBagRetriever::DeathCacheClass = UVictoryCore::BPLoadClass("Blueprint'/Game/PrimalEarth/Structures/DeathItemCache_PlayerDeath.DeathItemCache_PlayerDeath'");
	TArray<AActor*> AllCacheBag;
	UGameplayStatics::GetAllActorsOfClass(AsaApi::GetApiUtils().GetWorld(), DeathBagRetriever::DeathCacheClass, &AllCacheBag);

	for (AActor* cacheBag : AllCacheBag)
	{
		if (!cacheBag) continue;

		if (!cacheBag->IsA(DeathBagRetriever::DeathCacheClass)) continue;

		APrimalStructureItemContainer* cacheBagContainer = static_cast<APrimalStructureItemContainer*>(cacheBag);

		if (!cacheBagContainer) continue;

		if (!cacheBagContainer->DescriptiveNameField().Equals("Item Cache")) continue;

		if (cacheBagContainer->DeathCacheCharacterIDField() == 0) continue;

		if (cacheBagContainer->DeathCacheCharacterIDField() != shooter_character->GetLinkedPlayerDataID()) continue;

		UPrimalInventoryComponent* cacheBagInvComp = cacheBagContainer->MyInventoryComponentField();

		if (!cacheBagInvComp) continue;

		TArray<UPrimalItem*> forDelete;

		if (cacheBagInvComp->InventoryItemsField().Num() <= 0) continue;

		TArray<UPrimalItem*> cacheBagInvItems = cacheBagInvComp->InventoryItemsField();

		for (UPrimalItem* item : cacheBagInvItems)
		{
			if (item->bIsEngram().Get()) continue;

			// TODO expose this in config
			// Resources
			if (item->MyItemTypeField().GetIntValue() == 5) continue;

			// Artifact
			if (item->MyItemTypeField().GetIntValue() == 8) continue;

			// Fertile egg
			if (item->bIsEgg().Get() && item->UsesDurability()) continue;

			// remove items
			forDelete.Add(item);

			// finaly transfer
			bool addToSlot = item->SlotIndexField() > -1;
			item->AddToInventory(playerInvComp, false, addToSlot, &item->ItemIDField(), true, true, false, false, false);
		}

		// Handle Delete Items
		DeleteItems(cacheBagInvComp, forDelete);
	}
}

void RetrieveBag(AShooterCharacter* shooter_character)
{
	Log::GetLog()->info("{} called!", __FUNCTION__);

	AShooterPlayerController* spc = static_cast<AShooterPlayerController*>(shooter_character->GetOwnerController());

	if (!spc->LastDeathPrimalCharacterField())
	{
		Log::GetLog()->info("LastDeathPrimalCharacterField is invalid!");
		return;
	}

	// Player Corpse still available
	if (spc->LastDeathPrimalCharacterField().Get())
	{
		Log::GetLog()->info("Corpse Exists!");
		FindPlayerCorpse(spc);
	}
	// Item Cache Bag 
	else
	{
		Log::GetLog()->info("Corpse Doesnt Exists!");
		FindItemCacheBag(shooter_character);
	}
}