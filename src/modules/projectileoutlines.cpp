/*
 *  projectileoutlines.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "projectileoutlines.h"

#include "cbase.h"
#include "c_baseentity.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Panel.h"
#include "view_shared.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../tfdefs.h"

class ProjectileOutlines::Panel : public vgui::Panel {
public:
	Panel(vgui::Panel *parent, const char *panelName, std::function<void()> updateFunction, std::function<void(Vector)> updateFadesFunction);

	virtual void OnTick();

	GlowManager *GetGlowManager();
private:
	bool DoPostScreenSpaceEffectsHook(const CViewSetup *pSetup);

	int doPostScreenSpaceEffectsHook;
	GlowManager glowManager;
	std::function<void(Vector)> updateGlowFades;
	std::function<void()> updateGlows;
};

ProjectileOutlines::ProjectileOutlines() {
	colors["blu"].color = Color(88, 133, 162, 255);
	colors["blu"].command = new ConCommand("statusspec_projectileoutlines_color_blu", [](const CCommand &command) { g_ModuleManager->GetModule<ProjectileOutlines>()->ColorCommand(command); }, "the color used for outlines for BLU projectiles", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<ProjectileOutlines>()->GetCurrentColor(partial, commands); });
	colors["red"].color = Color(184, 56, 59, 255);
	colors["red"].command = new ConCommand("statusspec_projectileoutlines_color_red", [](const CCommand &command) { g_ModuleManager->GetModule<ProjectileOutlines>()->ColorCommand(command); }, "the color used for outlines for RED projectiles", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<ProjectileOutlines>()->GetCurrentColor(partial, commands); });
	panel = nullptr;

	enabled = new ConVar("statusspec_projectileoutlines_enabled", "0", FCVAR_NONE, "enable projectile outlines", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<ProjectileOutlines>()->ToggleEnabled(var, pOldValue, flOldValue); });
	fade = new ConVar("statusspec_projectileoutlines_fade", "0", FCVAR_NONE, "make outlines fade with distance");
	fade_distance = new ConVar("statusspec_projectileoutlines_fade_distance", "1600", FCVAR_NONE, "the distance (in Hammer units) at which outlines will fade");
	grenades = new ConVar("statusspec_projectileoutlines_grenades", "0", FCVAR_NONE, "enable outlines for grenades");
	rockets = new ConVar("statusspec_projectileoutlines_rockets", "0", FCVAR_NONE, "enable outlines for rockets");
	stickybombs = new ConVar("statusspec_projectileoutlines_stickybombs", "0", FCVAR_NONE, "enable outlines for stickybombs");
}

bool ProjectileOutlines::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", g_ModuleManager->GetModuleName<ProjectileOutlines>().c_str());

		ready = false;
	}

	if (!Interfaces::pClientEntityList) {
		PRINT_TAG();
		Warning("Required interface IClientEntityList for module %s not available!\n", g_ModuleManager->GetModuleName<ProjectileOutlines>().c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CTFGrenadePipebombProjectile", { "m_iType" })) {
		PRINT_TAG();
		Warning("Required property m_iType for CTFGrenadePipebombProjectile for module %s not available!\n", g_ModuleManager->GetModuleName<ProjectileOutlines>().c_str());

		ready = false;
	}

	if (!GlowManager::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required glow manager for module %s not available!\n", g_ModuleManager->GetModuleName<ProjectileOutlines>().c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", g_ModuleManager->GetModuleName<ProjectileOutlines>().c_str());
	}

	return ready;
}

void ProjectileOutlines::UpdateGlowFades(Vector origin) {
	if (fade->GetBool()) {
		for (auto iterator : glows) {
			if (iterator.first.Get()) {
				vec_t distance = origin.DistTo(iterator.first->GetRenderOrigin());

				if (distance > fade_distance->GetFloat()) {
					Color glowColor = GetGlowColor(iterator.first);
					float alpha = glowColor.a() / 255.0f;

					float scalar = fade_distance->GetFloat() / distance;

					iterator.second->SetAlpha(alpha * scalar);
				}
			}
		}
	}
}

void ProjectileOutlines::UpdateGlows() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i < maxEntity; i++) {
		IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (rockets->GetBool() && Entities::CheckEntityBaseclass(entity, "TFProjectile_Rocket")) {
			Color glowColor = GetGlowColor(entity);

			float red = glowColor.r() / 255.0f;
			float green = glowColor.g() / 255.0f;
			float blue = glowColor.b() / 255.0f;
			float alpha = glowColor.a() / 255.0f;

			SetGlowEffect(entity, true, Vector(red, green, blue), alpha);

			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "TFProjectile_Pipebomb")) {
			int type = *Entities::GetEntityProp<int *>(entity, { "m_iType" });

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

Color ProjectileOutlines::GetGlowColor(IClientEntity *entity) {
	float red = 255.0f;
	float green = 255.0f;
	float blue = 255.0f;
	float alpha = 255.0f;

	TFTeam team = (TFTeam)dynamic_cast<C_BaseEntity *>(entity)->GetTeamNumber();

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
			glows[entityHandle] = new GlowManager::GlowObject(panel->GetGlowManager(), entityHandle.Get(), color, alpha, true, true);
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
	if (strncmp(command.Arg(0), "statusspec_projectileoutlines_color_", 36) == 0) {
		std::string mainCommand = command.Arg(0);
		std::string colorType = mainCommand.substr(36);

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

	if (command.compare(0, 36, "statusspec_projectileoutlines_color_") == 0) {
		std::string colorType = command.substr(36);

		if (colors.find(colorType) != colors.end()) {
			V_snprintf(commands[0], sizeof(commands[0]), "%s %i %i %i %i", command.c_str(), colors[colorType].color.r(), colors[colorType].color.g(), colors[colorType].color.b(), colors[colorType].color.a());

			return 1;
		}
	}

	return 0;
}

void ProjectileOutlines::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!panel) {
			panel = new Panel(nullptr, "ProjectileOutlines", std::bind(&ProjectileOutlines::UpdateGlows, this), std::bind(&ProjectileOutlines::UpdateGlowFades, this, std::placeholders::_1));
		}

		if (panel) {
			panel->SetEnabled(true);
		}
	}
	else {
		if (panel) {
			delete panel;
			panel = nullptr;
		}
	}
}

ProjectileOutlines::Panel::Panel(vgui::Panel *parent, const char *panelName, std::function<void()> updateFunction, std::function<void(Vector)> updateFadesFunction) : vgui::Panel(parent, panelName) {
	g_pVGui->AddTickSignal(GetVPanel());

	try {
		doPostScreenSpaceEffectsHook = Funcs::AddHook_IClientMode_DoPostScreenSpaceEffects(Interfaces::GetClientMode(), SH_MEMBER(this, &ProjectileOutlines::Panel::DoPostScreenSpaceEffectsHook), false);
	}
	catch (bad_pointer &e) {
		Warning("%s\n", e.what());
		doPostScreenSpaceEffectsHook = 0;
	}

	updateGlows = updateFunction;
	updateGlowFades = updateFadesFunction;
}

void ProjectileOutlines::Panel::OnTick() {
	if (!doPostScreenSpaceEffectsHook) {
		try {
			doPostScreenSpaceEffectsHook = Funcs::AddHook_IClientMode_DoPostScreenSpaceEffects(Interfaces::GetClientMode(), SH_MEMBER(this, &ProjectileOutlines::Panel::DoPostScreenSpaceEffectsHook), false);
		}
		catch (bad_pointer &e) {
			Warning("%s\n", e.what());
		}
	}

	updateGlows();
}

GlowManager *ProjectileOutlines::Panel::GetGlowManager() {
	return &glowManager;
}

bool ProjectileOutlines::Panel::DoPostScreenSpaceEffectsHook(const CViewSetup *pSetup) {
	updateGlowFades(pSetup->origin);

	glowManager.RenderGlowEffects(pSetup);

	RETURN_META_VALUE(MRES_IGNORED, false);
}