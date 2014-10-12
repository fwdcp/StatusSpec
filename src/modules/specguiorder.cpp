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
	frameHook = 0;
	setPosHook = 0;
	specguiSettings = new KeyValues("Resource/UI/SpectatorTournament.res");
	specguiSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/spectatortournament.res", "mod");

	enabled = new ConVar("statusspec_specguiorder_enabled", "0", FCVAR_NONE, "enable ordering of spec GUI", [](IConVar *var, const char *pOldValue, float flOldValue) { g_SpecGUIOrder->ToggleEnabled(var, pOldValue, flOldValue); });
	reverse_blu = new ConVar("statusspec_specguiorder_reverse_blu", "0", FCVAR_NONE, "reverse order for BLU players");
	reverse_red = new ConVar("statusspec_specguiorder_reverse_red", "0", FCVAR_NONE, "reverse order for RED players");
}

void SpecGUIOrder::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		bluPlayers.clear();
		redPlayers.clear();

		for (int i = 1; i <= MAX_PLAYERS; i++) {
			Player player = i;

			if (!player) {
				continue;
			}

			TFTeam team = player.GetTeam();

			if (team == TFTeam_Red) {
				redPlayers.push_back(player);
			}
			else if (team == TFTeam_Blue) {
				bluPlayers.push_back(player);
			}
		}

		bluPlayers.sort();
		redPlayers.sort();
	}

	RETURN_META(MRES_IGNORED);
}

void SpecGUIOrder::SetPosOverride(vgui::VPANEL vguiPanel, int x, int y) {
	if (strcmp(g_pVGuiPanel->GetClassName(vguiPanel), "CTFPlayerPanel") == 0) {
		vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(vguiPanel, "ClientDLL"));

		if (panel) {
			KeyValues *dialogVariables = panel->GetDialogVariables();

			if (dialogVariables) {
				const char *name = dialogVariables->GetString("playername");

				for (int i = 1; i <= MAX_PLAYERS; i++) {
					Player player = i;

					if (player && strcmp(player.GetName(), name) == 0) {
						TFTeam team = player.GetTeam();

						if (team == TFTeam_Red) {
							int position;

							if (!reverse_red->GetBool()) {
								position = std::distance(redPlayers.begin(), std::find(redPlayers.begin(), redPlayers.end(), player));
							}
							else {
								position = std::distance(redPlayers.rbegin(), std::find(redPlayers.rbegin(), redPlayers.rend(), player));
							}

							int baseX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_base_offset_x"));
							int baseY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_base_y"));
							int deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_x"));
							int deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_y"));

							int newX = baseX + (position * deltaX);
							int newY = baseY + (position * deltaY);

							RETURN_META_NEWPARAMS(MRES_HANDLED, &IPanel::SetPos, (vguiPanel, newX, newY));
						}
						else if (team == TFTeam_Blue) {
							int position;

							if (!reverse_blu->GetBool()) {
								position = std::distance(bluPlayers.begin(), std::find(bluPlayers.begin(), bluPlayers.end(), player));
							}
							else {
								position = std::distance(bluPlayers.rbegin(), std::find(bluPlayers.rbegin(), bluPlayers.rend(), player));
							}

							int baseX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_base_offset_x"));
							int baseY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_base_y"));
							int deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_x"));
							int deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_y"));

							int newX = baseX + (position * deltaX);
							int newY = baseY + (position * deltaY);

							RETURN_META_NEWPARAMS(MRES_HANDLED, &IPanel::SetPos, (vguiPanel, newX, newY));
						}
					}
				}
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

void SpecGUIOrder::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &SpecGUIOrder::FrameHook), true);
		}

		if (!setPosHook) {
			setPosHook = Funcs::AddHook_IPanel_SetPos(g_pVGuiPanel, SH_MEMBER(this, &SpecGUIOrder::SetPosOverride), false);
		}
	}
	else {
		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}

		if (setPosHook) {
			if (Funcs::RemoveHook(setPosHook)) {
				setPosHook = 0;
			}
		}
	}
}