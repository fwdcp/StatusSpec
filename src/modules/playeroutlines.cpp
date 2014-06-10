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
	team_blu = Color(88, 133, 162);
	team_red = Color(184, 56, 59);
	
	color_blu = new ConCommand("statusspec_playeroutlines_color_blu", PlayerOutlines::SetColor, "the color used for BLU team player outlines", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	color_red = new ConCommand("statusspec_playeroutlines_color_red", PlayerOutlines::SetColor, "the color used for RED team player outlines", FCVAR_NONE, PlayerOutlines::GetCurrentColor);
	enabled = new ConVar("statusspec_playeroutlines_enabled", "0", FCVAR_NONE, "enable player outlines", PlayerOutlines::ToggleEnabled);
	force_refresh = new ConCommand("statusspec_playeroutlines_force_refresh", PlayerOutlines::ForceRefresh, "force the player outlines to refresh", FCVAR_NONE);
	team_colors = new ConVar("statusspec_playeroutlines_team_colors", "0", FCVAR_NONE, "override default health-based outline colors with team colors");
}

PlayerOutlines::~PlayerOutlines() {
	for (auto iterator = glowHooks.begin(); iterator != glowHooks.end(); iterator++) {
		C_BaseCombatCharacter *baseCombatCharacter = dynamic_cast<C_BaseCombatCharacter *>(Interfaces::pClientEntityList->GetClientEntity(iterator->first.GetEntryIndex())->GetBaseEntity());
		GlowHooks_t hooks = iterator->second;

		if (hooks.glowColorHook) {
			SH_REMOVE_HOOK_ID(hooks.glowColorHook);
		}

		if (hooks.glowForceHook) {
			SH_REMOVE_HOOK_ID(hooks.glowForceHook);
		}

		if (baseCombatCharacter) {
			bool* glowEnabled = MAKE_PTR(bool*, baseCombatCharacter, Entities::pCTFPlayer__m_bGlowEnabled);
			*glowEnabled = false;
		}

		glowHooks.erase(iterator);
	}
}

bool PlayerOutlines::IsEnabled() {
	return enabled->GetBool();
}

bool PlayerOutlines::DataChangeOverride(C_BaseCombatCharacter *baseCombatCharacter) {
	if (baseCombatCharacter) {
		bool* glowEnabled = MAKE_PTR(bool*, baseCombatCharacter, Entities::pCTFPlayer__m_bGlowEnabled);

		if (enabled->GetBool()) {
			*glowEnabled = true;
		}

		return true;
	}
	else {
		return false;
	}
}

bool PlayerOutlines::GetGlowEffectColorOverride(C_TFPlayer *tfPlayer, float *r, float *g, float *b) {
	if (team_colors->GetBool()) {
		TFTeam team = (TFTeam) *MAKE_PTR(int*, tfPlayer, Entities::pCTFPlayer__m_iTeamNum);
	
		if (team == TFTeam_Red) {
			*r = float(team_red.r() / 255.0f);
			*g = float(team_red.g() / 255.0f);
			*b = float(team_red.b() / 255.0f);

			return true;
		}
		else if (team == TFTeam_Blue) {
			*r = float(team_blu.r() / 255.0f);
			*g = float(team_blu.g() / 255.0f);
			*b = float(team_blu.b() / 255.0f);

			return true;
		}
	}
	
	return false;
}

void PlayerOutlines::ProcessEntity(IClientEntity* entity) {
	if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
		return;
	}

	C_BaseCombatCharacter *baseCombatCharacter = dynamic_cast<C_BaseCombatCharacter *>(entity->GetBaseEntity());
	CHandle<C_BaseCombatCharacter> baseCombatCharacterHandle = baseCombatCharacter;

	if (glowHooks.find(baseCombatCharacterHandle) == glowHooks.end()) {
		glowHooks[baseCombatCharacter].glowColorHook = AddHook_C_TFPlayer_GetGlowEffectColor((C_TFPlayer *) baseCombatCharacter);
		glowHooks[baseCombatCharacter].glowForceHook = AddHook_C_BaseCombatCharacter_OnDataChanged(baseCombatCharacter);
	}

	bool* glowEnabled = MAKE_PTR(bool*, baseCombatCharacter, Entities::pCTFPlayer__m_bGlowEnabled);

	if (enabled->GetBool()) {
		*glowEnabled = true;
	}
}

void PlayerOutlines::ForceRefresh() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();
	
	for (int i = 0; i < maxEntity; i++) {
		IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);
		
		if (!entity) {
			continue;
		}

		if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
			continue;
		}

		bool* glowEnabled = MAKE_PTR(bool*, entity, Entities::pCTFPlayer__m_bGlowEnabled);
		*glowEnabled = g_PlayerOutlines->enabled->GetBool();

		Call_C_TFPlayer_UpdateGlowEffect((C_TFPlayer *) entity);
	}
}

int PlayerOutlines::GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	if (stricmp(command.c_str(), "statusspec_playeroutlines_color_blu") == 0) {
		V_snprintf(commands[0], COMMAND_COMPLETION_ITEM_LENGTH, "%s %i %i %i", command.c_str(), g_PlayerOutlines->team_blu.r(), g_PlayerOutlines->team_blu.g(), g_PlayerOutlines->team_blu.b());
		
		return 1;
	}
	else if (stricmp(command.c_str(), "statusspec_playeroutlines_color_red") == 0) {
		V_snprintf(commands[0], COMMAND_COMPLETION_ITEM_LENGTH, "%s %i %i %i", command.c_str(), g_PlayerOutlines->team_red.r(), g_PlayerOutlines->team_red.g(), g_PlayerOutlines->team_red.b());

		return 1;
	}
	else {
		return 0;
	}
}

void PlayerOutlines::SetColor(const CCommand &command) {
	if (command.ArgC() == 0) {
		if (stricmp(command.Arg(0), "statusspec_playeroutlines_color_blu")) {
			Msg("The current BLU team player outline color is rgb(%i, %i, %i).\n", g_PlayerOutlines->team_blu.r(), g_PlayerOutlines->team_blu.g(), g_PlayerOutlines->team_blu.b());
			return;
		}
		else if (stricmp(command.Arg(0), "statusspec_playeroutlines_color_red")) {
			Msg("The current RED team player outline color is rgb(%i, %i, %i).\n", g_PlayerOutlines->team_red.r(), g_PlayerOutlines->team_red.g(), g_PlayerOutlines->team_red.b());
			return;
		}
	}
	else if (command.ArgC() < 3 || !IsInteger(command.Arg(1)) || !IsInteger(command.Arg(2)) || !IsInteger(command.Arg(3)))
	{
		Warning("Usage: %s <red> <green> <blue>\n", command.Arg(0));
		return;
	}

	int red = ColorRangeRestrict(std::stoi(command.Arg(1)));
	int green = ColorRangeRestrict(std::stoi(command.Arg(2)));
	int blue = ColorRangeRestrict(std::stoi(command.Arg(3)));

	if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_active")) {
		g_PlayerOutlines->team_blu.SetColor(red, green, blue);
		Msg("Set BLU team player outline color to rgb(%i, %i, %i).\n", red, green, blue);
	}
	else if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_nonactive")) {
		g_PlayerOutlines->team_red.SetColor(red, green, blue);
		Msg("Set RED team player outline color to rgb(%i, %i, %i).\n", red, green, blue);
	}
	else {
		Warning("Unrecognized command!\n");
	}
}

void PlayerOutlines::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (!g_PlayerOutlines->enabled->GetBool()) {
		int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();
	
		for (int i = 0; i < maxEntity; i++) {
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