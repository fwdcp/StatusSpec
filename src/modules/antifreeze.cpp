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
	frameHook = 0;
	freezeInfoPanel = nullptr;
	lastEntityUpdate = Plat_FloatTime();
	performLayoutCommand = new KeyValues("Command", "Command", "performlayout");
	specguiPanel = vgui::INVALID_PANEL;

	display = new ConVar("statusspec_antifreeze_display", "0", FCVAR_NONE, "enables display of an info panel when a freeze is detected", [](IConVar *var, const char *pOldValue, float flOldValue) { g_AntiFreeze->ToggleDisplay(var, pOldValue, flOldValue); });
	display_reload_settings = new ConCommand("statusspec_antifreeze_display_reload_settings", []() { g_AntiFreeze->ReloadSettings(); }, "reload settings for the freeze info panel from the resource file", FCVAR_NONE);
	display_threshold = new ConVar("statusspec_antifreeze_display_threshold", "1", FCVAR_NONE, "the time of a freeze (in seconds) before the info panel is displayed");
	enabled = new ConVar("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable antifreeze (forces the spectator GUI to refresh)", [](IConVar *var, const char *pOldValue, float flOldValue) { g_AntiFreeze->ToggleEnabled(var, pOldValue, flOldValue); });
}

void AntiFreeze::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		if (display->GetBool()) {
			lastEntityUpdate = Plat_FloatTime();
		}

		if (enabled->GetBool()) {
			GetSpecGUI();

			if (specguiPanel != vgui::INVALID_PANEL) {
				vgui::VPANEL specguiVPanel = g_pVGui->HandleToPanel(specguiPanel);

				g_pVGuiPanel->SendMessage(specguiVPanel, performLayoutCommand, specguiVPanel);
			}
		}
	}
	else if (curStage == FRAME_START) {
		if (display->GetBool() && Interfaces::pEngineClient->IsInGame()) {
			double freezeTime = Plat_FloatTime() - lastEntityUpdate;

			if (freezeTime >= display_threshold->GetFloat()) {
				int seconds = int(floor(freezeTime)) % 60;
				int minutes = floor(freezeTime / 60);

				char *formattedTime = new char[16];
				V_snprintf(formattedTime, 15, "%i:%02i", minutes, seconds);

				if (freezeInfoPanel) {
					freezeInfoPanel->SetDialogVariable("time", formattedTime);
					freezeInfoPanel->SetVisible(true);
				}
			}
			else {
				if (freezeInfoPanel) {
					freezeInfoPanel->SetVisible(false);
				}
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

void AntiFreeze::GetSpecGUI() {
	if (specguiPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
				vgui::VPANEL panel = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

				if (strcmp(g_pVGuiPanel->GetName(panel), "specgui") == 0) {
					specguiPanel = g_pVGui->PanelToHandle(panel);
				}
			}
		}
	}
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
	if (display->GetBool()) {
		InitHud();

		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &AntiFreeze::FrameHook), true);
		}

		if (freezeInfoPanel) {
			freezeInfoPanel->SetEnabled(true);
			freezeInfoPanel->SetVisible(false);
		}
	}
	else {
		if (!enabled->GetBool() && frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}

		if (freezeInfoPanel) {
			freezeInfoPanel->SetEnabled(false);
			freezeInfoPanel->SetVisible(false);
		}
	}
}

void AntiFreeze::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &AntiFreeze::FrameHook), true);
		}
	}
	else {
		if (!display->GetBool() && frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}