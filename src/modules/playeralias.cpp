/*
 *  playeralias.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "playeralias.h"

PlayerAlias *g_PlayerAlias;

std::map<CSteamID, std::string> PlayerAlias::playerAliases;

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
			uint32_t accountID = (2 * strtoul(authID.c_str(), NULL, 10)) + strtoul(server.c_str(), NULL, 10);

			static EUniverse universe = k_EUniverseInvalid;

			if (universe == k_EUniverseInvalid) {
				universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
			}

			return CSteamID(accountID, universe, k_EAccountTypeIndividual);
		}

		return CSteamID();
	}
	else if (IsInteger(textID)) {
		uint64_t steamID = strtoull(textID.c_str(), NULL, 10);

		return CSteamID(steamID);
	}

	return CSteamID();
}

inline CSteamID GetClientSteamID(int client) {
	player_info_t playerInfo;

	if (SH_CALL(Interfaces::pEngineClient, &IVEngineClient::GetPlayerInfo)(client, &playerInfo)) {
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

ConVar PlayerAlias::enabled("statusspec_playeralias_enabled", "0", FCVAR_NONE, "enable player aliases", PlayerAlias::ToggleState);
ConCommand PlayerAlias::get("statusspec_playeralias_get", PlayerAlias::GetPlayerAlias, "get a player alias", FCVAR_NONE, PlayerAlias::GetCurrentAliasedPlayers);
ConCommand PlayerAlias::remove("statusspec_playeralias_remove", PlayerAlias::RemovePlayerAlias, "remove a player alias", FCVAR_NONE, PlayerAlias::GetCurrentAliasedPlayers);
ConCommand PlayerAlias::set("statusspec_playeralias_set", PlayerAlias::SetPlayerAlias, "set a player alias", FCVAR_NONE, PlayerAlias::GetCurrentGamePlayers);

bool PlayerAlias::GetPlayerInfoOverride(int ent_num, player_info_t *pinfo) {
	bool result = SH_CALL(Interfaces::pEngineClient, &IVEngineClient::GetPlayerInfo)(ent_num, pinfo);

	CSteamID playerSteamID = GetClientSteamID(ent_num);

	if (playerAliases.find(playerSteamID) != playerAliases.end()) {
		V_strcpy_safe(pinfo->name, playerAliases[playerSteamID].c_str());
	}

	return result;
}

const char * PlayerAlias::GetPlayerNameOverride(int client) {
	CSteamID playerSteamID = GetClientSteamID(client);

	if (playerAliases.find(playerSteamID) != playerAliases.end()) {
		return playerAliases[playerSteamID].c_str();
	}
	else {
		return SH_CALL(Interfaces::GetGameResources(), &IGameResources::GetPlayerName)(client);
	}
}

int PlayerAlias::GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (auto playerAlias = playerAliases.begin(); playerAlias != playerAliases.end() && playerCount < COMMAND_COMPLETION_MAXITEMS; ++playerAlias) {
		CSteamID playerSteamID = playerAlias->first;

		V_snprintf(commands[playerCount], COMMAND_COMPLETION_ITEM_LENGTH, "%s %llu", command.c_str(), playerSteamID.ConvertToUint64());
		playerCount++;
	}

	return playerCount;
}

int PlayerAlias::GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (int i = 0; i <= maxEntity; i++) {
		if (Interfaces::GetGameResources()->IsConnected(i)) {
			CSteamID playerSteamID = GetClientSteamID(i);
			
			V_snprintf(commands[playerCount], COMMAND_COMPLETION_ITEM_LENGTH, "%s %llu", command.c_str(), playerSteamID.ConvertToUint64());
			playerCount++;
		}
	}

	return playerCount;


}

void PlayerAlias::GetPlayerAlias(const CCommand &command) {
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

	if (playerAliases.find(playerSteamID) != playerAliases.end()) {
		Msg("Steam ID %llu has an associated alias '%s'.\n", playerSteamID.ConvertToUint64(), playerAliases[playerSteamID].c_str());
	}
	else {
		Msg("Steam ID %llu does not have an associated alias.\n", playerSteamID.ConvertToUint64());
	}
}

void PlayerAlias::RemovePlayerAlias(const CCommand &command) {
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

	playerAliases.erase(playerSteamID);
	Msg("Alias associated with Steam ID %llu erased.\n", playerSteamID.ConvertToUint64());
}

void PlayerAlias::SetPlayerAlias(const CCommand &command) {
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

	playerAliases[playerSteamID] = command.Arg(2);
	Msg("Steam ID %llu has been associated with alias '%s'.\n", playerSteamID.ConvertToUint64(), playerAliases[playerSteamID].c_str());
}

void PlayerAlias::ToggleState(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled.GetBool() && !g_PlayerAlias) {
		g_PlayerAlias = new PlayerAlias();
	}
	else if (!enabled.GetBool() && g_PlayerAlias) {
		delete g_PlayerAlias;
	}
}