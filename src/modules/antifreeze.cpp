/*
 *  antifreeze.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "antifreeze.h"

AntiFreeze *g_AntiFreeze;

ConVar AntiFreeze::enabled("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable antifreeze (forces the spectator GUI to refresh)", AntiFreeze::ToggleState);
vgui::HPanel AntiFreeze::specguiPanel = vgui::INVALID_PANEL;
vgui::HPanel AntiFreeze::topPanel = vgui::INVALID_PANEL;

void AntiFreeze::Paint(vgui::VPANEL vguiPanel) {
	if (specguiPanel == vgui::INVALID_PANEL || topPanel == vgui::INVALID_PANEL) {
		std::string name = g_pVGuiPanel->GetName(vguiPanel);

		if (name.compare(SPEC_GUI_NAME) == 0) {
			specguiPanel = g_pVGui->PanelToHandle(vguiPanel);
		}
		else if (name.compare(TOP_PANEL_NAME) == 0) {
			topPanel = g_pVGui->PanelToHandle(vguiPanel);
		}
	}

	if (g_pVGui->HandleToPanel(topPanel) == vguiPanel && specguiPanel != vgui::INVALID_PANEL) {
		g_pVGuiPanel->SendMessage(g_pVGui->HandleToPanel(specguiPanel), new KeyValues("Command", "Command", "performlayout"), g_pVGui->HandleToPanel(specguiPanel));
	}
}

void AntiFreeze::ToggleState(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled.GetBool() && !g_AntiFreeze) {
		g_AntiFreeze = new AntiFreeze();
	}
	else if (!enabled.GetBool() && g_AntiFreeze) {
		delete g_AntiFreeze;
	}
}