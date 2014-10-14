/*
 *  antifreeze.h
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
#include "icliententity.h"
#include "KeyValues.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"

#include "../entities.h"
#include "../ifaces.h"
#include "../player.h"

#define PERFORM_LAYOUT_COMMAND 
#define SPEC_GUI_NAME "specgui"
#define TOP_PANEL_NAME "MatSystemTopPanel"

typedef struct EntityInfo_s {
	Vector origin;
	QAngle angles;
} EntityInfo_t;

class AntiFreeze {
public:
	AntiFreeze();

	void FrameHook(ClientFrameStage_t curStage);
private:
	bool entitiesUpdated;
	int frameHook;
	vgui::EditablePanel *freezeInfoPanel;
	double lastEntityUpdate;
	KeyValues *performLayoutCommand;
	vgui::HPanel specguiPanel;

	void GetSpecGUI();
	void InitHud();

	ConVar *display;
	ConCommand *display_reload_settings;
	ConVar *display_threshold;
	ConVar *enabled;
	void ReloadSettings();
	void ToggleDisplay(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern AntiFreeze *g_AntiFreeze;