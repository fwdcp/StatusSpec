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

#include <cstdint>
#include <map>
#include <sstream>
#include <string>

#include "convar.h"

#undef null
#include "json/json.h"

#include "../entities.h"
#include "../enums.h"
#include "../funcs.h"
#include "../ifaces.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

#define MAX_URL_LENGTH 2048

#define ESEA_PLAYER_API_URL "http://play.esea.net/index.php"
#define ETF2L_PLAYER_API_URL "http://api.etf2l.org/player/%llu.json"
#define TWITCH_USER_API_URL "http://api.twitch.tv/api/steam/%llu"
#define STEAM_USER_ID_FORMAT "[U:%i:%lu]"

typedef enum APIStatus_s {
	API_UNKNOWN,
	API_REQUESTED,
	API_FAILED,
	API_NO_RESULT,
	API_SUCCESSFUL
} APIStatus_t;

class PlayerAliases;

typedef struct APIAlias_s {
	std::string name;
	APIStatus_t status = API_UNKNOWN;
	CCallResult<PlayerAliases, HTTPRequestCompleted_t> call;
} APIAlias_t;

class PlayerAliases {
public:
	PlayerAliases();

	bool IsEnabled();

	bool GetPlayerInfoOverride(int ent_num, player_info_t *pinfo);

	void GetESEAPlayerInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure);
	void GetETF2LPlayerInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure);
	void GetTwitchUserInfo(HTTPRequestCompleted_t *requestCompletionInfo, bool bIOFailure);
	void RequestESEAPlayerInfo(CSteamID player);
	void RequestETF2LPlayerInfo(CSteamID player);
	void RequestTwitchUserInfo(CSteamID player);
private:
	std::map<CSteamID, std::string> customAliases;
	std::map<CSteamID, APIAlias_t> eseaAliases;
	std::map<CSteamID, APIAlias_t> etf2lAliases;
	std::map<CSteamID, APIAlias_t> twitchAliases;

	std::string GetAlias(CSteamID player, std::string gameAlias);

	ConVar *enabled;
	ConVar *esea;
	ConVar *etf2l;
	ConVar *format_blu;
	ConVar *format_red;
	ConCommand *get;
	ConCommand *remove;
	ConCommand *set;
	ConCommand *switch_teams;
	ConVar *twitch;
	static int GetCurrentAliasedPlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static int GetCurrentGamePlayers(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void GetCustomPlayerAlias(const CCommand &command);
	static void RemoveCustomPlayerAlias(const CCommand &command);
	static void SetCustomPlayerAlias(const CCommand &command);
	static void SwitchTeams();
};

extern PlayerAliases *g_PlayerAliases;