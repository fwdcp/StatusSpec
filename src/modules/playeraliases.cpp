/*
 *  playeraliases.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "playeraliases.h"

inline bool IsInteger(const std::string &s) {
   if (s.empty() || !isdigit(s[0])) return false;

   char *p;
   strtoull(s.c_str(), &p, 10);

   return (*p == 0);
}

inline CSteamID ConvertTextToSteamID(std::string textID) {
	if (textID.substr(0, 6).compare("STEAM_") == 0 && std::count(textID.begin(), textID.end(), ':') == 2) {
		std::stringstream ss(textID);
		std::string universe;
		std::string server;
		std::string authID;
		std::getline(ss, universe, ':');
		std::getline(ss, server, ':');
		std::getline(ss, authID, ':');

		if (IsInteger(server) && IsInteger(authID)) {
			uint32_t accountID = (2 * strtoul(authID.c_str(), nullptr, 10)) + strtoul(server.c_str(), nullptr, 10);

			static EUniverse universe = k_EUniverseInvalid;

			if (universe == k_EUniverseInvalid) {
				universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
			}

			return CSteamID(accountID, universe, k_EAccountTypeIndividual);
		}

		return CSteamID();
	}
	else if (IsInteger(textID)) {
		uint64_t steamID = strtoull(textID.c_str(), nullptr, 10);

		return CSteamID(steamID);
	}

	return CSteamID();
}

inline CSteamID GetClientSteamID(int client) {
	player_info_t playerInfo;

	if (Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, client, &playerInfo)) {
		if (playerInfo.friendsID) {
			static EUniverse universe = k_EUniverseInvalid;

			if (universe == k_EUniverseInvalid) {
				universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
			}

			return CSteamID(playerInfo.friendsID, 1, universe, k_EAccountTypeIndividual);
		}
	}

	return CSteamID();
}

PlayerAliases::PlayerAliases() {
	std::map<CSteamID, std::string> customAliases;

	enabled = new ConVar("statusspec_playeraliases_enabled", "0", FCVAR_NONE, "enable player aliases");
	etf2l = new ConVar("statusspec_playeraliases_etf2l", "0", FCVAR_NONE, "enable player aliases from the ETF2L API");
	get = new ConCommand("statusspec_playeraliases_get", PlayerAliases::GetCustomPlayerAlias, "get a custom player alias", FCVAR_NONE, PlayerAliases::GetCurrentAliasedPlayers);
	remove = new ConCommand("statusspec_playeraliases_remove", PlayerAliases::RemoveCustomPlayerAlias, "remove a custom player alias", FCVAR_NONE, PlayerAliases::GetCurrentAliasedPlayers);
	set = new ConCommand("statusspec_playeraliases_set", PlayerAliases::SetCustomPlayerAlias, "set a custom player alias", FCVAR_NONE, PlayerAliases::GetCurrentGamePlayers);
}

bool PlayerAliases::IsEnabled() {
	return enabled->GetBool();
}

bool PlayerAliases::GetPlayerInfoOverride(int ent_num, player_info_t *pinfo) {
	bool result = Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, ent_num, pinfo);

	CSteamID playerSteamID = GetClientSteamID(ent_num);

	std::string playerAlias;
	if (GetAlias(playerSteamID, playerAlias)) {
		V_strcpy_safe(pinfo->name, playerAlias.c_str());
	}

	return result;
}

const char * PlayerAliases::GetPlayerNameOverride(int client) {
	CSteamID playerSteamID = GetClientSteamID(client);

	std::string playerAlias;
	if (GetAlias(playerSteamID, playerAlias)) {
		return playerAlias.c_str();
	}
	else {
		return Funcs::CallFunc_IGameResources_GetPlayerName(Interfaces::GetGameResources(), client);
	}
}

bool PlayerAliases::GetAlias(CSteamID player, std::string &alias) {
	if (!player.IsValid()) {
		return false;
	}

	if (customAliases.find(player) != customAliases.end()) {
		alias = customAliases[player];
		return true;
	}

	if (etf2l->GetBool()) {
		if (etf2lAliases[player].status == API_UNKNOWN) {
			RequestETF2LPlayerInfo(player);
		}
		else if (etf2lAliases[player].status == API_SUCCESSFUL) {
			alias = etf2lAliases[player].name;
			return true;
		}
	}
	
	return false;
}

void PlayerAliases::GetETF2LPlayerInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure) {
	CSteamID player = CSteamID(requestCompletionInfo->m_ulContextValue);

	if (!requestCompletionInfo->m_bRequestSuccessful) {
		etf2lAliases[player].status = API_UNKNOWN;
	}
	else if (requestCompletionInfo->m_eStatusCode != k_EHTTPStatusCode200OK) {
		etf2lAliases[player].status = API_FAILED;
	}
	else {
		ISteamHTTP *httpClient = Interfaces::pSteamAPIContext->SteamHTTP();
		
		uint32 bodySize;
		httpClient->GetHTTPResponseBodySize(requestCompletionInfo->m_hRequest, &bodySize);

		uint8 *body = new uint8[bodySize];
		httpClient->GetHTTPResponseBodyData(requestCompletionInfo->m_hRequest, (uint8 *)body, bodySize);

		std::string json = (char *)body;
		
		Json::Reader reader;
		Json::Value root;

		if (reader.parse(json, root)) {
			if (root["status"]["code"].asInt() == 200) {
				if (root["player"]["steam"]["id64"].asString() == std::to_string(player.ConvertToUint64())) {
					etf2lAliases[player].name = root["player"]["name"].asString();
					etf2lAliases[player].status = API_SUCCESSFUL;
					httpClient->ReleaseHTTPRequest(requestCompletionInfo->m_hRequest);

					return;
				}
			}
		}

		etf2lAliases[player].status = API_FAILED;
	}

	Interfaces::pSteamAPIContext->SteamHTTP()->ReleaseHTTPRequest(requestCompletionInfo->m_hRequest);
}

void PlayerAliases::RequestETF2LPlayerInfo(CSteamID player) {
	ISteamHTTP *httpClient = Interfaces::pSteamAPIContext->SteamHTTP();

	char url[MAX_URL_LENGTH];
	V_snprintf(url, sizeof(url), ETF2L_PLAYER_API_URL, player.ConvertToUint64());

	HTTPRequestHandle request = httpClient->CreateHTTPRequest(k_EHTTPMethodGET, url);
	httpClient->SetHTTPRequestContextValue(request, player.ConvertToUint64());

	SteamAPICall_t apiCall;
	if (httpClient->SendHTTPRequest(request, &apiCall)) {
		etf2lAliases[player].status = API_REQUESTED;
		etf2lAliases[player].call.Set(apiCall, this, &PlayerAliases::GetETF2LPlayerInfo);
	}
}

int PlayerAliases::GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (auto playerAlias = g_PlayerAliases->customAliases.begin(); playerAlias != g_PlayerAliases->customAliases.end() && playerCount < COMMAND_COMPLETION_MAXITEMS; ++playerAlias) {
		CSteamID playerSteamID = playerAlias->first;

		V_snprintf(commands[playerCount], COMMAND_COMPLETION_ITEM_LENGTH, "%s %llu", command.c_str(), playerSteamID.ConvertToUint64());
		playerCount++;
	}

	return playerCount;
}

int PlayerAliases::GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (int i = 0; i <= MAX_PLAYERS; i++) {
		if (Interfaces::GetGameResources()->IsConnected(i)) {
			CSteamID playerSteamID = GetClientSteamID(i);
			
			if (playerSteamID.IsValid()) {
				V_snprintf(commands[playerCount], COMMAND_COMPLETION_ITEM_LENGTH, "%s %llu", command.c_str(), playerSteamID.ConvertToUint64());
				playerCount++;
			}
		}
	}

	return playerCount;


}

void PlayerAliases::GetCustomPlayerAlias(const CCommand &command) {
	if (command.ArgC() < 1)
	{
		Warning("Usage: statusspec_player_alias_get <steamid>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(command.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	if (g_PlayerAliases->customAliases.find(playerSteamID) != g_PlayerAliases->customAliases.end()) {
		Msg("Steam ID %llu has an associated alias '%s'.\n", playerSteamID.ConvertToUint64(), g_PlayerAliases->customAliases[playerSteamID].c_str());
	}
	else {
		Msg("Steam ID %llu does not have an associated alias.\n", playerSteamID.ConvertToUint64());
	}
}

void PlayerAliases::RemoveCustomPlayerAlias(const CCommand &command) {
	if (command.ArgC() < 1)
	{
		Warning("Usage: statusspec_player_alias_remove <steamid>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(command.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	g_PlayerAliases->customAliases.erase(playerSteamID);
	Msg("Alias associated with Steam ID %llu erased.\n", playerSteamID.ConvertToUint64());
}

void PlayerAliases::SetCustomPlayerAlias(const CCommand &command) {
	if (command.ArgC() < 2)
	{
		Warning("Usage: statusspec_player_alias_set <steamid> <alias>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(command.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	g_PlayerAliases->customAliases[playerSteamID] = command.Arg(2);
	Msg("Steam ID %llu has been associated with alias '%s'.\n", playerSteamID.ConvertToUint64(), g_PlayerAliases->customAliases[playerSteamID].c_str());
}