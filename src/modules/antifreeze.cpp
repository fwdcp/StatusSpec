/*
 *  antifreeze.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "antifreeze.h"

AntiFreeze::AntiFreeze() {
	bluTime = 0;
	bluTimerPanel = vgui::INVALID_PANEL;
	entitiesUpdated = false;
	frameHook = 0;
	freezeInfoPanel = nullptr;
	lastEntityUpdate = Plat_FloatTime();
	mainTime = 0;
	mainTimerPanel = vgui::INVALID_PANEL;
	performLayoutCommand = new KeyValues("Command", "Command", "performlayout");
	redTime = 0;
	redTimerPanel = vgui::INVALID_PANEL;
	specguiPanel = vgui::INVALID_PANEL;
	stopwatchTime = 0;
	stopwatchTimerPanel = vgui::INVALID_PANEL;

	display = new ConVar("statusspec_antifreeze_display", "0", FCVAR_NONE, "enables display of an info panel when a freeze is detected", [](IConVar *var, const char *pOldValue, float flOldValue) { g_AntiFreeze->ToggleDisplay(var, pOldValue, flOldValue); });
	display_reload_settings = new ConCommand("statusspec_antifreeze_display_reload_settings", []() { g_AntiFreeze->ReloadSettings(); }, "reload settings for the freeze info panel from the resource file", FCVAR_NONE);
	display_threshold = new ConVar("statusspec_antifreeze_display_threshold", "1", FCVAR_NONE, "the time of a freeze (in seconds) before the info panel is displayed");
	enabled = new ConVar("statusspec_antifreeze_enabled", "0", FCVAR_NONE, "enable antifreeze (forces the spectator GUI to refresh)", [](IConVar *var, const char *pOldValue, float flOldValue) { g_AntiFreeze->ToggleEnabled(var, pOldValue, flOldValue); });
	timers = new ConVar("statusspec_antifreeze_timers", "0", FCVAR_NONE, "enable forcing of timers to right values", [](IConVar *var, const char *pOldValue, float flOldValue) { g_AntiFreeze->ToggleTimers(var, pOldValue, flOldValue); });
}

void AntiFreeze::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		if (display->GetBool()) {
			lastEntityUpdate = Plat_FloatTime();
		}

		if (enabled->GetBool()) {
			GetSpecGUI();

			if (specguiPanel != vgui::INVALID_PANEL) {
				vgui::VPANEL specguiVPanel = g_pVGui->HandleToPanel(specguiPanel);

				g_pVGuiPanel->SendMessage(specguiVPanel, performLayoutCommand, specguiVPanel);
			}
		}

		if (timers->GetBool()) {
			int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

			for (int i = 0; i < maxEntity; i++) {
				IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

				if (entity) {
					if (Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFObjectiveResource")) {
						IClientEntity *mainEntity = Interfaces::pClientEntityList->GetClientEntity(*MAKE_PTR(int *, entity, Entities::pCTFObjectiveResource__m_iTimerToShowInHUD));

						if (mainEntity && Entities::CheckClassBaseclass(mainEntity->GetClientClass(), "DT_TeamRoundTimer")) {
							C_TeamRoundTimer *mainTimer = dynamic_cast<C_TeamRoundTimer *>(mainEntity);

							if (mainTimer) {
								mainTime = mainTimer->GetTimeRemaining();
							}
						}

						IClientEntity *stopwatchEntity = Interfaces::pClientEntityList->GetClientEntity(*MAKE_PTR(int *, entity, Entities::pCTFObjectiveResource__m_iStopWatchTimer));

						if (stopwatchEntity && Entities::CheckClassBaseclass(stopwatchEntity->GetClientClass(), "DT_TeamRoundTimer")) {
							C_TeamRoundTimer *stopwatchTimer = dynamic_cast<C_TeamRoundTimer *>(stopwatchEntity);

							if (stopwatchTimer) {
								stopwatchTime = stopwatchTimer->GetTimeRemaining();
							}
						}
					}
					else if (Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFGameRulesProxy")) {
						IClientEntity *redEntity = Interfaces::pClientEntityList->GetClientEntity(ENTITY_INDEX_FROM_ENTITY_OFFSET(entity, Entities::pCTFGameRulesProxy__m_hRedKothTimer));

						if (redEntity && Entities::CheckClassBaseclass(redEntity->GetClientClass(), "DT_TeamRoundTimer")) {
							C_TeamRoundTimer *redTimer = dynamic_cast<C_TeamRoundTimer *>(redEntity);

							if (redTimer) {
								redTime = redTimer->GetTimeRemaining();
							}
						}

						IClientEntity *bluEntity = Interfaces::pClientEntityList->GetClientEntity(ENTITY_INDEX_FROM_ENTITY_OFFSET(entity, Entities::pCTFGameRulesProxy__m_hBlueKothTimer));

						if (bluEntity && Entities::CheckClassBaseclass(bluEntity->GetClientClass(), "DT_TeamRoundTimer")) {
							C_TeamRoundTimer *bluTimer = dynamic_cast<C_TeamRoundTimer *>(bluEntity);

							if (bluTimer) {
								bluTime = bluTimer->GetTimeRemaining();
							}
						}
					}
				}
			}
		}
	}
	else if (curStage == FRAME_START) {
		if (display->GetBool() && Interfaces::pEngineClient->IsInGame()) {
			double freezeTime = Plat_FloatTime() - lastEntityUpdate;

			if (freezeTime >= display_threshold->GetFloat()) {
				int seconds = int(floor(freezeTime)) % 60;
				int minutes = int(floor(freezeTime)) / 60;

				char *formattedTime = new char[16];
				V_snprintf(formattedTime, 15, "%i:%02i", minutes, seconds);

				if (freezeInfoPanel) {
					freezeInfoPanel->SetDialogVariable("time", formattedTime);
					freezeInfoPanel->SetVisible(true);
				}
			}
			else {
				if (freezeInfoPanel) {
					freezeInfoPanel->SetVisible(false);
				}
			}
		}

		if (timers->GetBool() && Interfaces::pEngineClient->IsInGame()) {
			GetTimers();

			InitTimers(true);

			if (mainTimerPanel != vgui::INVALID_PANEL) {
				vgui::VPANEL mainTimerVPanel = g_pVGui->HandleToPanel(mainTimerPanel);

				for (int i = 0; i < g_pVGuiPanel->GetChildCount(mainTimerVPanel); i++) {
					vgui::VPANEL mainTimerValue = g_pVGuiPanel->GetChild(mainTimerVPanel, i);

					if (strcmp(g_pVGuiPanel->GetName(mainTimerValue), REAL_TIME_NAME) == 0) {
						int mainSeconds = int(floor(mainTime)) % 60;
						int mainMinutes = int(floor(mainTime)) / 60;

						char *mainFormattedTime = new char[16];
						V_snprintf(mainFormattedTime, 15, "%i:%02i", mainMinutes, mainSeconds);

						g_pVGuiPanel->SendMessage(mainTimerValue, new KeyValues("SetText", "text", mainFormattedTime), mainTimerVPanel);

						break;
					}
				}
			}

			if (stopwatchTimerPanel != vgui::INVALID_PANEL) {
				vgui::VPANEL stopwatchTimerVPanel = g_pVGui->HandleToPanel(stopwatchTimerPanel);

				for (int i = 0; i < g_pVGuiPanel->GetChildCount(stopwatchTimerVPanel); i++) {
					vgui::VPANEL stopwatchTimerValue = g_pVGuiPanel->GetChild(stopwatchTimerVPanel, i);

					if (strcmp(g_pVGuiPanel->GetName(stopwatchTimerValue), REAL_TIME_NAME) == 0) {
						int stopwatchSeconds = int(floor(stopwatchTime)) % 60;
						int stopwatchMinutes = int(floor(stopwatchTime)) / 60;

						char *stopwatchFormattedTime = new char[16];
						V_snprintf(stopwatchFormattedTime, 15, "%i:%02i", stopwatchMinutes, stopwatchSeconds);

						g_pVGuiPanel->SendMessage(stopwatchTimerValue, new KeyValues("SetText", "text", stopwatchFormattedTime), stopwatchTimerVPanel);

						break;
					}
				}
			}

			if (redTimerPanel == vgui::INVALID_PANEL) {
				vgui::VPANEL redTimerVPanel = g_pVGui->HandleToPanel(redTimerPanel);

				for (int i = 0; i < g_pVGuiPanel->GetChildCount(redTimerVPanel); i++) {
					vgui::VPANEL redTimerValue = g_pVGuiPanel->GetChild(redTimerVPanel, i);

					if (strcmp(g_pVGuiPanel->GetName(redTimerValue), REAL_TIME_NAME) == 0) {
						int redSeconds = int(floor(redTime)) % 60;
						int redMinutes = int(floor(redTime)) / 60;

						char *redFormattedTime = new char[16];
						V_snprintf(redFormattedTime, 15, "%i:%02i", redMinutes, redSeconds);

						g_pVGuiPanel->SendMessage(redTimerValue, new KeyValues("SetText", "text", redFormattedTime), redTimerVPanel);

						break;
					}
				}
			}

			if (bluTimerPanel == vgui::INVALID_PANEL) {
				vgui::VPANEL bluTimerVPanel = g_pVGui->HandleToPanel(bluTimerPanel);

				for (int i = 0; i < g_pVGuiPanel->GetChildCount(bluTimerVPanel); i++) {
					vgui::VPANEL bluTimerValue = g_pVGuiPanel->GetChild(bluTimerVPanel, i);

					if (strcmp(g_pVGuiPanel->GetName(bluTimerValue), REAL_TIME_NAME) == 0) {
						int bluSeconds = int(floor(bluTime)) % 60;
						int bluMinutes = int(floor(bluTime)) / 60;

						char *bluFormattedTime = new char[16];
						V_snprintf(bluFormattedTime, 15, "%i:%02i", bluMinutes, bluSeconds);

						g_pVGuiPanel->SendMessage(bluTimerValue, new KeyValues("SetText", "text", bluFormattedTime), bluTimerVPanel);

						break;
					}
				}
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

void AntiFreeze::GetSpecGUI() {
	if (specguiPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
				vgui::VPANEL panel = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

				if (strcmp(g_pVGuiPanel->GetName(panel), "specgui") == 0) {
					specguiPanel = g_pVGui->PanelToHandle(panel);

					break;
				}
			}
		}
	}
}

void AntiFreeze::GetTimers() {
	if (mainTimerPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
				vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

				if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
						vgui::VPANEL objectiveStatus = g_pVGuiPanel->GetChild(specgui, i);

						if (strcmp(g_pVGuiPanel->GetName(objectiveStatus), "HudObjectiveStatus") == 0) {
							for (int i = 0; i < g_pVGuiPanel->GetChildCount(objectiveStatus); i++) {
								vgui::VPANEL objectiveStatusTimer = g_pVGuiPanel->GetChild(objectiveStatus, i);

								if (strcmp(g_pVGuiPanel->GetName(objectiveStatusTimer), "ObjectiveStatusTimePanel") == 0) {
									mainTimerPanel = g_pVGui->PanelToHandle(objectiveStatusTimer);

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

	if (stopwatchTimerPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
				vgui::VPANEL stopwatch = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

				if (strcmp(g_pVGuiPanel->GetName(stopwatch), "HudStopWatch") == 0) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(stopwatch); i++) {
						vgui::VPANEL stopwatchTimer = g_pVGuiPanel->GetChild(stopwatch, i);

						if (strcmp(g_pVGuiPanel->GetName(stopwatchTimer), "ObjectiveStatusTimePanel") == 0) {
							stopwatchTimerPanel = g_pVGui->PanelToHandle(stopwatchTimer);

							break;
						}
					}

					break;
				}
			}
		}
	}

	if (redTimerPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
				vgui::VPANEL kothTime = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

				if (strcmp(g_pVGuiPanel->GetName(kothTime), "HudKothTimeStatus") == 0) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(kothTime); i++) {
						vgui::VPANEL redTimer = g_pVGuiPanel->GetChild(kothTime, i);

						if (strcmp(g_pVGuiPanel->GetName(redTimer), "RedTimer") == 0) {
							redTimerPanel = g_pVGui->PanelToHandle(redTimer);

							break;
						}
					}

					break;
				}
			}
		}
	}

	if (bluTimerPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport->GetVPanel()); i++) {
				vgui::VPANEL kothTime = g_pVGuiPanel->GetChild(viewport->GetVPanel(), i);

				if (strcmp(g_pVGuiPanel->GetName(kothTime), "HudKothTimeStatus") == 0) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(kothTime); i++) {
						vgui::VPANEL bluTimer = g_pVGuiPanel->GetChild(kothTime, i);

						if (strcmp(g_pVGuiPanel->GetName(bluTimer), "BlueTimer") == 0) {
							bluTimerPanel = g_pVGui->PanelToHandle(bluTimer);

							break;
						}
					}

					break;
				}
			}
		}
	}
}

void AntiFreeze::InitHUD() {
	if (!freezeInfoPanel) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			freezeInfoPanel = new vgui::EditablePanel(viewport, "FreezeInfo");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), freezeInfoPanel);

			freezeInfoPanel->LoadControlSettings("Resource/UI/FreezeInfo.res");

			freezeInfoPanel->SetVisible(false);
		}
	}
}

void AntiFreeze::InitTimers(bool moduleTime) {
	GetTimers();

	if (mainTimerPanel != vgui::INVALID_PANEL) {
		vgui::VPANEL mainTimerVPanel = g_pVGui->HandleToPanel(mainTimerPanel);

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(mainTimerVPanel); i++) {
			vgui::VPANEL mainTimerValue = g_pVGuiPanel->GetChild(mainTimerVPanel, i);

			if (strcmp(g_pVGuiPanel->GetName(mainTimerValue), REAL_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(mainTimerValue, moduleTime);
				g_pVGuiPanel->SetVisible(mainTimerValue, moduleTime);
			}
			else if (strcmp(g_pVGuiPanel->GetName(mainTimerValue), REGULAR_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(mainTimerValue, !moduleTime);
				g_pVGuiPanel->SetVisible(mainTimerValue, !moduleTime);
			}
		}
	}

	if (stopwatchTimerPanel != vgui::INVALID_PANEL) {
		vgui::VPANEL stopwatchTimerVPanel = g_pVGui->HandleToPanel(stopwatchTimerPanel);

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(stopwatchTimerVPanel); i++) {
			vgui::VPANEL stopwatchTimerValue = g_pVGuiPanel->GetChild(stopwatchTimerVPanel, i);

			if (strcmp(g_pVGuiPanel->GetName(stopwatchTimerValue), REAL_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(stopwatchTimerValue, moduleTime);
				g_pVGuiPanel->SetVisible(stopwatchTimerValue, moduleTime);
			}
			else if (strcmp(g_pVGuiPanel->GetName(stopwatchTimerValue), REGULAR_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(stopwatchTimerValue, !moduleTime);
				g_pVGuiPanel->SetVisible(stopwatchTimerValue, !moduleTime);
			}
		}
	}

	if (redTimerPanel == vgui::INVALID_PANEL) {
		vgui::VPANEL redTimerVPanel = g_pVGui->HandleToPanel(redTimerPanel);

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(redTimerVPanel); i++) {
			vgui::VPANEL redTimerValue = g_pVGuiPanel->GetChild(redTimerVPanel, i);

			if (strcmp(g_pVGuiPanel->GetName(redTimerValue), REAL_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(redTimerValue, moduleTime);
				g_pVGuiPanel->SetVisible(redTimerValue, moduleTime);
			}
			else if (strcmp(g_pVGuiPanel->GetName(redTimerValue), REGULAR_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(redTimerValue, !moduleTime);
				g_pVGuiPanel->SetVisible(redTimerValue, !moduleTime);
			}
		}
	}

	if (bluTimerPanel == vgui::INVALID_PANEL) {
		vgui::VPANEL bluTimerVPanel = g_pVGui->HandleToPanel(bluTimerPanel);

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(bluTimerVPanel); i++) {
			vgui::VPANEL bluTimerValue = g_pVGuiPanel->GetChild(bluTimerVPanel, i);

			if (strcmp(g_pVGuiPanel->GetName(bluTimerValue), REAL_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(bluTimerValue, moduleTime);
				g_pVGuiPanel->SetVisible(bluTimerValue, moduleTime);
			}
			else if (strcmp(g_pVGuiPanel->GetName(bluTimerValue), REGULAR_TIME_NAME) == 0) {
				g_pVGuiPanel->SetEnabled(bluTimerValue, !moduleTime);
				g_pVGuiPanel->SetVisible(bluTimerValue, !moduleTime);
			}
		}
	}
}

void AntiFreeze::ReloadSettings() {
	if (freezeInfoPanel) {
		freezeInfoPanel->LoadControlSettings("Resource/UI/FreezeInfo.res");
	}
}

void AntiFreeze::ToggleDisplay(IConVar *var, const char *pOldValue, float flOldValue) {
	if (display->GetBool()) {
		InitHUD();

		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &AntiFreeze::FrameHook), true);
		}

		if (freezeInfoPanel) {
			freezeInfoPanel->SetEnabled(true);
			freezeInfoPanel->SetVisible(false);
		}
	}
	else {
		if (!enabled->GetBool() && !timers->GetBool() && frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}

		if (freezeInfoPanel) {
			freezeInfoPanel->SetEnabled(false);
			freezeInfoPanel->SetVisible(false);
		}
	}
}

void AntiFreeze::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &AntiFreeze::FrameHook), true);
		}
	}
	else {
		if (!display->GetBool() && !timers->GetBool() && frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}

void AntiFreeze::ToggleTimers(IConVar *var, const char *pOldValue, float flOldValue) {
	if (timers->GetBool()) {
		InitTimers(true);

		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &AntiFreeze::FrameHook), true);
		}
	}
	else {
		InitTimers(false);

		if (!display->GetBool() && !enabled->GetBool() && frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}