/*
 *  projectileoutlines.h
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

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../glows.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"
#include "../tfdefs.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

class ProjectileOutlines : public Module {
public:
	ProjectileOutlines(std::string name);

	static bool CheckDependencies(std::string name);

	bool DoPostScreenSpaceEffectsHook(const CViewSetup *pSetup);
	void FrameHook(ClientFrameStage_t curStage);
private:
	std::map<std::string, ColorConCommand_t> colors;
	int doPostScreenSpaceEffectsHook;
	int frameHook;
	std::map<EHANDLE, GlowManager::GlowObject *> glows;
	GlowManager glowManager;

	Color GetGlowColor(IClientEntity *entity);
	void SetGlowEffect(IClientEntity *entity, bool enabled, Vector color = Vector(1.0f, 1.0f, 1.0f), float alpha = 1.0f);

	ConVar *enabled;
	ConVar *fade;
	ConVar *fade_distance;
	ConVar *grenades;
	ConVar *rockets;
	ConVar *stickybombs;
	void ColorCommand(const CCommand &command);
	int GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleFade(IConVar *var, const char *pOldValue, float flOldValue);
};