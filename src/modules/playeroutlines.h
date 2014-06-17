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

#include "../statusspec.h"
#include "../enums.h"
#include "../entities.h"
#include "../hooks.h"
#include "../ifaces.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

typedef struct ColorConCommand_s {
	Color color;
	ConCommand *command;
} ColorConCommand_t;

class PlayerOutlines {
public:
	PlayerOutlines();
	~PlayerOutlines();

	bool IsEnabled();

	bool GetGlowEffectColorOverride(C_TFPlayer *tfPlayer, float *r, float *g, float *b);

	void ProcessEntity(IClientEntity* entity);
private:
	std::map<std::string, ColorConCommand_t> colors;
	
	ConVar *enabled;
	ConCommand *force_refresh;
	ConVar *team_colors;
	static void ForceRefresh();
	static void ColorCommand(const CCommand &command);
	static int GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern PlayerOutlines *g_PlayerOutlines;