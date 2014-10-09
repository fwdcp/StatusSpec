/*
*  multipanel.h
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#pragma once

#include "../stdafx.h"

#include "game/client/iviewport.h"
#include "vgui/IVGui.h"
#include "../vgui_controls/gameconsoledialog.h"
#include "vgui_controls/animationcontroller.h"

#include "../ifaces.h"

#define SCOREBOARD_PANEL_NAME "scores"

class MultiPanel {
public:
	MultiPanel();
private:
	CGameConsoleDialog *consoleDialog;
	vgui::HPanel scoreboardPanel;

	void InitHud();

	ConVar *console;
	ConVar *scoreboard;
	void ToggleConsole(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleScoreboard(IConVar *var, const char *pOldValue, float flOldValue);
};

extern MultiPanel *g_MultiPanel;