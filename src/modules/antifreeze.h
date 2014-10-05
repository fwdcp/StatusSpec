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

#define PERFORM_LAYOUT_COMMAND new KeyValues("Command", "Command", "performlayout")
#define SPEC_GUI_NAME "specgui"
#define TOP_PANEL_NAME "MatSystemTopPanel"

typedef struct EntityInfo_s {
	Vector origin;
	QAngle angles;
} EntityInfo_t;

class AntiFreeze {
public:
	AntiFreeze();

	bool IsEnabled();

	void Paint(vgui::VPANEL vguiPanel);
	void ProcessEntity(IClientEntity* entity);
	void PostEntityUpdate();
private:
	bool entitiesUpdated;
	std::map<Player, EntityInfo_t> entityInfo;
	vgui::EditablePanel *freezeInfoPanel;
	double lastEntityUpdate;
	vgui::HPanel specguiPanel;
	vgui::HPanel topPanel;

	void InitHud();

	ConVar *display;
	ConCommand *display_reload_settings;
	ConVar *display_threshold;
	ConVar *enabled;
	static void ReloadSettings();
	static void ToggleDisplay(IConVar *var, const char *pOldValue, float flOldValue);
};

extern AntiFreeze *g_AntiFreeze;