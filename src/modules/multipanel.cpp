/*
 *  multipanel.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "multipanel.h"

#include "cbase.h"
#include "convar.h"
#include "game/client/iviewport.h"
#include "iclientmode.h"
#include "vgui/IVGui.h"
#include "../vgui_controls/gameconsoledialog.h"
#include "viewport_panel_names.h"

#include "../common.h"
#include "../ifaces.h"

MultiPanel::MultiPanel() {
	consoleDialog = nullptr;
	scoreboardPanel = vgui::INVALID_PANEL;

	console = new ConVar("statusspec_multipanel_console", "0", FCVAR_NONE, "displays a console in the HUD", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<MultiPanel>()->ToggleConsole(var, pOldValue, flOldValue); });
	scoreboard = new ConVar("statusspec_multipanel_scoreboard", "0", FCVAR_NONE, "displays the scoreboard", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<MultiPanel>()->ToggleScoreboard(var, pOldValue, flOldValue); });
}

bool MultiPanel::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", g_ModuleManager->GetModuleName<MultiPanel>().c_str());

		ready = false;
	}

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface vgui::IVGui for module %s not available!\n", g_ModuleManager->GetModuleName<MultiPanel>().c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", g_ModuleManager->GetModuleName<MultiPanel>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<MultiPanel>().c_str());
	}

	return ready;
}

void MultiPanel::InitHud() {
	if (!consoleDialog) {
		consoleDialog = new CGameConsoleDialog();
	}

	if (scoreboardPanel == vgui::INVALID_PANEL) {
		IClientMode *clientMode = Interfaces::GetClientMode();

		if (clientMode) {
			vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

			if (viewport) {
				vgui::VPANEL vpanel = viewport->GetVPanel();
				int children = g_pVGuiPanel->GetChildCount(vpanel);

				for (int i = 0; i < children; i++) {
					vgui::VPANEL child = g_pVGuiPanel->GetChild(vpanel, i);

					if (strcmp(g_pVGuiPanel->GetName(child), PANEL_SCOREBOARD) == 0) {
						scoreboardPanel = g_pVGui->PanelToHandle(child);

						break;
					}
				}
			}
		}
	}
}

void MultiPanel::ToggleConsole(IConVar *var, const char *pOldValue, float flOldValue) {
	bool enabled = console->GetBool();

	if (!consoleDialog) {
		InitHud();
	}

	if (consoleDialog) {
		if (enabled) {
			consoleDialog->Activate();
		}
		else {
			consoleDialog->Hide();
		}
	}
}

void MultiPanel::ToggleScoreboard(IConVar *var, const char *pOldValue, float flOldValue) {
	if (scoreboardPanel == vgui::INVALID_PANEL) {
		InitHud();
	}

	if (scoreboardPanel != vgui::INVALID_PANEL) {
		IClientMode *clientMode = Interfaces::GetClientMode();

		if (clientMode) {
			IViewPort *viewport = dynamic_cast<IViewPort *>(Interfaces::GetClientMode()->GetViewport());

			if (viewport) {
				viewport->ShowPanel(PANEL_SCOREBOARD, true);
				viewport->ShowPanel(PANEL_SCOREBOARD, false);
			}
		}

		g_pVGuiPanel->SetVisible(g_pVGui->HandleToPanel(scoreboardPanel), scoreboard->GetBool());
	}
}