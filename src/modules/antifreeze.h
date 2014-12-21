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

#define CLIENT_DLL
#define TF_CLIENT_DLL
#define TF_DLL

#include "cbase.h"
#include "convar.h"
#include "icliententity.h"
#include "KeyValues.h"
#include "teamplay_round_timer.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"

#define REAL_TIME_NAME "RealTime"
#define REGULAR_TIME_NAME "TimePanelValue"
#define SPEC_GUI_NAME "specgui"

typedef struct EntityInfo_s {
	Vector origin;
	QAngle angles;
} EntityInfo_t;

class AntiFreeze : public Module {
public:
	AntiFreeze(std::string name);

	static bool CheckDependencies(std::string name);

	void FrameHook(ClientFrameStage_t curStage);
private:
	float bluTime;
	vgui::HPanel bluTimerPanel;
	bool entitiesUpdated;
	int frameHook;
	vgui::EditablePanel *freezeInfoPanel;
	double lastEntityUpdate;
	float mainTime;
	vgui::HPanel mainTimerPanel;
	KeyValues *performLayoutCommand;
	float redTime;
	vgui::HPanel redTimerPanel;
	vgui::HPanel specguiPanel;
	float stopwatchTime;
	vgui::HPanel stopwatchTimerPanel;

	void GetSpecGUI();
	void GetTimers();
	void InitHUD();
	void InitTimers(bool moduleTime);

	ConVar *display;
	ConCommand *display_reload_settings;
	ConVar *display_threshold;
	ConVar *enabled;
	ConVar *timers;
	void ReloadSettings();
	void ToggleDisplay(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void ToggleTimers(IConVar *var, const char *pOldValue, float flOldValue);
};