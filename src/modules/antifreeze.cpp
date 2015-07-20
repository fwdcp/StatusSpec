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

#include "cbase.h"
#include "convar.h"
#include "iclientmode.h"
#include "tier3/tier3.h"
#include "toolframework/ienginetool.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../ifaces.h"

class AntiFreeze::Panel : public vgui::Panel {
public:
	Panel(vgui::Panel *parent, const char *panelName);

	virtual void OnTick();
};

AntiFreeze::AntiFreeze() {
	panel = nullptr;

	enabled = new ConVar("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable antifreeze (forces the spectator GUI to refresh)", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<AntiFreeze>()->ToggleEnabled(var, pOldValue, flOldValue); });
}

bool AntiFreeze::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for module %s not available!\n", g_ModuleManager->GetModuleName<AntiFreeze>().c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", g_ModuleManager->GetModuleName<AntiFreeze>().c_str());

		ready = false;
	}

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface vgui::IVGui for module %s not available!\n", g_ModuleManager->GetModuleName<AntiFreeze>().c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", g_ModuleManager->GetModuleName<AntiFreeze>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<AntiFreeze>().c_str());
	}

	return ready;
}

void AntiFreeze::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			try {
				vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

				if (viewport) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
						vgui::VPANEL childPanel = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

						if (strcmp(g_pVGuiPanel->GetName(childPanel), "specgui") == 0) {
							panel = new Panel(viewport, "AntiFreeze");
							panel->SetParent(childPanel);

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

AntiFreeze::Panel::Panel(vgui::Panel *parent, const char *panelName) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());
}

void AntiFreeze::Panel::OnTick() {
	Interfaces::pEngineTool->ForceSend();
	Interfaces::pEngineTool->ForceUpdateDuringPause();

	g_pVGuiPanel->GetPanel(GetVParent(), GAME_PANEL_MODULE)->OnCommand("performlayout");
}