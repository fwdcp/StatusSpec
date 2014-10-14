/*
 *  projectileoutlines.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "projectileoutlines.h"

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

ProjectileOutlines::ProjectileOutlines() {
	colors["blu"].color = Color(88, 133, 162, 255);
	colors["blu"].command = new ConCommand("statusspec_projectileoutlines_color_blu", [](const CCommand &command) { g_ProjectileOutlines->ColorCommand(command); }, "the color used for outlines for BLU projectiles", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ProjectileOutlines->GetCurrentColor(partial, commands); });
	colors["red"].color = Color(184, 56, 59, 255);
	colors["red"].command = new ConCommand("statusspec_projectileoutlines_color_red", [](const CCommand &command) { g_ProjectileOutlines->ColorCommand(command); }, "the color used for outlines for RED projectiles", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ProjectileOutlines->GetCurrentColor(partial, commands); });
	doPostScreenSpaceEffectsHook = 0;
	frameHook = 0;

	enabled = new ConVar("statusspec_projectileoutlines_enabled", "0", FCVAR_NONE, "enable projectile outlines", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ProjectileOutlines->ToggleEnabled(var, pOldValue, flOldValue); });
	fade = new ConVar("statusspec_projectileoutlines_fade", "0", FCVAR_NONE, "make outlines fade with distance", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ProjectileOutlines->ToggleFade(var, pOldValue, flOldValue); });
	fade_distance = new ConVar("statusspec_projectileoutlines_fade_distance", "1600", FCVAR_NONE, "the distance (in Hammer units) at which outlines will fade");
	grenades = new ConVar("statusspec_projectileoutlines_grenades", "0", FCVAR_NONE, "enable outlines for grenades");
	rockets = new ConVar("statusspec_projectileoutlines_rockets", "0", FCVAR_NONE, "enable outlines for rockets");
	stickybombs = new ConVar("statusspec_projectileoutlines_stickybombs", "0", FCVAR_NONE, "enable outlines for stickybombs");
}

bool ProjectileOutlines::DoPostScreenSpaceEffectsHook(const CViewSetup *pSetup) {
	if (fade->GetBool()) {
		for (auto iterator = glows.begin(); iterator != glows.end(); ++iterator) {
			if (iterator->first.Get()) {
				vec_t distance = pSetup->origin.DistTo(iterator->first->GetRenderOrigin());

				if (distance > fade_distance->GetFloat()) {
					Color glowColor = GetGlowColor(iterator->first);
					float alpha = glowColor.a() / 255.0f;

					float scalar = fade_distance->GetFloat() / distance;

					iterator->second->SetAlpha(alpha * scalar);
				}
			}
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
}

void ProjectileOutlines::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

		for (int i = 0; i < maxEntity; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity) {
				continue;
			}

			if (rockets->GetBool() && Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFProjectile_Rocket")) {
				Color glowColor = GetGlowColor(entity);

				float red = glowColor.r() / 255.0f;
				float green = glowColor.g() / 255.0f;
				float blue = glowColor.b() / 255.0f;
				float alpha = glowColor.a() / 255.0f;

				SetGlowEffect(entity, true, Vector(red, green, blue), alpha);

				continue;
			}

			if (strcmp(entity->GetClientClass()->GetName(), "CTFGrenadePipebombProjectile") == 0) {
				int type = *MAKE_PTR(int*, entity, Entities::pCTFGrenadePipebombProjectile__m_iType);

				if (type == TFGrenadePipebombType_Grenade && grenades->GetBool()) {
					Color glowColor = GetGlowColor(entity);

					float red = glowColor.r() / 255.0f;
					float green = glowColor.g() / 255.0f;
					float blue = glowColor.b() / 255.0f;
					float alpha = glowColor.a() / 255.0f;

					SetGlowEffect(entity, true, Vector(red, green, blue), alpha);

					continue;
				}
				else if (type == TFGrenadePipebombType_Stickybomb && stickybombs->GetBool()) {
					Color glowColor = GetGlowColor(entity);

					float red = glowColor.r() / 255.0f;
					float green = glowColor.g() / 255.0f;
					float blue = glowColor.b() / 255.0f;
					float alpha = glowColor.a() / 255.0f;

					SetGlowEffect(entity, true, Vector(red, green, blue), alpha);

					continue;
				}
			}

			SetGlowEffect(entity, false);
		}
	}
	else if (curStage == FRAME_START) {
		if (fade->GetBool() && !doPostScreenSpaceEffectsHook) {
			try {
				doPostScreenSpaceEffectsHook = Funcs::AddHook_IClientMode_DoPostScreenSpaceEffects(Interfaces::GetClientMode(), SH_MEMBER(this, &ProjectileOutlines::DoPostScreenSpaceEffectsHook), false);
			}
			catch (bad_pointer &e) {
				Warning(e.what());
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

Color ProjectileOutlines::GetGlowColor(IClientEntity *entity) {
	float red = 255.0f;
	float green = 255.0f;
	float blue = 255.0f;
	float alpha = 255.0f;

	TFTeam team = (TFTeam)*MAKE_PTR(int *, entity, Entities::pCBaseEntity__m_iTeamNum);

	if (team == TFTeam_Red) {
		red = colors["red"].color.r();
		green = colors["red"].color.g();
		blue = colors["red"].color.b();
		alpha = colors["red"].color.a();
	}
	else if (team == TFTeam_Blue) {
		red = colors["blu"].color.r();
		green = colors["blu"].color.g();
		blue = colors["blu"].color.b();
		alpha = colors["blu"].color.a();
	}

	return Color(red, green, blue, alpha);
}

void ProjectileOutlines::SetGlowEffect(IClientEntity *entity, bool enabled, Vector color, float alpha) {
	EHANDLE entityHandle = dynamic_cast<C_BaseEntity *>(entity);

	if (enabled) {
		if (glows.find(entityHandle) == glows.end()) {
			glows[entityHandle] = new CGlowObject(entityHandle.Get(), color, alpha, true, true);
		}
		else {
			glows[entityHandle]->SetColor(color);
			glows[entityHandle]->SetAlpha(alpha);
		}
	}
	else {
		if (glows.find(entityHandle) != glows.end()) {
			delete glows[entityHandle];
			glows.erase(entityHandle);
		}
	}
}

void ProjectileOutlines::ColorCommand(const CCommand &command) {
	if (strncmp(command.Arg(0), "statusspec_projectileoutlines_color_", 32) == 0) {
		std::string mainCommand = command.Arg(0);
		std::string colorType = mainCommand.substr(32);

		if (colors.find(colorType) != colors.end()) {
			if (command.ArgC() == 1) {
				Warning("\"%s\" = %i %i %i %i\n", colors[colorType].command->GetName(), colors[colorType].color.r(), colors[colorType].color.g(), colors[colorType].color.b(), colors[colorType].color.a());
				Msg(" - %s\n", colors[colorType].command->GetHelpText());

				return;
			}
			else if (command.ArgC() >= 5 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2)) && IsInteger(command.Arg(3)) && IsInteger(command.Arg(4)))
			{
				colors[colorType].color.SetColor(ColorRangeRestrict(std::stoi(command.Arg(1))), ColorRangeRestrict(std::stoi(command.Arg(2))), ColorRangeRestrict(std::stoi(command.Arg(3))), ColorRangeRestrict(std::stoi(command.Arg(4))));

				return;
			}
			else if (command.ArgC() >= 4 && IsInteger(command.Arg(1)) && IsInteger(command.Arg(2)) && IsInteger(command.Arg(3)))
			{
				colors[colorType].color.SetColor(ColorRangeRestrict(std::stoi(command.Arg(1))), ColorRangeRestrict(std::stoi(command.Arg(2))), ColorRangeRestrict(std::stoi(command.Arg(3))), 255);

				return;
			}
			else {
				Warning("Usage: %s <red> <green> <blue> [alpha]\n", command.Arg(0));

				return;
			}
		}
	}

	Warning("Unrecognized command!\n");
}

int ProjectileOutlines::GetCurrentColor(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	if (command.compare(0, 32, "statusspec_projectileoutlines_color_") == 0) {
		std::string colorType = command.substr(32);

		if (colors.find(colorType) != colors.end()) {
			V_snprintf(commands[0], sizeof(commands[0]), "%s %i %i %i %i", command.c_str(), colors[colorType].color.r(), colors[colorType].color.g(), colors[colorType].color.b(), colors[colorType].color.a());

			return 1;
		}
	}

	return 0;
}

void ProjectileOutlines::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &ProjectileOutlines::FrameHook), true);
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

void ProjectileOutlines::ToggleFade(IConVar *var, const char *pOldValue, float flOldValue) {
	if (fade->GetBool()) {
		if (!doPostScreenSpaceEffectsHook) {
			try {
				doPostScreenSpaceEffectsHook = Funcs::AddHook_IClientMode_DoPostScreenSpaceEffects(Interfaces::GetClientMode(), SH_MEMBER(this, &ProjectileOutlines::DoPostScreenSpaceEffectsHook), false);
			}
			catch (bad_pointer &e) {
				Warning(e.what());
			}
		}
	}
	else {
		if (doPostScreenSpaceEffectsHook) {
			if (Funcs::RemoveHook(doPostScreenSpaceEffectsHook)) {
				doPostScreenSpaceEffectsHook = 0;
			}
		}
	}
}