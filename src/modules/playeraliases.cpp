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

PlayerAliases::PlayerAliases() {
	getPlayerInfoHook = 0;

	enabled = new ConVar("statusspec_playeraliases_enabled", "0", FCVAR_NONE, "enable player aliases", [](IConVar *var, const char *pOldValue, float flOldValue) { g_PlayerAliases->ToggleEnabled(var, pOldValue, flOldValue); });
	esea = new ConVar("statusspec_playeraliases_esea", "0", FCVAR_NONE, "enable player aliases from the ESEA API");
	etf2l = new ConVar("statusspec_playeraliases_etf2l", "0", FCVAR_NONE, "enable player aliases from the ETF2L API");
	format_blu = new ConVar("statusspec_playeraliases_format_blu", "%alias%", FCVAR_NONE, "the name format for BLU players");
	format_red = new ConVar("statusspec_playeraliases_format_red", "%alias%", FCVAR_NONE, "the name format for RED players");
	get = new ConCommand("statusspec_playeraliases_get", [](const CCommand &command) { g_PlayerAliases->GetCustomPlayerAlias(command); }, "get a custom player alias", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_PlayerAliases->GetCurrentAliasedPlayers(partial, commands); });
	remove = new ConCommand("statusspec_playeraliases_remove", [](const CCommand &command) { g_PlayerAliases->RemoveCustomPlayerAlias(command); }, "remove a custom player alias", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_PlayerAliases->GetCurrentAliasedPlayers(partial, commands); });
	set = new ConCommand("statusspec_playeraliases_set", [](const CCommand &command) { g_PlayerAliases->SetCustomPlayerAlias(command); }, "set a custom player alias", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_PlayerAliases->GetCurrentGamePlayers(partial, commands); });
	twitch = new ConVar("statusspec_playeraliases_twitch", "0", FCVAR_NONE, "enable player aliases from the Twitch API");
	switch_teams = new ConCommand("statusspec_playeraliases_switch_teams", []() { g_PlayerAliases->SwitchTeams(); }, "switch name formats for both teams", FCVAR_NONE);
}

bool PlayerAliases::GetPlayerInfoOverride(int ent_num, player_info_t *pinfo) {
	bool result = Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, ent_num, pinfo);

	Player player = ent_num;

	if (!player) {
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	CSteamID playerSteamID = player.GetSteamID();
	TFTeam team = player.GetTeam();

	std::string playerAlias = GetAlias(playerSteamID, pinfo->name);

	std::string gameName;

	if (team == TFTeam_Red) {
		gameName = format_red->GetString();
	}
	else if (team == TFTeam_Blue) {
		gameName = format_blu->GetString();
	}
	else {
		gameName = "%alias%";
	}

	FindAndReplaceInString(gameName, "%alias%", playerAlias);

	V_strcpy_safe(pinfo->name, gameName.c_str());

	RETURN_META_VALUE(MRES_SUPERCEDE, result);
}

std::string PlayerAliases::GetAlias(CSteamID player, std::string gameAlias) {
	if (!player.IsValid()) {
		return gameAlias;
	}

	if (customAliases.find(player) != customAliases.end()) {
		return customAliases[player];
	}

	if (esea->GetBool()) {
		if (eseaAliases[player].status == API_UNKNOWN) {
			RequestESEAPlayerInfo(player);
		}
		else if (eseaAliases[player].status == API_SUCCESSFUL) {
			return eseaAliases[player].name;
		}
	}

	if (etf2l->GetBool()) {
		if (etf2lAliases[player].status == API_UNKNOWN) {
			RequestETF2LPlayerInfo(player);
		}
		else if (etf2lAliases[player].status == API_SUCCESSFUL) {
			return etf2lAliases[player].name;
		}
	}

	if (twitch->GetBool()) {
		if (twitchAliases[player].status == API_UNKNOWN) {
			RequestTwitchUserInfo(player);
		}
		else if (twitchAliases[player].status == API_SUCCESSFUL) {
			return twitchAliases[player].name;
		}
	}
	
	return gameAlias;
}

void PlayerAliases::GetESEAPlayerInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure) {
	CSteamID player = CSteamID(requestCompletionInfo->m_ulContextValue);

	if (bIOFailure || !requestCompletionInfo->m_bRequestSuccessful) {
		eseaAliases[player].status = API_UNKNOWN;
	}
	else if (requestCompletionInfo->m_eStatusCode != k_EHTTPStatusCode200OK) {
		eseaAliases[player].status = API_FAILED;
	}
	else {
		ISteamHTTP *httpClient = Interfaces::pSteamAPIContext->SteamHTTP();

		uint32 bodySize;
		httpClient->GetHTTPResponseBodySize(requestCompletionInfo->m_hRequest, &bodySize);

		uint8 *body = new uint8[bodySize];
		httpClient->GetHTTPResponseBodyData(requestCompletionInfo->m_hRequest, body, bodySize);

		std::string json = (char *)body;

		Json::Reader reader;
		Json::Value root;

		if (reader.parse(json, root)) {
			if (root.isMember("results") && root["results"].size() > 0) {
				if (root["results"][0].isMember("alias")) {
					eseaAliases[player].name = root["results"][0]["alias"].asString();
					eseaAliases[player].status = API_SUCCESSFUL;
					httpClient->ReleaseHTTPRequest(requestCompletionInfo->m_hRequest);

					return;
				}
			}
		}

		eseaAliases[player].status = API_FAILED;
	}

	Interfaces::pSteamAPIContext->SteamHTTP()->ReleaseHTTPRequest(requestCompletionInfo->m_hRequest);
}

void PlayerAliases::GetETF2LPlayerInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure) {
	CSteamID player = CSteamID(requestCompletionInfo->m_ulContextValue);

	if (bIOFailure || !requestCompletionInfo->m_bRequestSuccessful) {
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
		httpClient->GetHTTPResponseBodyData(requestCompletionInfo->m_hRequest, body, bodySize);

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

void PlayerAliases::GetTwitchUserInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure) {
	CSteamID player = CSteamID(requestCompletionInfo->m_ulContextValue);

	if (bIOFailure || !requestCompletionInfo->m_bRequestSuccessful) {
		twitchAliases[player].status = API_UNKNOWN;
	}
	else if (requestCompletionInfo->m_eStatusCode != k_EHTTPStatusCode200OK) {
		twitchAliases[player].status = API_FAILED;
	}
	else {
		ISteamHTTP *httpClient = Interfaces::pSteamAPIContext->SteamHTTP();

		uint32 bodySize;
		httpClient->GetHTTPResponseBodySize(requestCompletionInfo->m_hRequest, &bodySize);

		uint8 *body = new uint8[bodySize];
		httpClient->GetHTTPResponseBodyData(requestCompletionInfo->m_hRequest, body, bodySize);

		std::string json = (char *)body;

		Json::Reader reader;
		Json::Value root;

		if (reader.parse(json, root)) {
			if (!root.isMember("status") || root["status"] == 200) {
				twitchAliases[player].name = root["name"].asString();
				twitchAliases[player].status = API_SUCCESSFUL;
				httpClient->ReleaseHTTPRequest(requestCompletionInfo->m_hRequest);

				return;
			}
		}

		twitchAliases[player].status = API_FAILED;
	}

	Interfaces::pSteamAPIContext->SteamHTTP()->ReleaseHTTPRequest(requestCompletionInfo->m_hRequest);
}

void PlayerAliases::RequestESEAPlayerInfo(CSteamID player) {
	ISteamHTTP *httpClient = Interfaces::pSteamAPIContext->SteamHTTP();

	char id[MAX_URL_LENGTH];
	V_snprintf(id, sizeof(id), STEAM_USER_ID_FORMAT, Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse(), player.GetAccountID());

	HTTPRequestHandle request = httpClient->CreateHTTPRequest(k_EHTTPMethodGET, ESEA_PLAYER_API_URL);
	httpClient->SetHTTPRequestGetOrPostParameter(request, "s", "search");
	httpClient->SetHTTPRequestGetOrPostParameter(request, "query", id);
	httpClient->SetHTTPRequestGetOrPostParameter(request, "source", "users");
	httpClient->SetHTTPRequestGetOrPostParameter(request, "fields%5Bunique_ids%5D", "1");
	httpClient->SetHTTPRequestGetOrPostParameter(request, "format", "json");
	httpClient->SetHTTPRequestHeaderValue(request, "Cookie", "viewed_welcome_page=1");
	httpClient->SetHTTPRequestContextValue(request, player.ConvertToUint64());

	SteamAPICall_t apiCall;
	if (httpClient->SendHTTPRequest(request, &apiCall)) {
		eseaAliases[player].status = API_REQUESTED;
		eseaAliases[player].call.Set(apiCall, this, &PlayerAliases::GetESEAPlayerInfo);
	}
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

void PlayerAliases::RequestTwitchUserInfo(CSteamID player) {
	ISteamHTTP *httpClient = Interfaces::pSteamAPIContext->SteamHTTP();

	char url[MAX_URL_LENGTH];
	V_snprintf(url, sizeof(url), TWITCH_USER_API_URL, player.ConvertToUint64());

	HTTPRequestHandle request = httpClient->CreateHTTPRequest(k_EHTTPMethodGET, url);
	httpClient->SetHTTPRequestContextValue(request, player.ConvertToUint64());

	SteamAPICall_t apiCall;
	if (httpClient->SendHTTPRequest(request, &apiCall)) {
		twitchAliases[player].status = API_REQUESTED;
		twitchAliases[player].call.Set(apiCall, this, &PlayerAliases::GetTwitchUserInfo);
	}
}

int PlayerAliases::GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (auto playerAlias = customAliases.begin(); playerAlias != customAliases.end() && playerCount < COMMAND_COMPLETION_MAXITEMS; ++playerAlias) {
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
		Player player = i;

		if (!player) {
			continue;
		}

		CSteamID playerSteamID = player.GetSteamID();
			
		if (playerSteamID.IsValid()) {
			V_snprintf(commands[playerCount], COMMAND_COMPLETION_ITEM_LENGTH, "%s %llu", command.c_str(), playerSteamID.ConvertToUint64());
			playerCount++;
		}
}

	return playerCount;
}

void PlayerAliases::GetCustomPlayerAlias(const CCommand &command) {
	if (command.ArgC() < 2)
	{
		Warning("Usage: statusspec_playeraliases_get <steamid>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(command.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	if (customAliases.find(playerSteamID) != customAliases.end()) {
		Msg("Steam ID %llu has an associated alias '%s'.\n", playerSteamID.ConvertToUint64(), customAliases[playerSteamID].c_str());
	}
	else {
		Msg("Steam ID %llu does not have an associated alias.\n", playerSteamID.ConvertToUint64());
	}
}

void PlayerAliases::RemoveCustomPlayerAlias(const CCommand &command) {
	if (command.ArgC() < 2)
	{
		Warning("Usage: statusspec_playeraliases_remove <steamid>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(command.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	customAliases.erase(playerSteamID);
	Msg("Alias associated with Steam ID %llu erased.\n", playerSteamID.ConvertToUint64());
}

void PlayerAliases::SetCustomPlayerAlias(const CCommand &command) {
	if (command.ArgC() < 3)
	{
		Warning("Usage: statusspec_playeraliases_set <steamid> <alias>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(command.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	customAliases[playerSteamID] = command.Arg(2);
	Msg("Steam ID %llu has been associated with alias '%s'.\n", playerSteamID.ConvertToUint64(), customAliases[playerSteamID].c_str());
}

void PlayerAliases::SwitchTeams() {
	std::string newBluFormat = format_red->GetString();
	std::string newRedFormat = format_blu->GetString();

	format_blu->SetValue(newBluFormat.c_str());
	format_red->SetValue(newRedFormat.c_str());
}

void PlayerAliases::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!getPlayerInfoHook) {
			getPlayerInfoHook = Funcs::AddHook_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, SH_MEMBER(this, &PlayerAliases::GetPlayerInfoOverride), false);
		}
	}
	else {
		if (getPlayerInfoHook) {
			if (Funcs::RemoveHook(getPlayerInfoHook)) {
				getPlayerInfoHook = 0;
			}
		}
	}
}