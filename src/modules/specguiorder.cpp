/*
*  specguiorder.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "specguiorder.h"

SpecGUIOrder::SpecGUIOrder() {
	enabled = new ConVar("statusspec_specguiorder_enabled", "0", FCVAR_NONE, "enable ordering of spec GUI");
	reverse_blu = new ConVar("statusspec_specguiorder_reverse_blu", "0", FCVAR_NONE, "reverse order for BLU players");
	reverse_red = new ConVar("statusspec_specguiorder_reverse_red", "0", FCVAR_NONE, "reverse order for RED players");

	specguiSettings = new KeyValues("Resource/UI/SpectatorTournament.res");
	specguiSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/spectatortournament.res", "mod");
}

bool SpecGUIOrder::IsEnabled() {
	return enabled->GetBool();
}

void SpecGUIOrder::InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	std::string originPanelName = g_pVGuiPanel->GetName(ifromPanel);

	if (originPanelName.substr(0, 11).compare("playerpanel") == 0 && strcmp(params->GetName(), "DialogVariables") == 0) {
		const char *playerName = params->GetString("playername", NULL);

		if (playerName) {
			for (int i = 0; i <= MAX_PLAYERS; i++) {
				Player player = i;

				if (!player) {
					continue;
				}

				if (strcmp(playerName, player.GetName()) == 0) {
					playerPanels[originPanelName] = player;

					break;
				}
			}
		}
	}
}

bool SpecGUIOrder::SetPosOverride(vgui::VPANEL vguiPanel, int &x, int &y) {
	std::string panelName = g_pVGuiPanel->GetName(vguiPanel);

	if (panelName.substr(0, 11).compare("playerpanel") == 0) {
		Player player = playerPanels[panelName];

		if (!player) {
			return false;
		}

		TFTeam team = player.GetTeam();

		if (team == TFTeam_Red) {
			int position;
			
			if (!reverse_red->GetBool()) {
				position = std::distance(redPlayers.begin(), std::find(redPlayers.begin(), redPlayers.end(), player));
			}
			else {
				position = std::distance(redPlayers.rbegin(), std::find(redPlayers.rbegin(), redPlayers.rend(), player));
			}

			int baseX = specguiSettings->FindKey("specgui")->GetInt("team2_player_base_offset_x");
			int baseY = specguiSettings->FindKey("specgui")->GetInt("team2_player_base_y");
			int deltaX = specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_x");
			int deltaY = specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_y");

			x = g_pVGuiSchemeManager->GetProportionalScaledValue(baseX + (position * deltaX));
			y = g_pVGuiSchemeManager->GetProportionalScaledValue(baseY + (position * deltaY));

			return true;
		}
		else if (team == TFTeam_Blue) {
			int position;

			if (!reverse_red->GetBool()) {
				position = std::distance(bluPlayers.begin(), std::find(bluPlayers.begin(), bluPlayers.end(), player));
			}
			else {
				position = std::distance(bluPlayers.rbegin(), std::find(bluPlayers.rbegin(), bluPlayers.rend(), player));
			}

			int baseX = specguiSettings->FindKey("specgui")->GetInt("team1_player_base_offset_x");
			int baseY = specguiSettings->FindKey("specgui")->GetInt("team1_player_base_y");
			int deltaX = specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_x");
			int deltaY = specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_y");

			x = g_pVGuiSchemeManager->GetProportionalScaledValue(baseX + (position * deltaX));
			y = g_pVGuiSchemeManager->GetProportionalScaledValue(baseY + (position * deltaY));

			return true;
		}
	}

	return false;
}

void SpecGUIOrder::PreEntityUpdate() {
	bluPlayers.clear();
	redPlayers.clear();
}

void SpecGUIOrder::ProcessEntity(IClientEntity *entity) {
	Player player = entity;

	if (!player) {
		return;
	}

	TFTeam team = player.GetTeam();

	if (team == TFTeam_Red) {
		redPlayers.push_back(player);
	}
	else if (team == TFTeam_Blue) {
		bluPlayers.push_back(player);
	}
}

void SpecGUIOrder::PostEntityUpdate() {
	bluPlayers.sort();
	redPlayers.sort();
}