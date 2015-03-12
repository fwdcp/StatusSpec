/*
*  timelefttools.h
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

class ConVar;
class IConVar;
class KeyValues;

class TimeleftTools : public Module {
public:
	TimeleftTools(std::string name);

	static bool CheckDependencies(std::string name);

	void SendMessageOverride(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);
private:
	int sendMessageHook;

	class TeamplayRoundBasedRulesOverride;

	ConVar *count_up;
	ConVar *enabled;
	ConVar *timelimit;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};