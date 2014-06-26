/*
 *  playeroutlines.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "playeroutlines.h"

inline float ChangeScale(float currentValue, float currentMin, float currentMax, float newMin, float newMax) {
	float deltaScaler = ((newMax - newMin) / (currentMax - currentMin));
	float newDelta = ((currentValue - currentMin) * deltaScaler);
	float newValue = newMin + newDelta;

	return newValue;
}

inline int ColorRangeRestrict(int color) {
	if (color < 0) return 0;
	else if (color > 255) return 255;
	else return color;
}

inline bool IsInteger(const std::string &s) {
   if (s.empty() || !isdigit(s[0])) return false;

   char *p;
   strtoull(s.c_str(), &p, 10);

   return (*p == 0);
}

PlayerOutlines::PlayerOutlines() {
	topPanel = vgui::INVALID_PANEL;

	colors["blu_low"].color = Color(88, 133, 162);
	colors["blu_low"].command = new ConCommand("statusspec_playeroutlines_color_blu_low", PlayerOutlines::ColorCommand, "the color used for outlines for BLU team players at low health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["blu_medium"].color = Color(88, 133, 162);
	colors["blu_medium"].command = new ConCommand("statusspec_playeroutlines_color_blu_medium", PlayerOutlines::ColorCommand, "the color used for outlines for BLU team players at medium health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["blu_full"].color = Color(88, 133, 162);
	colors["blu_full"].command = new ConCommand("statusspec_playeroutlines_color_blu_full", PlayerOutlines::ColorCommand, "the color used for outlines for BLU team players at full health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["blu_buff"].color = Color(88, 133, 162);
	colors["blu_buff"].command = new ConCommand("statusspec_playeroutlines_color_blu_buff", PlayerOutlines::ColorCommand, "the color used for outlines for BLU team players at max buffed health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["red_low"].color = Color(184, 56, 59);
	colors["red_low"].command = new ConCommand("statusspec_playeroutlines_color_red_low", PlayerOutlines::ColorCommand, "the color used for outlines for RED team players at low health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["red_medium"].color = Color(184, 56, 59);
	colors["red_medium"].command = new ConCommand("statusspec_playeroutlines_color_red_medium", PlayerOutlines::ColorCommand, "the color used for outlines for RED team players at medium health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["red_full"].color = Color(184, 56, 59);
	colors["red_full"].command = new ConCommand("statusspec_playeroutlines_color_red_full", PlayerOutlines::ColorCommand, "the color used for outlines for RED team players at full health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	colors["red_buff"].color = Color(184, 56, 59);
	colors["red_buff"].command = new ConCommand("statusspec_playeroutlines_color_red_buff", PlayerOutlines::ColorCommand, "the color used for outlines for RED team players at max buffed health", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	
	enabled = new ConVar("statusspec_playeroutlines_enabled", "0", FCVAR_NONE, "enable player outlines", PlayerOutlines::ToggleEnabled);
	force_refresh = new ConCommand("statusspec_playeroutlines_force_refresh", PlayerOutlines::ForceRefresh, "force the player outlines to refresh", FCVAR_NONE);
	frequent_override_enabled = new ConVar("statusspec_playeroutlines_frequent_override_enabled", "0", FCVAR_NONE, "enable more frequent player outline overrides (helps stop flickering at cost of performance)");
	health_adjusted_team_colors = new ConVar("statusspec_playeroutlines_health_adjusted_team_colors", "0", FCVAR_NONE, "adjusts team colors depending on health of players");
	team_colors = new ConVar("statusspec_playeroutlines_team_colors", "0", FCVAR_NONE, "override default health-based outline colors with team colors");
}

PlayerOutlines::~PlayerOutlines() {
	enabled->SetValue(0);

	ForceRefresh();
}

bool PlayerOutlines::IsEnabled() {
	return enabled->GetBool();
}

bool PlayerOutlines::IsFrequentOverrideEnabled() {
	return frequent_override_enabled->GetBool();
}

bool PlayerOutlines::GetGlowEffectColorOverride(C_TFPlayer *tfPlayer, float *r, float *g, float *b) {
	if (team_colors->GetBool()) {
		TFTeam team = (TFTeam)*MAKE_PTR(int*, tfPlayer, Entities::pCTFPlayer__m_iTeamNum);
		C_BasePlayer *basePlayer = reinterpret_cast<C_BasePlayer *>(tfPlayer);

		if (basePlayer) {
			if (!health_adjusted_team_colors->GetBool()) {
				if (team == TFTeam_Red) {
					*r = colors["red_full"].color.r() / 255.0f;
					*g = colors["red_full"].color.g() / 255.0f;
					*b = colors["red_full"].color.b() / 255.0f;

					return true;
				}
				else if (team == TFTeam_Blue) {
					*r = colors["blu_full"].color.r() / 255.0f;
					*g = colors["blu_full"].color.g() / 255.0f;
					*b = colors["blu_full"].color.b() / 255.0f;

					return true;
				}
			}
			else {
				C_PlayerResource *playerResource = dynamic_cast<C_PlayerResource *>(Interfaces::GetGameResources());

				if (playerResource) {
					int health = *MAKE_PTR(int*, playerResource, Entities::pCTFPlayerResource__m_iHealth[basePlayer->entindex()]);
					int maxHealth = *MAKE_PTR(int*, playerResource, Entities::pCTFPlayerResource__m_iMaxHealth[basePlayer->entindex()]);

					// CTFPlayerResource isn't giving us proper values so let's calculate it manually
					int maxBuffedHealth = floor((float(maxHealth) / 5.0f) * 1.5f) * 5;

					// calculate this once instead of several times
					float halfHealth = float(maxHealth) / 2.0f;

					if (team == TFTeam_Red) {
						float red;
						float green;
						float blue;

						if (health < 0) {
							// this should never happen

							red = colors["red_low"].color.r();
							green = colors["red_low"].color.g();
							blue = colors["red_low"].color.b();
						}
						else if (health >= 0 && health < halfHealth) {
							red = ChangeScale(health, 0, halfHealth, colors["red_low"].color.r(), colors["red_medium"].color.r());
							green = ChangeScale(health, 0, halfHealth, colors["red_low"].color.g(), colors["red_medium"].color.g());
							blue = ChangeScale(health, 0, halfHealth, colors["red_low"].color.b(), colors["red_medium"].color.b());
						}
						else if (health >= halfHealth && health < maxHealth) {
							red = ChangeScale(health, halfHealth, maxHealth, colors["red_medium"].color.r(), colors["red_full"].color.r());
							green = ChangeScale(health, halfHealth, maxHealth, colors["red_medium"].color.g(), colors["red_full"].color.g());
							blue = ChangeScale(health, halfHealth, maxHealth, colors["red_medium"].color.b(), colors["red_full"].color.b());
						}
						else if (health >= maxHealth && health <= maxBuffedHealth) {
							red = ChangeScale(health, maxHealth, maxBuffedHealth, colors["red_full"].color.r(), colors["red_buff"].color.r());
							green = ChangeScale(health, maxHealth, maxBuffedHealth, colors["red_full"].color.g(), colors["red_buff"].color.g());
							blue = ChangeScale(health, maxHealth, maxBuffedHealth, colors["red_full"].color.b(), colors["red_buff"].color.b());
						}
						else if (health >= maxBuffedHealth) {
							// our max buffed health above does not take into account special cases so we have to compensate

							red = colors["red_buff"].color.r();
							green = colors["red_buff"].color.g();
							blue = colors["red_buff"].color.b();
						}
						else {
							// this DEFINITELY shouldn't ever happen

							return false;
						}

						*r = red / 255.0f;
						*g = green / 255.0f;
						*b = blue / 255.0f;

						return true;
					}
					else if (team == TFTeam_Blue) {
						int red;
						int green;
						int blue;

						if (health < 0) {
							// this should never happen

							red = colors["blu_low"].color.r();
							green = colors["blu_low"].color.g();
							blue = colors["blu_low"].color.b();
						}
						else if (health >= 0 && health < (maxHealth * 0.5f)) {
							red = ChangeScale(health, 0, maxHealth * 0.5f, colors["blu_low"].color.r(), colors["blu_medium"].color.r());
							green = ChangeScale(health, 0, maxHealth * 0.5f, colors["blu_low"].color.g(), colors["blu_medium"].color.g());
							blue = ChangeScale(health, 0, maxHealth * 0.5f, colors["blu_low"].color.b(), colors["blu_medium"].color.b());
						}
						else if (health >= (maxHealth * 0.5f) && health < maxHealth) {
							red = ChangeScale(health, maxHealth * 0.5f, maxHealth, colors["blu_medium"].color.r(), colors["blu_full"].color.r());
							green = ChangeScale(health, maxHealth * 0.5f, maxHealth, colors["blu_medium"].color.g(), colors["blu_full"].color.g());
							blue = ChangeScale(health, maxHealth * 0.5f, maxHealth, colors["blu_medium"].color.b(), colors["blu_full"].color.b());
						}
						else if (health >= maxHealth && health <= maxBuffedHealth) {
							red = ChangeScale(health, maxHealth, maxBuffedHealth, colors["blu_full"].color.r(), colors["blu_buff"].color.r());
							green = ChangeScale(health, maxHealth, maxBuffedHealth, colors["blu_full"].color.g(), colors["blu_buff"].color.g());
							blue = ChangeScale(health, maxHealth, maxBuffedHealth, colors["blu_full"].color.b(), colors["blu_buff"].color.b());
						}
						else if (health >= maxBuffedHealth) {
							// our max buffed health above does not take into account special cases so we have to compensate

							red = colors["blu_buff"].color.r();
							green = colors["blu_buff"].color.g();
							blue = colors["blu_buff"].color.b();
						}
						else {
							// this DEFINITELY shouldn't ever happen

							return false;
						}

						*r = red / 255.0f;
						*g = green / 255.0f;
						*b = blue / 255.0f;

						return true;
					}
				}
			}
		}
	}
	
	return false;
}

void PlayerOutlines::Paint(vgui::VPANEL vguiPanel) {
	if (topPanel == vgui::INVALID_PANEL) {
		std::string name = g_pVGuiPanel->GetName(vguiPanel);

		if (name.compare(TOP_PANEL_NAME) == 0) {
			topPanel = g_pVGui->PanelToHandle(vguiPanel);
		}
	}

	if (g_pVGui->HandleToPanel(topPanel) == vguiPanel) {
		for (int i = 0; i <= MAX_PLAYERS; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity) {
				continue;
			}

			if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
				continue;
			}

			bool* glowEnabled = MAKE_PTR(bool*, entity, Entities::pCTFPlayer__m_bGlowEnabled);
			*glowEnabled = g_PlayerOutlines->enabled->GetBool();
		}
	}
}

void PlayerOutlines::ProcessEntity(IClientEntity* entity) {
	static bool getGlowEffectColorHooked = false;

	if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
		return;
	}

	C_BaseCombatCharacter *baseCombatCharacter = dynamic_cast<C_BaseCombatCharacter *>(entity->GetBaseEntity());
	EHANDLE entityHandle = entity->GetBaseEntity();

	if (!getGlowEffectColorHooked) {
		Hooks::AddHook_C_TFPlayer_GetGlowEffectColor((C_TFPlayer *)baseCombatCharacter, Hook_C_TFPlayer_GetGlowEffectColor);
		getGlowEffectColorHooked = true;
	}

	bool* glowEnabled = MAKE_PTR(bool*, baseCombatCharacter, Entities::pCTFPlayer__m_bGlowEnabled);

	if (enabled->GetBool()) {
		*glowEnabled = true;
	}
}

void PlayerOutlines::ForceRefresh() {
	for (int i = 0; i <= MAX_PLAYERS; i++) {
		IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);
		
		if (!entity) {
			continue;
		}

		if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
			continue;
		}

		bool* glowEnabled = MAKE_PTR(bool*, entity, Entities::pCTFPlayer__m_bGlowEnabled);
		*glowEnabled = g_PlayerOutlines->enabled->GetBool();

		Hooks::CallFunc_C_TFPlayer_UpdateGlowEffect((C_TFPlayer *)entity);
	}
}

void PlayerOutlines::ColorCommand(const CCommand &command) {
	if (strncmp(command.Arg(0), "statusspec_playeroutlines_color_", 32) == 0) {
		std::string mainCommand = command.Arg(0);
		std::string colorType = mainCommand.substr(32);

		if (g_PlayerOutlines->colors.find(colorType) != g_PlayerOutlines->colors.end()) {
			if (command.ArgC() == 1) {
				Warning("\"%s\" = %i %i %i\n", g_PlayerOutlines->colors[colorType].command->GetName(), g_PlayerOutlines->colors[colorType].color.r(), g_PlayerOutlines->colors[colorType].color.g(), g_PlayerOutlines->colors[colorType].color.b());
				Msg(" - %s\n", g_PlayerOutlines->colors[colorType].command->GetHelpText());

				return;
			}
			else if (command.ArgC() >= 4 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2)) && IsInteger(command.Arg(3)))
			{
				g_PlayerOutlines->colors[colorType].color.SetColor(ColorRangeRestrict(std::stoi(command.Arg(1))), ColorRangeRestrict(std::stoi(command.Arg(2))), ColorRangeRestrict(std::stoi(command.Arg(3))));

				return;
			}
			else {
				Warning("Usage: %s <red> <green> <blue>\n", command.Arg(0));

				return;
			}
		}
	}

	Warning("Unrecognized command!\n");
}

int PlayerOutlines::GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	if (command.compare(0, 32, "statusspec_playeroutlines_color_") == 0) {
		std::string colorType = command.substr(32);

		if (g_PlayerOutlines->colors.find(colorType) != g_PlayerOutlines->colors.end()) {
			V_snprintf(commands[0], sizeof(commands[0]), "%s %i %i %i", command.c_str(), g_PlayerOutlines->colors[colorType].color.r(), g_PlayerOutlines->colors[colorType].color.g(), g_PlayerOutlines->colors[colorType].color.b());

			return 1;
		}
	}

	return 0;
}

void PlayerOutlines::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (!g_PlayerOutlines->enabled->GetBool()) {
		for (int i = 0; i <= MAX_PLAYERS; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);
		
			if (!entity) {
				continue;
			}

			if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
				continue;
			}

			bool* glowEnabled = MAKE_PTR(bool*, entity, Entities::pCTFPlayer__m_bGlowEnabled);
			*glowEnabled = false;
		}
	}
}