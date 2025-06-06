
#include <fstream>

PlayerDeathData ConstructPlayerDeathData(AShooterCharacter* shooter_character)
{
	FString eosID;
	shooter_character->GetUniqueNetIdAsString(&eosID);

	PlayerDeathData pdd;

	pdd.EosID = eosID;
	pdd.PlayerID = shooter_character->GetLinkedPlayerDataID();
	pdd.Corpse = shooter_character;
	pdd.Location = shooter_character->GetLocation();

	return pdd;
}



static bool startsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

void FetchMessageFromDiscordCallback(bool success, std::string results)
{
	//Log::GetLog()->warn("Function: {}", __FUNCTION__);

	if (success)
	{
		if(results.empty()) return;

		try
		{
			nlohmann::json resObj = nlohmann::json::parse(results)[0];

			if (resObj.is_null())
			{
				Log::GetLog()->warn("resObj is null");
				return;
			}

			auto globalName = resObj["author"]["global_name"];

			// if not sent by bot
			if (resObj.contains("bot") && globalName.is_null())
			{
				Log::GetLog()->warn("the sender is bot");
				return;
			}

			std::string msg = resObj["content"].get<std::string>();
			
			if (!startsWith(msg, "!"))
			{
				Log::GetLog()->warn("message not startswith !");
				return;
			}

			if (DeathBagRetriever::lastMessageID == resObj["id"].get<std::string>()) return;
			
			std::string sender = fmt::format("Discord: {}", globalName.get<std::string>());

			AsaApi::GetApiUtils().SendChatMessageToAll(FString(sender), msg.c_str());

			DeathBagRetriever::lastMessageID = resObj["id"].get<std::string>();
		}
		catch (std::exception& error)
		{
			Log::GetLog()->error("Error parsing JSON results. Error: {}",error.what());
		}
	}
	else
	{
		Log::GetLog()->warn("Failed to fetch messages. success: {}", success);
	}
}

void FetchMessageFromDiscord()
{
	//Log::GetLog()->warn("Function: {}", __FUNCTION__);

	std::string botToken = DeathBagRetriever::config["DiscordBot"].value("BotToken","");

	std::string channelID = DeathBagRetriever::config["DiscordBot"].value("ChannelID", "");

	std::string apiURL = FString::Format("https://discord.com/api/v10/channels/{}/messages?limit=1", channelID).ToString();

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: DeathBagRetriever/1.0",
		"Connection: keep-alive",
		"Accept: */*",
		"Content-Length: 0",
		"Authorization: Bot " + botToken
	};

	try
	{
		bool req = DeathBagRetriever::req.CreateGetRequest(apiURL, FetchMessageFromDiscordCallback, headers);

		if (!req)
			Log::GetLog()->error("Failed to perform Get request. req = {}", req);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to perform Get request. Error: {}", error.what());
	}
}

void SendMessageToDiscordCallback(bool success, std::string results, std::unordered_map<std::string, std::string> responseHeaders)
{
	if (!success)
	{
		Log::GetLog()->error("Failed to send Post request. {} {} {}", __FUNCTION__, success, results);
	}
	else
	{
		Log::GetLog()->info("Success. {} {} {}", __FUNCTION__, success, results);
	}
}

void SendMessageToDiscord(std::string msg)
{

	Log::GetLog()->warn("Function: {}", __FUNCTION__);

	
	std::string webhook = DeathBagRetriever::config["DiscordBot"].value("Webhook", "");
	std::string botImgUrl = DeathBagRetriever::config["DiscordBot"].value("BotImageURL", "");

	if (webhook == "" || webhook.empty()) return;

	FString msgFormat = L"{{\"content\":\"{}\",\"username\":\"{}\",\"avatar_url\":\"{}\"}}";

	FString msgOutput = FString::Format(*msgFormat, msg, "ArkBot", botImgUrl);

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: DeathBagRetriever/1.0",
		"Connection: keep-alive",
		"Accept: */*"
	};

	try
	{
		bool req = DeathBagRetriever::req.CreatePostRequest(webhook, SendMessageToDiscordCallback, msgOutput.ToStringUTF8(), "application/json", headers);

		if(!req)
			Log::GetLog()->error("Failed to send Post request. req = {}", req);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to send Post request. Error: {}", error.what());
	}
}

bool Points(FString eos_id, int cost, bool check_points = false)
{
	if (cost == -1)
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is -1");
		}
		return false;
	}

	if (cost == 0)
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is 0");
		}

		return true;
	}

	nlohmann::json config = DeathBagRetriever::config["PointsDBSettings"];

	if (config.value("Enabled", false) == false)
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Points system is disabled");
		}

		return true;
	}

	std::string tablename = config.value("TableName", "ArkShopPlayers");
	std::string unique_id = config.value("UniqueIDField", "EosId");
	std::string points_field = config.value("PointsField", "Points");
	std::string totalspent_field = config.value("TotalSpentField", "TotalSpent");

	if (tablename.empty() || unique_id.empty() || points_field.empty())
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("DB Fields are empty");
		}
		return false;
	}

	std::string escaped_eos_id = DeathBagRetriever::pointsDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}'", tablename, unique_id, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;

	if (!DeathBagRetriever::pointsDB->read(query, results))
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Error reading points db");
		}

		return false;
	}

	if (results.size() <= 0)
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("No record found");
		}
		return false;
	}

	int points = std::atoi(results[0].at(points_field).c_str());

	if (check_points)
	{
		if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Player got {} points", points);
		}

		if (points >= cost) return true;
	}
	else
	{
		int amount = points - cost;

		std::vector<std::pair<std::string, std::string>> data;

		data.push_back({ points_field, std::to_string(amount) });

		if (totalspent_field != "")
		{
			int total_spent = std::atoi(results[0].at(totalspent_field).c_str());
			std::string total_ts = std::to_string(total_spent + cost);

			data.push_back({totalspent_field, total_ts});
		}

		std::string condition = fmt::format("{}='{}'", unique_id, escaped_eos_id);

		if (DeathBagRetriever::pointsDB->update(tablename, data, condition))
		{
			if (DeathBagRetriever::config["Debug"].value("Points", false) == true)
			{
				Log::GetLog()->info("{} Points DB updated", amount);
			}

			return true;
		}
	}

	return false;
}

nlohmann::json GetCommandString(const std::string permission, const std::string command)
{
	if (permission.empty()) return {};
	if (command.empty()) return {};

	nlohmann::json config_obj = DeathBagRetriever::config["PermissionGroups"];
	nlohmann::json perm_obj = config_obj[permission];
	nlohmann::json command_obj = perm_obj["Commands"];
	nlohmann::json setting_obj = command_obj[command];

	return setting_obj;
}

TArray<FString> GetPlayerPermissions(FString eos_id)
{
	TArray<FString> PlayerPerms = { "Default" };

	std::string escaped_eos_id = DeathBagRetriever::permissionsDB->escapeString(eos_id.ToString());

	std::string tablename = DeathBagRetriever::config["PermissionsDBSettings"].value("TableName", "Players");

	std::string condition = DeathBagRetriever::config["PermissionsDBSettings"].value("UniqueIDField", "EOS_Id");

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}';", tablename, condition, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;
	if (!DeathBagRetriever::permissionsDB->read(query, results))
	{
		if (DeathBagRetriever::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->warn("Error reading permissions DB");
		}

		return PlayerPerms;
	}

	if (results.size() <= 0) return PlayerPerms;

	std::string permsfield = DeathBagRetriever::config["PermissionsDBSettings"].value("PermissionGroupField","PermissionGroups");

	FString playerperms = FString(results[0].at(permsfield));

	if (DeathBagRetriever::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("current player perms {}", playerperms.ToString());
	}

	playerperms.ParseIntoArray(PlayerPerms, L",", true);

	return PlayerPerms;
}

FString GetPriorPermByEOSID(FString eos_id)
{
	TArray<FString> player_groups = GetPlayerPermissions(eos_id);

	const nlohmann::json permGroups = DeathBagRetriever::config["PermissionGroups"];

	std::string defaultGroup = "Default";
	int minPriority = INT_MAX;
	nlohmann::json result;
	FString selectedPerm = "Default";

	for (const FString& param : player_groups)
	{
		if (permGroups.contains(param.ToString()))
		{
			int priority = static_cast<int>(permGroups[param.ToString()]["Priority"]);
			if (priority < minPriority)
			{
				minPriority = priority;
				result = permGroups[param.ToString()];
				selectedPerm = param;
			}
		}
	}

	if (result.is_null() && permGroups.contains(defaultGroup))
	{
		if(!permGroups[defaultGroup].is_null())
			result = permGroups[defaultGroup];

		result = {};
	}

	if (DeathBagRetriever::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("Selected Permission {}", selectedPerm.ToString());
	}

	return selectedPerm;
}

bool AddPlayer(FString eosID, int playerID, FString playerName)
{
	std::vector<std::pair<std::string, std::string>> data = {
		{"EosId", eosID.ToString()},
		{"PlayerId", std::to_string(playerID)},
		{"PlayerName", playerName.ToString()}
	};

	return DeathBagRetriever::pluginTemplateDB->create(DeathBagRetriever::config["PluginDBSettings"]["TableName"].get<std::string>(), data);
}

bool ReadPlayer(FString eosID)
{
	std::string escaped_id = DeathBagRetriever::pluginTemplateDB->escapeString(eosID.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", DeathBagRetriever::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_id);

	std::vector<std::map<std::string, std::string>> results;
	DeathBagRetriever::pluginTemplateDB->read(query, results);

	return results.size() <= 0 ? false : true;
}

bool UpdatePlayer(FString eosID, FString playerName)
{
	std::string unique_id = "EosId";

	std::string escaped_id = DeathBagRetriever::pluginTemplateDB->escapeString(eosID.ToString());

	std::vector<std::pair<std::string, std::string>> data = {
		{"PlayerName", playerName.ToString() + "123"}
	};

	std::string condition = fmt::format("{}='{}'", unique_id, escaped_id);

	return DeathBagRetriever::pluginTemplateDB->update(DeathBagRetriever::config["PluginDBSettings"]["TableName"].get<std::string>(), data, condition);
}

bool DeletePlayer(FString eosID)
{
	std::string escaped_id = DeathBagRetriever::pluginTemplateDB->escapeString(eosID.ToString());

	std::string condition = fmt::format("EosId='{}'", escaped_id);

	return DeathBagRetriever::pluginTemplateDB->deleteRow(DeathBagRetriever::config["PluginDBSettings"]["TableName"].get<std::string>(), condition);
}

void ReadConfig()
{
	try
	{
		const std::string config_path = AsaApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/" + PROJECT_NAME + "/config.json";
		std::ifstream file{config_path};
		if (!file.is_open())
		{
			throw std::runtime_error("Can't open config file.");
		}
		file >> DeathBagRetriever::config;

		Log::GetLog()->info("{} config file loaded.", PROJECT_NAME);

		DeathBagRetriever::isDebug = DeathBagRetriever::config["General"]["Debug"].get<bool>();

		Log::GetLog()->warn("Debug {}", DeathBagRetriever::isDebug);

	}
	catch(const std::exception& error)
	{
		Log::GetLog()->error("Config load failed. ERROR: {}", error.what());
		throw;
	}
}

void LoadDatabase()
{
	Log::GetLog()->warn("LoadDatabase");
	DeathBagRetriever::pluginTemplateDB = DatabaseFactory::createConnector(DeathBagRetriever::config["PluginDBSettings"]);

	nlohmann::ordered_json tableDefinition = {};
	if (DeathBagRetriever::config["PluginDBSettings"].value("UseMySQL", true) == true)
	{
		tableDefinition = {
			{"Id", "INT NOT NULL AUTO_INCREMENT"},
			{"EosId", "VARCHAR(50) NOT NULL"},
			{"PlayerId", "VARCHAR(50) NOT NULL"},
			{"PlayerName", "VARCHAR(50) NOT NULL"},
			{"CreateAt", "DATETIME DEFAULT CURRENT_TIMESTAMP"},
			{"PRIMARY", "KEY(Id)"},
			{"UNIQUE", "INDEX EosId_UNIQUE (EosId ASC)"}
		};
	}
	else
	{
		tableDefinition = {
			{"Id","INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT"},
			{"EosId","TEXT NOT NULL UNIQUE"},
			{"PlayerId","TEXT"},
			{"PlayerName","TEXT"},
			{"CreateAt","TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
		};
	}

	DeathBagRetriever::pluginTemplateDB->createTableIfNotExist(DeathBagRetriever::config["PluginDBSettings"].value("TableName", ""), tableDefinition);


	// PermissionsDB
	if (DeathBagRetriever::config["PermissionsDBSettings"].value("Enabled", true) == true)
	{
		DeathBagRetriever::permissionsDB = DatabaseFactory::createConnector(DeathBagRetriever::config["PermissionsDBSettings"]);
	}

	// PointsDB (ArkShop)
	if (DeathBagRetriever::config["PointsDBSettings"].value("Enabled", true) == true)
	{
		DeathBagRetriever::pointsDB = DatabaseFactory::createConnector(DeathBagRetriever::config["PointsDBSettings"]);
	}
	
}