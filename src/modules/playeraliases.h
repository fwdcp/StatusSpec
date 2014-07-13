/*
 *  playeraliases.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>

#include "convar.h"

#include "../funcs.h"
#include "../ifaces.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

class PlayerAliases {
public:
	PlayerAliases();

	bool IsEnabled();

	bool GetPlayerInfoOverride(int ent_num, player_info_t *pinfo);
	const char * GetPlayerNameOverride(int client);
private:
	std::map<CSteamID, std::string> playerAliases;

	ConVar* enabled;
	ConCommand* get;
	ConCommand* remove;
	ConCommand* set;
	static int GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static int GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void GetPlayerAlias(const CCommand &command);
	static void RemovePlayerAlias(const CCommand &command);
	static void SetPlayerAlias(const CCommand &command);
};

extern PlayerAliases *g_PlayerAliases;