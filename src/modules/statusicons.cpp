/*
 *  statusicons.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusicons.h"

#include "convar.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "tier3/tier3.h"
#include "vgui/IPanel.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

StatusIcons::StatusIcons(std::string name) : Module(name) {
	frameHook = 0;

	delta_x = new ConVar("statusspec_statusicons_delta_x", "15", FCVAR_NONE, "change in the x direction for each icon");
	delta_y = new ConVar("statusspec_statusicons_delta_y", "0", FCVAR_NONE, "change in the y direction for each icon");
	enabled = new ConVar("statusspec_statusicons_enabled", "0", FCVAR_NONE, "enable status icons", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<StatusIcons>("Status Icons")->ToggleEnabled(var, pOldValue, flOldValue); });
}

bool StatusIcons::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface vgui::IVGui for module %s not available!\n", name.c_str());

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

	if (!Player::conditionsRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player condition retrieval for module %s not available!\n", name.c_str());

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

	return ready;
}

void StatusIcons::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		if (Interfaces::GetClientMode() && Interfaces::GetClientMode()->GetViewport()) {
			vgui::VPANEL viewport = Interfaces::GetClientMode()->GetViewport()->GetVPanel();

			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport); i++) {
				vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport, i);

				if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
						vgui::VPANEL playerPanel = g_pVGuiPanel->GetChild(specgui, i);

						if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
							for (int i = 0; i < g_pVGuiPanel->GetChildCount(playerPanel); i++) {
								vgui::VPANEL statusIconsVPanel = g_pVGuiPanel->GetChild(playerPanel, i);

								if (strcmp(g_pVGuiPanel->GetName(statusIconsVPanel), "StatusIcons") == 0) {
									vgui::EditablePanel *statusIcons = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(statusIconsVPanel, "ClientDLL"));

									if (statusIcons) {
										statusIcons->SetEnabled(true);
										statusIcons->SetVisible(true);

										DisplayIcons(playerPanel);
									}

									break;
								}
							}
						}
					}

					break;
				}
			}
		}
	}
}

void StatusIcons::ClearIcons(vgui::VPANEL statusIconsPanel) {
	while (g_pVGuiPanel->GetChildCount(statusIconsPanel) != 0) {
		vgui::VPANEL child = g_pVGuiPanel->GetChild(statusIconsPanel, 0);
		g_pVGuiPanel->DeletePanel(child);
	}
}

void StatusIcons::DisableHUD() {
	if (Interfaces::GetClientMode() && Interfaces::GetClientMode()->GetViewport()) {
		vgui::VPANEL viewport = Interfaces::GetClientMode()->GetViewport()->GetVPanel();

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport); i++) {
			vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport, i);

			if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
				for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
					vgui::VPANEL playerPanel = g_pVGuiPanel->GetChild(specgui, i);

					if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
						for (int i = 0; i < g_pVGuiPanel->GetChildCount(playerPanel); i++) {
							vgui::VPANEL statusIconsVPanel = g_pVGuiPanel->GetChild(playerPanel, i);

							if (strcmp(g_pVGuiPanel->GetName(statusIconsVPanel), "StatusIcons") == 0) {
								ClearIcons(statusIconsVPanel);

								break;
							}
						}
					}
				}

				break;
			}
		}
	}
}

void StatusIcons::DisplayIcon(vgui::EditablePanel *panel, const char *iconTexture) {
	if (panel) {
		int offset = panel->GetChildCount();

		vgui::EditablePanel *iconContainer = new vgui::EditablePanel(panel, "StatusIcon");
		vgui::ImagePanel *icon = new vgui::ImagePanel(iconContainer, "StatusIconImage");

		iconContainer->LoadControlSettings("Resource/UI/StatusIcon.res");

		iconContainer->SetEnabled(true);
		iconContainer->SetVisible(true);

		int deltaX = g_pVGuiSchemeManager->GetProportionalScaledValue(delta_x->GetInt());
		int deltaY = g_pVGuiSchemeManager->GetProportionalScaledValue(delta_y->GetInt());

		iconContainer->SetPos(offset * deltaX, offset * deltaY);

		icon->SetEnabled(true);
		icon->SetVisible(true);

		icon->SetImage(GetVGUITexturePath(iconTexture).c_str());
	}
}

void StatusIcons::DisplayIcons(vgui::VPANEL playerPanel) {
	if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
		vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(playerPanel, "ClientDLL"));

		if (panel) {
			KeyValues *dialogVariables = panel->GetDialogVariables();

			if (dialogVariables) {
				const char *name = dialogVariables->GetString("playername");

				for (Player player : Player::Iterable()) {
					if (player.GetName().compare(name) == 0) {
						for (int i = 0; i < g_pVGuiPanel->GetChildCount(playerPanel); i++) {
							vgui::VPANEL statusIconsVPanel = g_pVGuiPanel->GetChild(playerPanel, i);

							if (strcmp(g_pVGuiPanel->GetName(statusIconsVPanel), "StatusIcons") == 0) {
								ClearIcons(statusIconsVPanel);

								vgui::EditablePanel *statusIcons = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(statusIconsVPanel, "ClientDLL"));

								if (statusIcons) {
									TFTeam team = player.GetTeam();

									if (player.CheckCondition(TFCond_Ubercharged)) {
										DisplayIcon(statusIcons, TEXTURE_UBERCHARGE);
									}

									if (player.CheckCondition(TFCond_Kritzkrieged)) {
										DisplayIcon(statusIcons, TEXTURE_CRITBOOST);
									}

									if (player.CheckCondition(TFCond_MegaHeal)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_MEGAHEALRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_MEGAHEALBLU);
										}
									}

									if (player.CheckCondition(TFCond_UberBulletResist)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_RESISTSHIELDRED);
											DisplayIcon(statusIcons, TEXTURE_BULLETRESISTRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_RESISTSHIELDBLU);
											DisplayIcon(statusIcons, TEXTURE_BULLETRESISTBLU);
										}
									}
									else if (player.CheckCondition(TFCond_SmallBulletResist)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_BULLETRESISTRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_BULLETRESISTBLU);
										}
									}

									if (player.CheckCondition(TFCond_UberBlastResist)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_RESISTSHIELDRED);
											DisplayIcon(statusIcons, TEXTURE_BLASTRESISTRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_RESISTSHIELDBLU);
											DisplayIcon(statusIcons, TEXTURE_BLASTRESISTBLU);
										}
									}
									else if (player.CheckCondition(TFCond_SmallBlastResist)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_BLASTRESISTRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_BLASTRESISTBLU);
										}
									}

									if (player.CheckCondition(TFCond_UberFireResist)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_RESISTSHIELDRED);
											DisplayIcon(statusIcons, TEXTURE_FIRERESISTRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_RESISTSHIELDBLU);
											DisplayIcon(statusIcons, TEXTURE_FIRERESISTBLU);
										}
									}
									else if (player.CheckCondition(TFCond_SmallFireResist)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_FIRERESISTRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_FIRERESISTBLU);
										}
									}

									if (player.CheckCondition(TFCond_Buffed)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_BUFFBANNERRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_BUFFBANNERBLU);
										}
									}

									if (player.CheckCondition(TFCond_DefenseBuffed)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_BATTALIONSBACKUPRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_BATTALIONSBACKUPBLU);
										}
									}

									if (player.CheckCondition(TFCond_RegenBuffed)) {
										if (team == TFTeam_Red) {
											DisplayIcon(statusIcons, TEXTURE_CONCHERORRED);
										}
										else if (team == TFTeam_Blue) {
											DisplayIcon(statusIcons, TEXTURE_CONCHERORBLU);
										}
									}

									if (player.CheckCondition(TFCond_Jarated)) {
										DisplayIcon(statusIcons, TEXTURE_JARATE);
									}

									if (player.CheckCondition(TFCond_Milked)) {
										DisplayIcon(statusIcons, TEXTURE_MADMILK);
									}

									if (player.CheckCondition(TFCond_MarkedForDeath) || player.CheckCondition(TFCond_MarkedForDeathSilent)) {
										DisplayIcon(statusIcons, TEXTURE_MARKFORDEATH);
									}

									if (player.CheckCondition(TFCond_Bleeding)) {
										DisplayIcon(statusIcons, TEXTURE_BLEEDING);
									}

									if (player.CheckCondition(TFCond_OnFire)) {
										DisplayIcon(statusIcons, TEXTURE_FIRE);
									}
								}

								break;
							}
						}

						break;
					}
				}
			}
		}
	}
}

void StatusIcons::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &StatusIcons::FrameHook), true);
		}
	}
	else {
		DisableHUD();

		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}