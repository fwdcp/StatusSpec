/*
 *  teamoverrides.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "teamoverrides.h"

#include "cbase.h"
#include "convar.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "tier3/tier3.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../funcs.h"

TeamOverrides::TeamOverrides(std::string name) : Module(name) {
	sendMessageHook = 0;

	enabled = new ConVar("statusspec_teamoverrides_enabled", "0", FCVAR_NONE, "enable team overrides", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<TeamOverrides>("Team Overrides")->ToggleEnabled(var, pOldValue, flOldValue); });
	name_blu = new ConVar("statusspec_teamoverrides_name_blu", "BLU", FCVAR_NONE, "BLU team name");
	name_red = new ConVar("statusspec_teamoverrides_name_red", "RED", FCVAR_NONE, "RED team name");
	names = new ConVar("statusspec_teamoverrides_names", "0", FCVAR_NONE, "enable overrides for team names");
	score_blu = new ConVar("statusspec_teamoverrides_score_blu", "0", FCVAR_NONE, "BLU team score");
	score_red = new ConVar("statusspec_teamoverrides_score_red", "0", FCVAR_NONE, "RED team score");
	scores = new ConVar("statusspec_teamoverrides_scores", "0", FCVAR_NONE, "enable overrides for team scores");
	switch_teams = new ConCommand("statusspec_teamoverrides_switch_teams", []() { g_ModuleManager->GetModule<TeamOverrides>("Team Overrides")->SwitchTeams(); }, "switch names and scores for both teams", FCVAR_NONE);
}

bool TeamOverrides::CheckDependencies(std::string name) {
	bool ready = true;

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", name.c_str());

		ready = false;
	}

	return ready;
}

void TeamOverrides::SendMessageOverride(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	if (strcmp(params->GetName(), "DialogVariables") == 0) {
		if (names->GetBool()) {
			if (strcmp(params->GetString("blueteamname"), "") != 0) {
				params->SetString("blueteamname", name_blu->GetString());
			}

			if (strcmp(params->GetString("redteamname"), "") != 0) {
				params->SetString("redteamname", name_red->GetString());
			}
		}

		if (scores->GetBool()) {
			if (strcmp(params->GetString("blueteamscore"), "") != 0) {
				params->SetInt("blueteamscore", score_blu->GetInt());
			}

			if (strcmp(params->GetString("redteamscore"), "") != 0) {
				params->SetInt("redteamscore", score_red->GetInt());
			}
		}

		RETURN_META(MRES_HANDLED);
	}

	RETURN_META(MRES_IGNORED);
}

void TeamOverrides::SwitchTeams() {
	std::string newBluName = name_red->GetString();
	std::string newRedName = name_blu->GetString();
	int newBluScore = score_red->GetInt();
	int newRedScore = score_blu->GetInt();

	name_blu->SetValue(newBluName.c_str());
	name_red->SetValue(newRedName.c_str());
	score_blu->SetValue(newBluScore);
	score_red->SetValue(newRedScore);
}

void TeamOverrides::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!sendMessageHook) {
			sendMessageHook = Funcs::AddHook_IPanel_SendMessage(g_pVGuiPanel, SH_MEMBER(this, &TeamOverrides::SendMessageOverride), false);
		}
	}
	else {
		if (sendMessageHook) {
			if (Funcs::RemoveHook(sendMessageHook)) {
				sendMessageHook = 0;
			}
		}
	}
}