

void AddOrRemoveCommands(bool addCmd = true)
{
	const FString CheckDebug = DeathBagRetriever::config["Commands"]["CheckCMD"].get<std::string>().c_str();
	if (!CheckDebug.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(CheckDebug, &CheckCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(CheckDebug);
		}
	}

	const FString KillMe = DeathBagRetriever::config["Commands"]["KillMeCMD"].get<std::string>().c_str();
	if (!KillMe.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(KillMe, &KillMeCallBack);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(KillMe);
		}
	}

	const FString Teleport = DeathBagRetriever::config["Commands"]["KillMeCMD"].get<std::string>().c_str();
	if (!Teleport.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(Teleport, &TeleportCallBack);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(Teleport);
		}
	}

	/*const FString RepairItems = DeathBagRetriever::config["Commands"]["RepairItemCMD"].get<std::string>().c_str();
	if (!RepairItems.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(RepairItems, &RepairItemsCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(RepairItems);
		}
	}

	const FString DeletePlayer = DeathBagRetriever::config["Commands"]["DeletePlayerCMD"].get<std::string>().c_str();
	if (!DeletePlayer.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(DeletePlayer, &DeletePlayerCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(DeletePlayer);
		}
	}*/
}