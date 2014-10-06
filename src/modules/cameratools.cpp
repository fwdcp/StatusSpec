/*
*  cameratools.cpp
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#include "cameratools.h"

inline bool IsInteger(const std::string &s) {
	if (s.empty() || !isdigit(s[0])) return false;

	char *p;
	strtoull(s.c_str(), &p, 10);

	return (*p == 0);
}

CameraTools::CameraTools() {
	spec_player = new ConCommand("statusspec_cameratools_spec_player", CameraTools::SpecPlayer, "spec a certain player", FCVAR_NONE);
	spec_pos = new ConCommand("statusspec_cameratools_spec_pos", CameraTools::SpecPosition, "spec a certain camera position", FCVAR_NONE);
}

void CameraTools::PreEntityUpdate() {
	bluPlayers.clear();
	redPlayers.clear();
}

void CameraTools::ProcessEntity(IClientEntity *entity) {
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

void CameraTools::PostEntityUpdate() {
	std::sort(bluPlayers.begin(), bluPlayers.end());
	std::sort(redPlayers.begin(), redPlayers.end());
}

void CameraTools::SpecPlayer(const CCommand &command) {
	if (command.ArgC() >= 3 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2))) {
		if (atoi(command.Arg(1)) == TFTeam_Blue) {
			int player = atoi(command.Arg(2));

			if (player < 0 || player >= g_CameraTools->bluPlayers.size()) {
				Warning("Must specify a valid player position.\n");

				return;
			}

			Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), g_CameraTools->bluPlayers[player]->entindex());
		}
		else if (atoi(command.Arg(1)) == TFTeam_Red) {
			int player = atoi(command.Arg(2));

			if (player < 0 || player >= g_CameraTools->redPlayers.size()) {
				Warning("Must specify a valid player position.\n");

				return;
			}

			Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), g_CameraTools->redPlayers[player]->entindex());
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_player <team> <position>\n");

		return;
	}
}

void CameraTools::SpecPosition(const CCommand &command) {
	if (command.ArgC() >= 6 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2)) && IsInteger(command.Arg(3)) && IsInteger(command.Arg(4)) && IsInteger(command.Arg(5))) {
		HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

		hltvcamera->m_nCameraMode = OBS_MODE_FIXED;
		hltvcamera->m_iCameraMan = 0;
		hltvcamera->m_vCamOrigin.x = atoi(command.Arg(1));
		hltvcamera->m_vCamOrigin.y = atoi(command.Arg(2));
		hltvcamera->m_vCamOrigin.z = atoi(command.Arg(3));
		hltvcamera->m_aCamAngle.x = atoi(command.Arg(4));
		hltvcamera->m_aCamAngle.y = atoi(command.Arg(5));
		hltvcamera->m_iTraget1 = 0;
		hltvcamera->m_iTraget2 = 0;
		hltvcamera->m_flLastAngleUpdateTime = Interfaces::pPlayerInfoManager->GetGlobalVars()->realtime;

		Interfaces::pEngineClient->SetViewAngles(hltvcamera->m_aCamAngle);
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_pos <x> <y> <z> <yaw> <pitch>\n");

		return;
	}
}