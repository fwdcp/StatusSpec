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

#include "cdll_int.h"
#include "vgui/VGUI.h"

#include "../modules.h"

class ConCommand;
class ConVar;
class IConVar;
class KeyValues;

namespace vgui {
	class EditablePanel;
};

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