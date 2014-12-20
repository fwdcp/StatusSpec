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

class HLTVCameraOverride : public C_HLTVCamera {
public:
	using C_HLTVCamera::m_nCameraMode;
	using C_HLTVCamera::m_iCameraMan;
	using C_HLTVCamera::m_vCamOrigin;
	using C_HLTVCamera::m_aCamAngle;
	using C_HLTVCamera::m_iTraget1;
	using C_HLTVCamera::m_iTraget2;
	using C_HLTVCamera::m_flFOV;
	using C_HLTVCamera::m_flOffset;
	using C_HLTVCamera::m_flDistance;
	using C_HLTVCamera::m_flLastDistance;
	using C_HLTVCamera::m_flTheta;
	using C_HLTVCamera::m_flPhi;
	using C_HLTVCamera::m_flInertia;
	using C_HLTVCamera::m_flLastAngleUpdateTime;
	using C_HLTVCamera::m_bEntityPacketReceived;
	using C_HLTVCamera::m_nNumSpectators;
	using C_HLTVCamera::m_szTitleText;
	using C_HLTVCamera::m_LastCmd;
	using C_HLTVCamera::m_vecVelocity;
};

CameraTools::CameraTools(std::string name) : Module(name) {
	specguiSettings = new KeyValues("Resource/UI/SpectatorTournament.res");
	specguiSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/spectatortournament.res", "mod");

	spec_player = new ConCommand("statusspec_cameratools_spec_player", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->SpecPlayer(command); }, "spec a certain player", FCVAR_NONE);
	spec_player_alive = new ConVar("statusspec_cameratools_spec_player_alive", "1", FCVAR_NONE, "prevent speccing dead players");
	spec_pos = new ConCommand("statusspec_cameratools_spec_pos", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->SpecPosition(command); }, "spec a certain camera position", FCVAR_NONE);
}

bool CameraTools::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Required interface IVEngineClient for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Interfaces::GetGlobalVars();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Required interface CGlobalVarsBase for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Required function C_HLTVCamera::SetPrimaryTarget for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGuiSchemeManager) {
		PRINT_TAG();
		Warning("Required interface vgui::ISchemeManager for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::nameRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player name retrieval for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", name.c_str());
	}

	try {
		Interfaces::GetHLTVCamera();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Module %s requires C_HLTVCamera, which cannot be verified at this time!\n", name.c_str());
	}

	return ready;
}

void CameraTools::SpecPlayer(const CCommand &command) {
	if (command.ArgC() >= 3 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2))) {
		try {
			if (Interfaces::GetClientMode() && Interfaces::GetClientMode()->GetViewport()) {
				vgui::VPANEL viewport = Interfaces::GetClientMode()->GetViewport()->GetVPanel();

				for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport); i++) {
					vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport, i);

					if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
						for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
							vgui::VPANEL playerPanel = g_pVGuiPanel->GetChild(specgui, i);

							if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
								vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(playerPanel, "ClientDLL"));

								if (panel) {
									KeyValues *dialogVariables = panel->GetDialogVariables();

									if (dialogVariables) {
										const char *name = dialogVariables->GetString("playername");

										for (auto iterator = Player::begin(); iterator != Player::end(); ++iterator) {
											Player player = *iterator;

											if (strcmp(player.GetName(), name) == 0) {
												int baseX = 0;
												int baseY = 0;
												int deltaX = 0;
												int deltaY = 0;

												if (player.GetTeam() == TFTeam_Red) {
													if (atoi(command.Arg(1)) != TFTeam_Red) {
														continue;
													}

													baseX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_base_offset_x"));
													baseY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_base_y"));
													deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_x"));
													deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team2_player_delta_y"));
												}
												else if (player.GetTeam() == TFTeam_Blue) {
													if (atoi(command.Arg(1)) != TFTeam_Blue) {
														continue;
													}

													baseX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_base_offset_x"));
													baseY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_base_y"));
													deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_x"));
													deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(specguiSettings->FindKey("specgui")->GetInt("team1_player_delta_y"));
												}

												int position = atoi(command.Arg(2));

												int x, y;
												g_pVGuiPanel->GetPos(playerPanel, x, y);

												int relativeX = x - baseX;
												int relativeY = y - baseY;

												if (deltaX != 0 && relativeX / deltaX != position) {
													continue;
												}
												else if (relativeX != 0 && deltaX == 0) {
													continue;
												}

												if (deltaY != 0 && relativeY / deltaY != position) {
													continue;
												}
												else if (relativeY != 0 && deltaY == 0) {
													continue;
												}

												if (!spec_player_alive->GetBool() || player.IsAlive()) {
													Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), player->entindex());
												}

												return;
											}
										}
									}
								}
							}
						}

						break;
					}
				}
			}

			Warning("No suitable player to switch to found!\n");
		}
		catch (bad_pointer &e) {
			Warning(e.what());
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_player <team> <position>\n");

		return;
	}
}

void CameraTools::SpecPosition(const CCommand &command) {
	if (command.ArgC() >= 6 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2)) && IsInteger(command.Arg(3)) && IsInteger(command.Arg(4)) && IsInteger(command.Arg(5))) {
		try {
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
			hltvcamera->m_flLastAngleUpdateTime = Interfaces::GetGlobalVars()->realtime;

			Interfaces::pEngineClient->SetViewAngles(hltvcamera->m_aCamAngle);
		}
		catch (bad_pointer &e) {
			Warning(e.what());
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_pos <x> <y> <z> <yaw> <pitch>\n");

		return;
	}
}