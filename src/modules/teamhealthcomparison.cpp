/*
*  teamhealthcomparison.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "teamhealthcomparison.h"

TeamHealthComparison::TeamHealthComparison() {
	frameHook = 0;

	enabled = new ConVar("statusspec_teamhealthcomparison_enabled", "0", FCVAR_NONE, "enable team health comparison", [](IConVar *var, const char *pOldValue, float flOldValue) { g_TeamHealthComparison->ToggleEnabled(var, pOldValue, flOldValue); });
	reload_settings = new ConCommand("statusspec_teamhealthcomparison_reload_settings", []() { g_TeamHealthComparison->ReloadSettings(); }, "reload settings for the team health comparison HUD from the resource file", FCVAR_NONE);
}

void TeamHealthComparison::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		teamHealthAggregate.clear();

		for (int i = 1; i <= MAX_PLAYERS; i++) {
			Player player = i;

			if (!player) {
				continue;
			}

			if (!player.IsAlive()) {
				continue;
			}

			teamHealthAggregate[player.GetTeam()] += player.GetHealth();
		}

		Paint();
	}

	RETURN_META(MRES_IGNORED);
}

void TeamHealthComparison::InitHUD() {
	if (panels.find("TeamHealthComparison") == panels.end()) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			vgui::EditablePanel *teamHealthComparisonPanel = new vgui::EditablePanel(viewport, "TeamHealthComparison");
			panels["TeamHealthComparison"] = teamHealthComparisonPanel;

			teamHealthComparisonPanel->LoadControlSettings("Resource/UI/TeamHealthComparison.res");
		}
	}
}

void TeamHealthComparison::Paint() {
	panels["TeamHealthComparison"]->SetVisible(true);

	if (teamHealthAggregate[TFTeam_Red] != 0 || teamHealthAggregate[TFTeam_Blue] != 0) {
		panels["TeamHealthComparison"]->SetVisible(true);

		int totalHealth = teamHealthAggregate[TFTeam_Red] + teamHealthAggregate[TFTeam_Blue];

		float bluPercentage = float(teamHealthAggregate[TFTeam_Blue]) / float(totalHealth);
		float redPercentage = float(teamHealthAggregate[TFTeam_Red]) / float(totalHealth);

		((vgui::EditablePanel *) panels["TeamHealthComparison"])->SetDialogVariable("blupercentage", bluPercentage * 100.0f);
		((vgui::EditablePanel *) panels["TeamHealthComparison"])->SetDialogVariable("redpercentage", redPercentage * 100.0f);
	}
	else {
		panels["TeamHealthComparison"]->SetVisible(false);

		((vgui::EditablePanel *) panels["TeamHealthComparison"])->SetDialogVariable("blupercentage", 0);
		((vgui::EditablePanel *) panels["TeamHealthComparison"])->SetDialogVariable("redpercentage", 0);
	}

	((vgui::EditablePanel *) panels["TeamHealthComparison"])->SetDialogVariable("redhealth", teamHealthAggregate[TFTeam_Red]);
	((vgui::EditablePanel *) panels["TeamHealthComparison"])->SetDialogVariable("bluhealth", teamHealthAggregate[TFTeam_Blue]);
}

void TeamHealthComparison::ReloadSettings() {
	if (panels.find("TeamHealthComparison") != panels.end()) {
		((vgui::EditablePanel *) panels["TeamHealthComparison"])->LoadControlSettings("Resource/UI/TeamHealthComparison.res");
	}
}

void TeamHealthComparison::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		InitHUD();

		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &TeamHealthComparison::FrameHook), true);
		}

		if (panels.find("TeamHealthComparison") != panels.end()) {
			panels["TeamHealthComparison"]->SetEnabled(true);
			panels["TeamHealthComparison"]->SetVisible(true);
		}
	}
	else {
		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}

		if (panels.find("TeamHealthComparison") != panels.end()) {
			panels["TeamHealthComparison"]->SetEnabled(false);
			panels["TeamHealthComparison"]->SetVisible(false);
		}
	}
}