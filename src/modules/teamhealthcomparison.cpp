/*
 *  teamhealthcomparison.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "teamhealthcomparison.h"

#include "cbase.h"
#include "convar.h"
#include "iclientmode.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class TeamHealthComparison::Panel : public vgui::EditablePanel {
public:
	Panel(vgui::Panel *parent, const char *panelName);

	virtual void OnTick();
private:
	std::map<TFTeam, int> teamHealthAggregate;
};

TeamHealthComparison::TeamHealthComparison() {
	panel = nullptr;

	enabled = new ConVar("statusspec_teamhealthcomparison_enabled", "0", FCVAR_NONE, "enable team health comparison", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<TeamHealthComparison>()->ToggleEnabled(var, pOldValue, flOldValue); });
	reload_settings = new ConCommand("statusspec_teamhealthcomparison_reload_settings", []() { g_ModuleManager->GetModule<TeamHealthComparison>()->ReloadSettings(); }, "reload settings for the team health comparison HUD from the resource file", FCVAR_NONE);
}

bool TeamHealthComparison::CheckDependencies() {
	bool ready = true;

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface IVGui for module %s not available!\n", g_ModuleManager->GetModuleName<TeamHealthComparison>().c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", g_ModuleManager->GetModuleName<TeamHealthComparison>().c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", g_ModuleManager->GetModuleName<TeamHealthComparison>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<TeamHealthComparison>().c_str());
	}

	return ready;
}

void TeamHealthComparison::ReloadSettings() {
	if (panel) {
		panel->LoadControlSettings("Resource/UI/TeamHealthComparison.res");
	}
}

void TeamHealthComparison::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			try {
				vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

				if (viewport) {
					panel = new Panel(viewport, "TeamHealthComparison");
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

TeamHealthComparison::Panel::Panel(vgui::Panel *parent, const char *panelName) : vgui::EditablePanel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());

	LoadControlSettings("Resource/UI/TeamHealthComparison.res");
}

void TeamHealthComparison::Panel::OnTick() {
	teamHealthAggregate.clear();

	for (Player player : Player::Iterable()) {
		if (player) {
			if (!player.IsAlive()) {
				continue;
			}

			teamHealthAggregate[player.GetTeam()] += player.GetHealth();
		}
	}

	float redPercentage = 0.0f;
	float bluPercentage = 0.0f;

	if (teamHealthAggregate[TFTeam_Red] != 0 || teamHealthAggregate[TFTeam_Blue] != 0) {
		int totalHealth = teamHealthAggregate[TFTeam_Red] + teamHealthAggregate[TFTeam_Blue];

		redPercentage = (float(teamHealthAggregate[TFTeam_Red]) / float(totalHealth)) * 100.0f;
		bluPercentage = (float(teamHealthAggregate[TFTeam_Blue]) / float(totalHealth)) * 100.0f;
	}

	SetDialogVariable("redpercentage", redPercentage);
	SetDialogVariable("blupercentage", bluPercentage);

	SetDialogVariable("redhealth", teamHealthAggregate[TFTeam_Red]);
	SetDialogVariable("bluhealth", teamHealthAggregate[TFTeam_Blue]);
}