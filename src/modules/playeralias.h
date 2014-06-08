/*
 *  playeralias.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <map>
#include <string>

#include "convar.h"
#include "sourcehook/sourcehook.h"

#include "../statusspec.h"
#include "../hooks.h"
#include "../ifaces.h"

class PlayerAlias {
public:
	bool GetPlayerInfoOverride(int ent_num, player_info_t *pinfo);
	const char * GetPlayerNameOverride(int client);
private:
	static std::map<CSteamID, std::string> playerAliases;

	static ConVar enabled;
	static ConCommand get;
	static ConCommand remove;
	static ConCommand set;
	static int GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static int GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void GetPlayerAlias(const CCommand &command);
	static void RemovePlayerAlias(const CCommand &command);
	static void SetPlayerAlias(const CCommand &command);
	static void ToggleState(IConVar *var, const char *pOldValue, float flOldValue);
};

extern PlayerAlias *g_PlayerAlias;