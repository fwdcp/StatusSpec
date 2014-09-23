/*
 *  teamoverrides.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "teamoverrides.h"

TeamOverrides::TeamOverrides() {
	enabled = new ConVar("statusspec_teamoverrides_enabled", "0", FCVAR_NONE, "enable team overrides");
	name_blu = new ConVar("statusspec_teamoverrides_name_blu", "BLU", FCVAR_NONE, "BLU team name");
	name_red = new ConVar("statusspec_teamoverrides_name_red", "RED", FCVAR_NONE, "RED team name");
	names = new ConVar("statusspec_teamoverrides_names", "0", FCVAR_NONE, "enable overrides for team names");
	score_blu = new ConVar("statusspec_teamoverrides_score_blu", "0", FCVAR_NONE, "BLU team score");
	score_red = new ConVar("statusspec_teamoverrides_score_red", "0", FCVAR_NONE, "RED team score");
	scores = new ConVar("statusspec_teamoverrides_scores", "0", FCVAR_NONE, "enable overrides for team scores");
}

bool TeamOverrides::IsEnabled() {
	return enabled->GetBool();
}

void TeamOverrides::InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
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
				params->SetString("blueteamscore", score_blu->GetString());
			}

			if (strcmp(params->GetString("redteamscore"), "") != 0) {
				params->SetString("redteamscore", score_red->GetString());
			}
		}
	}
}