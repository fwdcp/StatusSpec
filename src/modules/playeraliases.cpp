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

PlayerAliases::PlayerAliases() {
	std::map<CSteamID, std::string> playerAliases;

	enabled = new ConVar("statusspec_playeraliases_enabled", "0", FCVAR_NONE, "enable player aliases");
	get = new ConCommand("statusspec_playeraliases_get", PlayerAliases::GetPlayerAlias, "get a player alias", FCVAR_NONE, PlayerAliases::GetCurrentAliasedPlayers);
	remove = new ConCommand("statusspec_playeraliases_remove", PlayerAliases::RemovePlayerAlias, "remove a player alias", FCVAR_NONE, PlayerAliases::GetCurrentAliasedPlayers);
	set = new ConCommand("statusspec_playeraliases_set", PlayerAliases::SetPlayerAlias, "set a player alias", FCVAR_NONE, PlayerAliases::GetCurrentGamePlayers);
}

bool PlayerAliases::IsEnabled() {
	return enabled->GetBool();
}

bool PlayerAliases::GetPlayerInfoOverride(int ent_num, player_info_t *pinfo) {
	bool result = SH_CALL(Interfaces::pEngineClient, &IVEngineClient::GetPlayerInfo)(ent_num, pinfo);

	CSteamID playerSteamID = GetClientSteamID(ent_num);

	if (playerAliases.find(playerSteamID) != playerAliases.end()) {
		V_strcpy_safe(pinfo->name, playerAliases[playerSteamID].c_str());
	}

	return result;
}

const char * PlayerAliases::GetPlayerNameOverride(int client) {
	CSteamID playerSteamID = GetClientSteamID(client);

	if (playerAliases.find(playerSteamID) != playerAliases.end()) {
		return playerAliases[playerSteamID].c_str();
	}
	else {
		return SH_CALL(Interfaces::GetGameResources(), &IGameResources::GetPlayerName)(client);
	}
}

int PlayerAliases::GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (auto playerAlias = g_PlayerAliases->playerAliases.begin(); playerAlias != g_PlayerAliases->playerAliases.end() && playerCount < COMMAND_COMPLETION_MAXITEMS; ++playerAlias) {
		CSteamID playerSteamID = playerAlias->first;

		V_snprintf(commands[playerCount], COMMAND_COMPLETION_ITEM_LENGTH, "%s %llu", command.c_str(), playerSteamID.ConvertToUint64());
		playerCount++;
	}

	return playerCount;
}

int PlayerAliases::GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
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

void PlayerAliases::GetPlayerAlias(const CCommand &command) {
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

	if (g_PlayerAliases->playerAliases.find(playerSteamID) != g_PlayerAliases->playerAliases.end()) {
		Msg("Steam ID %llu has an associated alias '%s'.\n", playerSteamID.ConvertToUint64(), g_PlayerAliases->playerAliases[playerSteamID].c_str());
	}
	else {
		Msg("Steam ID %llu does not have an associated alias.\n", playerSteamID.ConvertToUint64());
	}
}

void PlayerAliases::RemovePlayerAlias(const CCommand &command) {
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

	g_PlayerAliases->playerAliases.erase(playerSteamID);
	Msg("Alias associated with Steam ID %llu erased.\n", playerSteamID.ConvertToUint64());
}

void PlayerAliases::SetPlayerAlias(const CCommand &command) {
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

	g_PlayerAliases->playerAliases[playerSteamID] = command.Arg(2);
	Msg("Steam ID %llu has been associated with alias '%s'.\n", playerSteamID.ConvertToUint64(), g_PlayerAliases->playerAliases[playerSteamID].c_str());
}