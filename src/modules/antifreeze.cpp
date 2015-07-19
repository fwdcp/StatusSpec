/*
 *  antifreeze.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "antifreeze.h"

#include "convar.h"
#include "iclientmode.h"
#include "tier3/tier3.h"
#include "toolframework/ienginetool.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"

class AntiFreeze::DisplayPanel : public vgui::EditablePanel {
public:
	DisplayPanel(vgui::Panel *parent, const char *panelName);
	virtual ~DisplayPanel();

	virtual void OnTick();

	void SetDisplayThreshold(float threshold);
private:
	void PostEntityPacketReceivedHook();

	float lastUpdate;
	int postEntityPacketReceivedHook;
	float threshold;
};

class AntiFreeze::RefreshPanel : public vgui::Panel {
public:
	RefreshPanel(vgui::Panel *parent, const char *panelName);

	virtual void OnTick();
};

AntiFreeze::AntiFreeze(std::string name) : Module(name) {
	displayPanel = nullptr;
	refreshPanel = nullptr;

	display = new ConVar("statusspec_antifreeze_display", "0", FCVAR_NONE, "enables display of an info panel when a freeze is detected", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<AntiFreeze>("AntiFreeze")->ToggleDisplay(var, pOldValue, flOldValue); });
	display_reload_settings = new ConCommand("statusspec_antifreeze_display_reload_settings", []() { g_ModuleManager->GetModule<AntiFreeze>("AntiFreeze")->ReloadSettings(); }, "reload settings for the freeze info panel from the resource file", FCVAR_NONE);
	display_threshold = new ConVar("statusspec_antifreeze_display_threshold", "1", FCVAR_NONE, "the time of a freeze (in seconds) before the info panel is displayed", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<AntiFreeze>("AntiFreeze")->ChangeDisplayThreshold(var, pOldValue, flOldValue); });
	enabled = new ConVar("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable antifreeze (forces the spectator GUI to refresh)", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<AntiFreeze>("AntiFreeze")->ToggleEnabled(var, pOldValue, flOldValue); });
}

bool AntiFreeze::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pPrediction) {
		PRINT_TAG();
		Warning("Required interface IPrediction for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface vgui::IVGui for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", name.c_str());
	}

	return ready;
}

void AntiFreeze::ChangeDisplayThreshold(IConVar *var, const char *pOldValue, float flOldValue) {
	if (displayPanel) {
		displayPanel->SetDisplayThreshold(display_threshold->GetFloat());
	}
}

void AntiFreeze::ReloadSettings() {
	if (displayPanel) {
		displayPanel->LoadControlSettings("Resource/UI/FreezeInfo.res");
	}
}

void AntiFreeze::ToggleDisplay(IConVar *var, const char *pOldValue, float flOldValue) {
	if (display->GetBool()) {
		if (!displayPanel) {
			try {
				vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

				if (viewport) {
					displayPanel = new DisplayPanel(viewport, "FreezeInfo");
					displayPanel->SetDisplayThreshold(display_threshold->GetFloat());
				}
				else {
					Warning("Could not initialize the panel!\n");
					var->SetValue(0);
				}
			}
			catch (bad_pointer) {
				Warning("Could not initialize the panel!\n");
				var->SetValue(0);
			}
		}

		if (displayPanel) {
			displayPanel->SetEnabled(true);
		}
	}
	else {
		if (displayPanel) {
			delete displayPanel;
			displayPanel = nullptr;
		}
	}
}

void AntiFreeze::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!refreshPanel) {
			try {
				vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

				if (viewport) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
						vgui::VPANEL panel = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

						if (strcmp(g_pVGuiPanel->GetName(panel), "specgui") == 0) {
							refreshPanel = new RefreshPanel(viewport, "AntiFreezeRefresh");
							refreshPanel->SetParent(panel);

							return;
						}
					}

					Warning("Could not initialize the panel!\n");
					var->SetValue(0);
				}
				else {
					Warning("Could not initialize the panel!\n");
					var->SetValue(0);
				}
			}
			catch (bad_pointer) {
				Warning("Could not initialize the panel!\n");
				var->SetValue(0);
			}
		}

		if (refreshPanel) {
			refreshPanel->SetEnabled(true);
		}
	}
	else {
		if (refreshPanel) {
			delete refreshPanel;
			refreshPanel = nullptr;
		}
	}
}

AntiFreeze::DisplayPanel::DisplayPanel(vgui::Panel *parent, const char *panelName) : vgui::EditablePanel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());
	postEntityPacketReceivedHook = Funcs::AddHook_IPrediction_PostEntityPacketReceived(Interfaces::pPrediction, SH_MEMBER(this, &AntiFreeze::DisplayPanel::PostEntityPacketReceivedHook), true);

	LoadControlSettings("Resource/UI/FreezeInfo.res");
}

AntiFreeze::DisplayPanel::~DisplayPanel() {
	Funcs::RemoveHook(postEntityPacketReceivedHook);
}

void AntiFreeze::DisplayPanel::OnTick() {
	float interval = Interfaces::pEngineTool->HostTime() - lastUpdate;

	if (interval > threshold) {
		if (!IsVisible()) {
			SetVisible(true);
		}

		int seconds = int(floor(interval)) % 60;
		int minutes = int(floor(interval)) / 60;

		char *formattedTime = new char[16];
		V_snprintf(formattedTime, 15, "%i:%02i", minutes, seconds);

		SetDialogVariable("time", formattedTime);
	}
	else {
		if (IsVisible()) {
			SetVisible(false);
		}
	}
}

void AntiFreeze::DisplayPanel::SetDisplayThreshold(float displayThreshold) {
	threshold = displayThreshold;
}

void AntiFreeze::DisplayPanel::PostEntityPacketReceivedHook() {
	lastUpdate = Interfaces::pEngineTool->HostTime();
}

AntiFreeze::RefreshPanel::RefreshPanel(vgui::Panel *parent, const char *panelName) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());
}

void AntiFreeze::RefreshPanel::OnTick() {
	Interfaces::pEngineTool->ForceSend();
	Interfaces::pEngineTool->ForceUpdateDuringPause();

	g_pVGuiPanel->GetPanel(GetVParent(), GAME_PANEL_MODULE)->OnCommand("performlayout");
}