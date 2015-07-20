/*
 *  projectileoutlines.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../common.h"
#include "../glows.h"
#include "../modules.h"

#include "cbase.h"
#include "c_baseentity.h"

class ProjectileOutlines : public Module {
public:
	ProjectileOutlines();

	static bool CheckDependencies();
private:
	void UpdateGlowFades(Vector origin);
	void UpdateGlows();

	struct ColorConCommand {
		Color color;
		ConCommand *command;
	};

	std::map<std::string, ColorConCommand> colors;
	std::map<EHANDLE, GlowManager::GlowObject *> glows;

	class Panel;
	Panel *panel;

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