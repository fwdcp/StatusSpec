/*
 *  playeroutlines.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include <map>
#include <sstream>
#include <string>

#include "convar.h"
#include "ehandle.h"

#include "../enums.h"
#include "../entities.h"
#include "../funcs.h"
#include "../glows.h"
#include "../ifaces.h"
#include "../player.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

class PlayerOutlines {
public:
	PlayerOutlines();

	bool DoPostScreenSpaceEffectsHook(const CViewSetup *pSetup);
	void FrameHook(ClientFrameStage_t curStage);
private:
	std::map<std::string, ColorConCommand_t> colors;
	int doPostScreenSpaceEffectsHook;
	int frameHook;
	std::map<EHANDLE, CGlowObject *> glows;

	Color GetGlowColor(Player player);
	void SetGlowEffect(IClientEntity *entity, bool enabled, Vector color = Vector(1.0f, 1.0f, 1.0f), float alpha = 1.0f);
	
	ConVar *enabled;
	ConVar *fade;
	ConVar *fade_distance;
	ConVar *health_adjusted_team_colors;
	ConVar *team_colors;
	void ColorCommand(const CCommand &command);
	int GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleFade(IConVar *var, const char *pOldValue, float flOldValue);
};

extern PlayerOutlines *g_PlayerOutlines;