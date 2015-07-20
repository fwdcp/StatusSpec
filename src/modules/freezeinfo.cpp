/*
 *  freezeinfo.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "freezeinfo.h"

#include "cbase.h"
#include "convar.h"
#include "iclientmode.h"
#include "tier3/tier3.h"
#include "toolframework/ienginetool.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"

class FreezeInfo::Panel : public vgui::EditablePanel {
public:
	Panel(vgui::Panel *parent, const char *panelName);
	virtual ~Panel();

	virtual void OnTick();

	void SetDisplayThreshold(float threshold);
private:
	void PostEntityPacketReceivedHook();

	float lastUpdate;
	int postEntityPacketReceivedHook;
	float threshold;
};

FreezeInfo::FreezeInfo() {
	panel = nullptr;

	enabled = new ConVar("statusspec_freezeinfo_enabled", "0", FCVAR_NONE, "enables display of an info panel when a freeze is detected", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<FreezeInfo>()->ToggleEnabled(var, pOldValue, flOldValue); });
	reload_settings = new ConCommand("statusspec_freezeinfo_reload_settings", []() { g_ModuleManager->GetModule<FreezeInfo>()->ReloadSettings(); }, "reload settings for the freeze info panel from the resource file", FCVAR_NONE);
	threshold = new ConVar("statusspec_freezeinfo_threshold", "1", FCVAR_NONE, "the time of a freeze (in seconds) before the info panel is displayed", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<FreezeInfo>()->ChangeThreshold(var, pOldValue, flOldValue); });
}

bool FreezeInfo::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<FreezeInfo>().c_str());

		ready = false;
	}

	if (!Interfaces::pPrediction) {
		PRINT_TAG();
		Warning("Required interface IPrediction for module %s not available!\n", g_ModuleManager->GetModuleName<FreezeInfo>().c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", g_ModuleManager->GetModuleName<FreezeInfo>().c_str());

		ready = false;
	}

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface vgui::IVGui for module %s not available!\n", g_ModuleManager->GetModuleName<FreezeInfo>().c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", g_ModuleManager->GetModuleName<FreezeInfo>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<FreezeInfo>().c_str());
	}

	return ready;
}

void FreezeInfo::ChangeThreshold(IConVar *var, const char *pOldValue, float flOldValue) {
	if (panel) {
		panel->SetDisplayThreshold(threshold->GetFloat());
	}
}

void FreezeInfo::ReloadSettings() {
	if (panel) {
		panel->LoadControlSettings("Resource/UI/FreezeInfo.res");
	}
}

void FreezeInfo::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			try {
				vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

				if (viewport) {
					panel = new Panel(viewport, "FreezeInfo");
					panel->SetDisplayThreshold(threshold->GetFloat());
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

		if (panel) {
			panel->SetEnabled(true);
		}
	}
	else {
		if (panel) {
			delete panel;
			panel = nullptr;
		}
	}
}

FreezeInfo::Panel::Panel(vgui::Panel *parent, const char *panelName) : vgui::EditablePanel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());
	postEntityPacketReceivedHook = Funcs::AddHook_IPrediction_PostEntityPacketReceived(Interfaces::pPrediction, SH_MEMBER(this, &FreezeInfo::Panel::PostEntityPacketReceivedHook), true);

	LoadControlSettings("Resource/UI/FreezeInfo.res");
}

FreezeInfo::Panel::~Panel() {
	Funcs::RemoveHook(postEntityPacketReceivedHook);
}

void FreezeInfo::Panel::OnTick() {
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

void FreezeInfo::Panel::SetDisplayThreshold(float displayThreshold) {
	threshold = displayThreshold;
}

void FreezeInfo::Panel::PostEntityPacketReceivedHook() {
	lastUpdate = Interfaces::pEngineTool->HostTime();
}