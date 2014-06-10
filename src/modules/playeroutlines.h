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

#include "../statusspec.h"
#include "../enums.h"
#include "../entities.h"
#include "../hooks.h"
#include "../ifaces.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

typedef struct GlowHooks_s {
	int glowColorHook;
	int glowForceHook;
} GlowHooks_t;

class PlayerOutlines {
public:
	PlayerOutlines();
	~PlayerOutlines();

	bool IsEnabled();

	bool DataChangeOverride(C_BaseCombatCharacter *baseCombatCharacter);

	bool GetGlowEffectColorOverride(C_TFPlayer *tfPlayer, float *r, float *g, float *b);

	void ProcessEntity(IClientEntity* entity);
private:
	std::map<CHandle<C_BaseCombatCharacter>, GlowHooks_t> glowHooks;
	Color team_blu;
	Color team_red;
	
	ConCommand* color_blu;
	ConCommand* color_red;
	ConVar* enabled;
	ConCommand* force_refresh;
	ConVar* team_colors;
	static void ForceRefresh();
	static int GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void SetColor(const CCommand &command);
	static void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern PlayerOutlines *g_PlayerOutlines;