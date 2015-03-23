/*
 *  teamoverrides.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "vgui/VGUI.h"

#include "../modules.h"

class ConCommand;
class ConVar;
class IConVar;
class KeyValues;

class TeamOverrides : public Module {
public:
	TeamOverrides(std::string name);

	static bool CheckDependencies(std::string name);

	void SendMessageOverride(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);
private:
	int sendMessageHook;

	ConVar *enabled;
	ConVar *name_blu;
	ConVar *name_red;
	ConVar *names;
	ConVar *score_blu;
	ConVar *score_red;
	ConVar *scores;
	ConCommand *switch_teams;
	void SwitchTeams();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};