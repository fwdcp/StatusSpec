/*
 *  playeraliases.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "cdll_int.h"
#include "convar.h"
#include "steam/steam_api.h"

#include "../modules.h"

class PlayerAliases : public Module {
public:
	PlayerAliases();

	static bool CheckDependencies();
private:
	bool GetPlayerInfoOverride(int ent_num, player_info_t *pinfo);

	std::map<CSteamID, std::string> customAliases;
	int getPlayerInfoHook;

	std::string GetAlias(CSteamID player, std::string gameAlias);

	ConVar *enabled;
	ConVar *format_blu;
	ConVar *format_red;
	ConCommand *get;
	ConCommand *remove;
	ConCommand *set;
	ConCommand *switch_teams;
	int GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	int GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	void GetCustomPlayerAlias(const CCommand &command);
	void RemoveCustomPlayerAlias(const CCommand &command);
	void SetCustomPlayerAlias(const CCommand &command);
	void SwitchTeams();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};