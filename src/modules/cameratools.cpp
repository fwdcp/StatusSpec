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

#include <thread>

#include "json/json.h"

class C_BaseEntity;
#include "cbase.h"
#include "convar.h"
#include "filesystem.h"
#include "gameeventdefs.h"
#include "globalvars_base.h"
#include "usercmd.h"
#include "hltvcamera.h"
#include "icliententity.h"
#include "iclientmode.h"
#include "in_buttons.h"
#include "KeyValues.h"
#include "shareddefs.h"
#include "tier3/tier3.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../exceptions.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class CameraTools::HLTVCameraOverride : public C_HLTVCamera {
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
	frameHook = 0;
	specguiSettings = new KeyValues("Resource/UI/SpectatorTournament.res");
	specguiSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/spectatortournament.res", "mod");

	killer_follow_enabled = new ConVar("statusspec_cameratools_killer_follow_enabled", "0", FCVAR_NONE, "enables switching to the kill", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->ToggleKillerFollowEnabled(var, pOldValue, flOldValue); });
	spec_player = new ConCommand("statusspec_cameratools_spec_player", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->SpecPlayer(command); }, "spec a certain player", FCVAR_NONE);
	spec_player_alive = new ConVar("statusspec_cameratools_spec_player_alive", "1", FCVAR_NONE, "prevent speccing dead players");
	spec_pos = new ConCommand("statusspec_cameratools_spec_pos", [](const CCommand &command) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->SpecPosition(command); }, "spec a certain camera position", FCVAR_NONE);
	state = new ConVar("statusspec_cameratools_state", "{}", FCVAR_NONE, "JSON of camera tools state", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->ChangeState(var, pOldValue, flOldValue); });
	state_enabled = new ConVar("statusspec_cameratools_state_enabled", "0", FCVAR_NONE, "enable exposure of camera tools state", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<CameraTools>("Camera Tools")->ToggleStateEnabled(var, pOldValue, flOldValue); });
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
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Required interface CGlobalVarsBase for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Funcs::GetFunc_C_HLTVCamera_SetPrimaryTarget();
	}
	catch (bad_pointer) {
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
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", name.c_str());
	}

	try {
		Interfaces::GetHLTVCamera();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires C_HLTVCamera, which cannot be verified at this time!\n", name.c_str());
	}

	return ready;
}

void CameraTools::FireGameEvent(IGameEvent *event) {
	if (killer_follow_enabled->GetBool()) {
		if (strcmp(event->GetName(), GAME_EVENT_PLAYER_DEATH) == 0) {
			Player localPlayer = Interfaces::pEngineClient->GetLocalPlayer();

			if (localPlayer) {
				if (localPlayer.GetObserverMode() == OBS_MODE_FIXED || localPlayer.GetObserverMode() == OBS_MODE_IN_EYE || localPlayer.GetObserverMode() == OBS_MODE_CHASE) {
					Player targetPlayer = localPlayer.GetObserverTarget();

					if (targetPlayer) {
						if (event->GetInt("userid") == targetPlayer->entindex()) {
							Player attacker = event->GetInt("attacker");

							if (attacker) {
								try {
									Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), event->GetInt("attacker"));
								}
								catch (bad_pointer &e) {
									Warning("%s\n", e.what());
								}
							}
						}
					}
				}
			}
		}
	}
}

void CameraTools::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_START) {
		std::thread update(std::bind(&CameraTools::UpdateState, this));
		update.detach();
	}

	RETURN_META(MRES_IGNORED);
}

void CameraTools::UpdateState() {
	std::string currentState;

	if (Interfaces::pEngineClient->IsInGame()) {
		try {
			HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

			Json::Value cameraState;
			cameraState["mode"] = hltvcamera->m_nCameraMode;
			cameraState["camera"] = hltvcamera->m_iCameraMan;
			cameraState["origin"]["x"] = hltvcamera->m_vCamOrigin.x;
			cameraState["origin"]["y"] = hltvcamera->m_vCamOrigin.y;
			cameraState["origin"]["z"] = hltvcamera->m_vCamOrigin.z;
			cameraState["angle"]["x"] = hltvcamera->m_aCamAngle.x;
			cameraState["angle"]["y"] = hltvcamera->m_aCamAngle.y;
			cameraState["angle"]["z"] = hltvcamera->m_aCamAngle.z;
			cameraState["target"][0] = hltvcamera->m_iTraget1;
			cameraState["target"][1] = hltvcamera->m_iTraget2;
			cameraState["fov"] = hltvcamera->m_flFOV;
			cameraState["offset"] = hltvcamera->m_flOffset;
			cameraState["distance"] = hltvcamera->m_flDistance;
			cameraState["lastdistance"] = hltvcamera->m_flLastDistance;
			cameraState["theta"] = hltvcamera->m_flTheta;
			cameraState["phi"] = hltvcamera->m_flPhi;
			cameraState["inertia"] = hltvcamera->m_flInertia;
			cameraState["cmd"]["angle"]["x"] = hltvcamera->m_LastCmd.viewangles.x;
			cameraState["cmd"]["angle"]["y"] = hltvcamera->m_LastCmd.viewangles.y;
			cameraState["cmd"]["angle"]["z"] = hltvcamera->m_LastCmd.viewangles.z;
			cameraState["cmd"]["buttons"]["speed"] = (hltvcamera->m_LastCmd.buttons & IN_SPEED) != 0;
			cameraState["cmd"]["forwardmove"] = hltvcamera->m_LastCmd.forwardmove;
			cameraState["cmd"]["sidemove"] = hltvcamera->m_LastCmd.sidemove;
			cameraState["cmd"]["upmove"] = hltvcamera->m_LastCmd.upmove;
			cameraState["velocity"]["x"] = hltvcamera->m_vecVelocity.x;
			cameraState["velocity"]["y"] = hltvcamera->m_vecVelocity.y;
			cameraState["velocity"]["z"] = hltvcamera->m_vecVelocity.z;

			currentState = Json::FastWriter().write(cameraState);
		}
		catch (bad_pointer &e) {
			Warning("%s\n", e.what());
		}
	}
	else {
		currentState = "{}";
	}

	currentlyUpdating = true;
	state->SetValue(currentState.c_str());
	currentlyUpdating = false;
}

void CameraTools::ChangeState(IConVar *var, const char *pOldValue, float flOldValue) {
	if (!state_enabled->GetBool()) {
		return;
	}

	if (currentlyUpdating) {
		return;
	}

	try {
		HLTVCameraOverride *hltvcamera = (HLTVCameraOverride *)Interfaces::GetHLTVCamera();

		Json::Value newState;
		Json::Reader().parse(state->GetString(), newState);

		if (newState.isMember("camera") && newState["camera"].isInt()) {
			hltvcamera->m_iCameraMan = newState["camera"].asInt();
		}

		if (newState.isMember("origin") && newState["origin"].isObject()) {
			if (newState["origin"].isMember("x") && newState["origin"]["x"].isDouble()) {
				hltvcamera->m_vCamOrigin.x = newState["origin"]["x"].asFloat();
			}

			if (newState["origin"].isMember("y") && newState["origin"]["y"].isDouble()) {
				hltvcamera->m_vCamOrigin.y = newState["origin"]["y"].asFloat();
			}

			if (newState["origin"].isMember("z") && newState["origin"]["z"].isDouble()) {
				hltvcamera->m_vCamOrigin.z = newState["origin"]["z"].asFloat();
			}
		}

		if (newState.isMember("target") && newState["target"].isArray()) {
			if (newState["target"].isValidIndex(1) && newState["target"][1].isInt()) {
				hltvcamera->m_iTraget2 = newState["target"][1].asInt();
			}
		}

		if (newState.isMember("fov") && newState["fov"].isDouble()) {
			hltvcamera->m_flFOV = newState["fov"].asFloat();
		}

		if (newState.isMember("offset") && newState["offset"].isDouble()) {
			hltvcamera->m_flOffset = newState["offset"].asFloat();
		}

		if (newState.isMember("distance") && newState["distance"].isDouble()) {
			hltvcamera->m_flDistance = newState["distance"].asFloat();
		}

		if (newState.isMember("lastdistance") && newState["lastdistance"].isDouble()) {
			hltvcamera->m_flLastDistance = newState["lastdistance"].asFloat();
		}

		if (newState.isMember("theta") && newState["theta"].isDouble()) {
			hltvcamera->m_flTheta = newState["theta"].asFloat();
		}

		if (newState.isMember("phi") && newState["phi"].isDouble()) {
			hltvcamera->m_flPhi = newState["phi"].asFloat();
		}

		if (newState.isMember("inertia") && newState["inertia"].isDouble()) {
			hltvcamera->m_flInertia = newState["inertia"].asFloat();
		}

		if (newState.isMember("cmd") && newState["cmd"].isObject()) {
			if (newState["cmd"].isMember("angle") && newState["cmd"]["angle"].isObject()) {
				if (newState["cmd"]["angle"].isMember("x") && newState["cmd"]["angle"]["x"].isDouble()) {
					hltvcamera->m_LastCmd.viewangles.x = newState["cmd"]["angle"]["x"].asFloat();
				}

				if (newState["cmd"]["angle"].isMember("y") && newState["cmd"]["angle"]["y"].isDouble()) {
					hltvcamera->m_LastCmd.viewangles.y = newState["cmd"]["angle"]["y"].asFloat();
				}

				if (newState["cmd"]["angle"].isMember("z") && newState["cmd"]["angle"]["z"].isDouble()) {
					hltvcamera->m_LastCmd.viewangles.z = newState["cmd"]["angle"]["z"].asFloat();
				}
			}

			if (newState["cmd"].isMember("buttons") && newState["cmd"]["buttons"].isObject()) {
				if (newState["cmd"]["buttons"].isMember("speed") && newState["cmd"]["buttons"]["speed"].isBool()) {
					if (newState["cmd"]["buttons"]["speed"].asBool()) {
						hltvcamera->m_LastCmd.buttons |= IN_SPEED;
					}
					else {
						hltvcamera->m_LastCmd.buttons &= ~(IN_SPEED);
					}
				}
			}

			if (newState["cmd"].isMember("forwardmove") && newState["cmd"]["forwardmove"].isDouble()) {
				hltvcamera->m_LastCmd.forwardmove = newState["cmd"]["forwardmove"].asFloat();
			}

			if (newState["cmd"].isMember("sidemove") && newState["cmd"]["sidemove"].isDouble()) {
				hltvcamera->m_LastCmd.sidemove = newState["cmd"]["sidemove"].asFloat();
			}

			if (newState["cmd"].isMember("upmove") && newState["cmd"]["upmove"].isDouble()) {
				hltvcamera->m_LastCmd.upmove = newState["cmd"]["upmove"].asFloat();
			}
		}

		if (newState.isMember("velocity") && newState["velocity"].isObject()) {
			if (newState["velocity"].isMember("x") && newState["velocity"]["x"].isDouble()) {
				hltvcamera->m_vecVelocity.x = newState["velocity"]["x"].asFloat();
			}

			if (newState["velocity"].isMember("y") && newState["velocity"]["y"].isDouble()) {
				hltvcamera->m_vecVelocity.y = newState["velocity"]["y"].asFloat();
			}

			if (newState["velocity"].isMember("z") && newState["velocity"]["z"].isDouble()) {
				hltvcamera->m_vecVelocity.z = newState["velocity"]["z"].asFloat();
			}
		}

		if (newState.isMember("mode") && newState["mode"].isInt()) {
			Funcs::CallFunc_C_HLTVCamera_SetMode(Interfaces::GetHLTVCamera(), newState["mode"].asInt());
		}

		if (newState.isMember("angle") && newState["angle"].isObject()) {
			QAngle angle;

			if (newState["angle"].isMember("x") && newState["angle"]["x"].isDouble()) {
				angle.x = newState["angle"]["x"].asFloat();
			}
			else {
				angle.x = hltvcamera->m_aCamAngle.x;
			}

			if (newState["angle"].isMember("y") && newState["angle"]["y"].isDouble()) {
				angle.y = newState["angle"]["y"].asFloat();
			}
			else {
				angle.y = hltvcamera->m_aCamAngle.y;
			}

			if (newState["angle"].isMember("z") && newState["angle"]["z"].isDouble()) {
				angle.z = newState["angle"]["z"].asFloat();
			}
			else {
				angle.z = hltvcamera->m_aCamAngle.z;
			}

			Funcs::CallFunc_C_HLTVCamera_SetCameraAngle(Interfaces::GetHLTVCamera(), angle);
		}

		if (newState.isMember("target") && newState["target"].isArray()) {
			if (newState["target"].isValidIndex(0) && newState["target"][0].isInt()) {
				Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), newState["target"][0].asInt());
			}
		}

		std::thread update(std::bind(&CameraTools::UpdateState, this));
		update.detach();
	}
	catch (bad_pointer &e) {
		Warning("%s\n", e.what());
	}
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

											if (player.GetName().compare(name) == 0) {
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
			Warning("%s\n", e.what());
		}
	}
	else if (command.ArgC() == 2 && IsInteger(command.Arg(1))) {
		Player player = atoi(command.Arg(1));

		if (player) {
			if (!spec_player_alive->GetBool() || player.IsAlive()) {
				try {
					Funcs::CallFunc_C_HLTVCamera_SetPrimaryTarget(Interfaces::GetHLTVCamera(), player->entindex());
				}
				catch (bad_pointer &e) {
					Warning("%s\n", e.what());
				}
			}
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_player <team> <position> || statusspec_cameratools_spec_player <index>\n");

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
			Warning("%s\n", e.what());
		}
	}
	else {
		Warning("Usage: statusspec_cameratools_spec_pos <x> <y> <z> <yaw> <pitch>\n");

		return;
	}
}

void CameraTools::ToggleKillerFollowEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (killer_follow_enabled->GetBool()) {
		if (!Interfaces::pGameEventManager->FindListener(this, GAME_EVENT_PLAYER_DEATH)) {
			Interfaces::pGameEventManager->AddListener(this, GAME_EVENT_PLAYER_DEATH, false);
		}
	}
	else {
		if (Interfaces::pGameEventManager->FindListener(this, GAME_EVENT_PLAYER_DEATH)) {
			Interfaces::pGameEventManager->RemoveListener(this);
		}
	}
}

void CameraTools::ToggleStateEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (state_enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &CameraTools::FrameHook), true);
		}
	}
	else {
		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}