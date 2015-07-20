/*
 *  playeraliases.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "playeraliases.h"

#include "rapidjson/document.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

PlayerAliases::PlayerAliases() {
	getPlayerInfoHook = 0;

	enabled = new ConVar("statusspec_playeraliases_enabled", "0", FCVAR_NONE, "enable player aliases", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<PlayerAliases>()->ToggleEnabled(var, pOldValue, flOldValue); });
	format_blu = new ConVar("statusspec_playeraliases_format_blu", "%alias%", FCVAR_NONE, "the name format for BLU players");
	format_red = new ConVar("statusspec_playeraliases_format_red", "%alias%", FCVAR_NONE, "the name format for RED players");
	get = new ConCommand("statusspec_playeraliases_get", [](const CCommand &command) { g_ModuleManager->GetModule<PlayerAliases>()->GetCustomPlayerAlias(command); }, "get a custom player alias", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<PlayerAliases>()->GetCurrentAliasedPlayers(partial, commands); });
	remove = new ConCommand("statusspec_playeraliases_remove", [](const CCommand &command) { g_ModuleManager->GetModule<PlayerAliases>()->RemoveCustomPlayerAlias(command); }, "remove a custom player alias", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<PlayerAliases>()->GetCurrentAliasedPlayers(partial, commands); });
	set = new ConCommand("statusspec_playeraliases_set", [](const CCommand &command) { g_ModuleManager->GetModule<PlayerAliases>()->SetCustomPlayerAlias(command); }, "set a custom player alias", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<PlayerAliases>()->GetCurrentGamePlayers(partial, commands); });
	switch_teams = new ConCommand("statusspec_playeraliases_switch_teams", []() { g_ModuleManager->GetModule<PlayerAliases>()->SwitchTeams(); }, "switch name formats for both teams", FCVAR_NONE);
}

bool PlayerAliases::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerAliases>().c_str());

		ready = false;
	}

	if (!Interfaces::steamLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required Steam libraries for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerAliases>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerAliases>().c_str());

		ready = false;
	}

	if (!Player::steamIDRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player Steam ID retrieval for module %s not available!\n", g_ModuleManager->GetModuleName<PlayerAliases>().c_str());

		ready = false;
	}

	return ready;
}

bool PlayerAliases::GetPlayerInfoOverride(int ent_num, player_info_t *pinfo) {
	bool result = Funcs::CallFunc_IVEngineClient_GetPlayerInfo(Interfaces::pEngineClient, ent_num, pinfo);

	Player player = ent_num;

	if (!player) {
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	static EUniverse universe = k_EUniverseInvalid;

	if (universe == k_EUniverseInvalid) {
		if (Interfaces::pSteamAPIContext->SteamUtils()) {
			universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
		}
		else {
			PRINT_TAG();
			Warning("Steam libraries not available - assuming public universe for user Steam IDs!\n");

			universe = k_EUniversePublic;
		}
	}

	CSteamID playerSteamID = CSteamID(pinfo->friendsID, 1, universe, k_EAccountTypeIndividual);
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
	
	return gameAlias;
}

int PlayerAliases::GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	int playerCount = 0;

	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	for (auto iterator : customAliases) {
		CSteamID playerSteamID = iterator.first;

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

	for (Player player : Player::Iterable()) {
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