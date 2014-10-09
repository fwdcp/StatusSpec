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
	freezeInfoPanel = nullptr;
	lastEntityUpdate = Plat_FloatTime();
	specguiPanel = vgui::INVALID_PANEL;
	topPanel = vgui::INVALID_PANEL;

	display = new ConVar("statusspec_antifreeze_display", "0", FCVAR_NONE, "enables display of an info panel when a freeze is detected", [](IConVar *var, const char *pOldValue, float flOldValue) { g_AntiFreeze->ToggleDisplay(var, pOldValue, flOldValue); });
	display_reload_settings = new ConCommand("statusspec_antifreeze_display_reload_settings", []() { g_AntiFreeze->ReloadSettings(); }, "reload settings for the freeze info panel from the resource file", FCVAR_NONE);
	display_threshold = new ConVar("statusspec_antifreeze_display_threshold", "1", FCVAR_NONE, "the time of a freeze (in seconds) before the info panel is displayed");
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
	Player player = entity;

	if (player && player.GetTeam() != TFTeam_Unassigned && player.GetTeam() != TFTeam_Spectator) {
		Vector origin = player->GetAbsOrigin();
		QAngle angles = player->GetAbsAngles();

		if (entityInfo.find(player) == entityInfo.end()) {
			entitiesUpdated = true;
		}
		else if (entityInfo[player].origin != origin || entityInfo[player].angles != angles) {
			entitiesUpdated = true;
		}

		entityInfo[player].origin = origin;
		entityInfo[player].angles = angles;
	}
}

void AntiFreeze::PostEntityUpdate() {
	if (entitiesUpdated) {
		lastEntityUpdate = Plat_FloatTime();

		if (freezeInfoPanel) {
			freezeInfoPanel->SetVisible(false);
		}
	}
	else if (Interfaces::pEngineClient->IsInGame()) {
		float freezeTime = Plat_FloatTime() - lastEntityUpdate;

		if (display->GetBool() && freezeTime >= display_threshold->GetFloat()) {
			int seconds = int(floor(freezeTime)) % 60;
			int minutes = floor(freezeTime / 60);

			char *formattedTime = new char[16];
			V_snprintf(formattedTime, 15, "%i:%02i", minutes, seconds);

			if (freezeInfoPanel) {
				freezeInfoPanel->SetDialogVariable("time", formattedTime);
				freezeInfoPanel->SetVisible(true);
			}
		}
	}

	entitiesUpdated = false;
}

void AntiFreeze::InitHud() {
	if (!freezeInfoPanel) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			freezeInfoPanel = new vgui::EditablePanel(viewport, "FreezeInfo");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), freezeInfoPanel);

			freezeInfoPanel->LoadControlSettings("Resource/UI/FreezeInfo.res");

			freezeInfoPanel->SetVisible(false);
		}
	}
}

void AntiFreeze::ReloadSettings() {
	if (freezeInfoPanel) {
		freezeInfoPanel->LoadControlSettings("Resource/UI/FreezeInfo.res");
	}
}

void AntiFreeze::ToggleDisplay(IConVar *var, const char *pOldValue, float flOldValue) {
	bool enabled = display->GetBool();

	if (enabled) {
		InitHud();
	}

	if (freezeInfoPanel) {
		freezeInfoPanel->SetEnabled(enabled);
		freezeInfoPanel->SetVisible(enabled);
	}
}