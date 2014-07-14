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

AntiFreeze::AntiFreeze() {
	entitiesUpdated = false;
	lastEntityUpdate = Plat_FloatTime();
	specguiPanel = vgui::INVALID_PANEL;
	topPanel = vgui::INVALID_PANEL;

	enabled = new ConVar("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable antifreeze (forces the spectator GUI to refresh)");
}

bool AntiFreeze::IsEnabled() {
	return enabled->GetBool();
}

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
		g_pVGuiPanel->SendMessage(g_pVGui->HandleToPanel(specguiPanel), PERFORM_LAYOUT_COMMAND, g_pVGui->HandleToPanel(specguiPanel));
	}
}

void AntiFreeze::ProcessEntity(IClientEntity *entity) {
	int index = entity->entindex();

	if (Interfaces::GetGameResources()->IsConnected(index) && Interfaces::GetGameResources()->GetTeam(index) != TEAM_UNASSIGNED && Interfaces::GetGameResources()->GetTeam(index) != TEAM_SPECTATOR) {
		Vector origin = entity->GetAbsOrigin();
		QAngle angles = entity->GetAbsAngles();

		if (entityInfo.find(index) == entityInfo.end()) {
			entitiesUpdated = true;
		}
		else if (entityInfo[index].origin != origin || entityInfo[index].angles != angles) {
			entitiesUpdated = true;
		}

		entityInfo[index].origin = origin;
		entityInfo[index].angles = angles;
	}
}

void AntiFreeze::PostEntityUpdate() {
	if (entitiesUpdated) {
		lastEntityUpdate = Plat_FloatTime();
	}
	else if (Interfaces::pEngineClient->IsInGame()) {
		DevMsg("Last entity update %f seconds ago.\n", Plat_FloatTime() - lastEntityUpdate);
	}

	entitiesUpdated = false;
}