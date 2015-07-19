/*
 *  multipanel.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "vgui/VGUI.h"

#include "../modules.h"

class CGameConsoleDialog;
class ConVar;
class IConVar;

class MultiPanel : public Module {
public:
	MultiPanel();

	static bool CheckDependencies();
private:
	CGameConsoleDialog *consoleDialog;
	vgui::HPanel scoreboardPanel;

	void InitHud();

	ConVar *console;
	ConVar *scoreboard;
	void ToggleConsole(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleScoreboard(IConVar *var, const char *pOldValue, float flOldValue);
};