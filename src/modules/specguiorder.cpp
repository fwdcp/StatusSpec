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

inline bool SortPlayers(int a, int b) {
	IClientEntity *first = Interfaces::pClientEntityList->GetClientEntity(a);

	if (!Player::CheckPlayer(first)) {
		return true;
	}

	IClientEntity *second = Interfaces::pClientEntityList->GetClientEntity(b);

	if (!Player::CheckPlayer(second)) {
		return false;
	}

	static std::array<TFClassType, 10> classes = { TFClass_Unknown, TFClass_Scout, TFClass_Soldier, TFClass_Pyro, TFClass_DemoMan, TFClass_Heavy, TFClass_Engineer, TFClass_Medic, TFClass_Sniper, TFClass_Spy };

	TFClassType firstClass = Player::GetClass(first);
	TFClassType secondClass = Player::GetClass(second);

	return std::distance(classes.begin(), std::find(classes.begin(), classes.end(), firstClass)) < std::distance(classes.begin(), std::find(classes.begin(), classes.end(), secondClass));
}

SpecGUIOrder::SpecGUIOrder() {
	enabled = new ConVar("statusspec_specguiorder_enabled", "0", FCVAR_NONE, "enable ordering of spec GUI");

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
				IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

				if (entity == nullptr || !Interfaces::GetGameResources()->IsConnected(i)) {
					continue;
				}

				if (strcmp(playerName, Interfaces::GetGameResources()->GetPlayerName(i)) == 0) {
					playerPanels[originPanelName] = i;

					break;
				}
			}
		}
	}
}

bool SpecGUIOrder::SetPosOverride(vgui::VPANEL vguiPanel, int &x, int &y) {
	std::string panelName = g_pVGuiPanel->GetName(vguiPanel);

	if (panelName.substr(0, 11).compare("playerpanel") == 0) {
		int player = playerPanels[panelName];
		IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(player);

		if (!Player::CheckPlayer(entity)) {
			return false;
		}

		TFTeam team = Player::GetTeam(entity);

		if (team == TFTeam_Red) {
			int position = std::distance(redPlayers.begin(), std::find(redPlayers.begin(), redPlayers.end(), player));

			int baseX = specguiSettings->FindKey("specgui")->GetInt("team2_player_base_offset_x");
			int baseY = specguiSettings->FindKey("specgui")->GetInt("team2_player_base_y");
			int deltaX = specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_x");
			int deltaY = specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_y");

			x = g_pVGuiSchemeManager->GetProportionalScaledValue(baseX + (position * deltaX));
			y = g_pVGuiSchemeManager->GetProportionalScaledValue(baseY + (position * deltaY));

			return true;
		}
		else if (team == TFTeam_Blue) {
			int position = std::distance(bluPlayers.begin(), std::find(bluPlayers.begin(), bluPlayers.end(), player));

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
	if (!Player::CheckPlayer(entity)) {
		return;
	}

	TFTeam team = Player::GetTeam(entity);

	if (team == TFTeam_Red) {
		redPlayers.push_back(entity->entindex());
	}
	else if (team == TFTeam_Blue) {
		bluPlayers.push_back(entity->entindex());
	}
}

void SpecGUIOrder::PostEntityUpdate() {
	bluPlayers.sort(SortPlayers);
	redPlayers.sort(SortPlayers);
}