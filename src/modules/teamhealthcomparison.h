/*
*  teamhealthcomparison.h
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
#include <string>

#include "convar.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Controls.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "../vgui_controls/ImageProgressBar.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui_controls/ScalableImagePanel.h"
#include "../vgui_controls/VariableLabel.h"

#include "../entities.h"
#include "../enums.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class TeamHealthComparison {
public:
	TeamHealthComparison();

	void FrameHook(ClientFrameStage_t curStage);
private:
	int frameHook;
	std::map<std::string, vgui::Panel *> panels;
	std::map<TFTeam, int> teamHealthAggregate;

	void InitHUD();
	void Paint();

	ConVar *enabled;
	ConCommand *reload_settings;
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern TeamHealthComparison *g_TeamHealthComparison;