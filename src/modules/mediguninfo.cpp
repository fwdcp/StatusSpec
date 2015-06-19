/*
 *  mediguninfo.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "mediguninfo.h"

#include <vector>

#include "cbase.h"
#include "convar.h"
#include "iclientmode.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class MedigunInfo::MainPanel : public vgui::EditablePanel {
public:
	MainPanel(vgui::Panel *parent, const char *panelName);

	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void OnTick();
private:
	int bluBaseX;
	int bluBaseY;
	int bluOffsetX;
	int bluOffsetY;
	int redBaseX;
	int redBaseY;
	int redOffsetX;
	int redOffsetY;

	std::vector<MedigunPanel *> bluMedigunPanels;
	std::vector<MedigunPanel *> redMedigunPanels;
};

class MedigunInfo::MedigunPanel : public vgui::EditablePanel {
	DECLARE_CLASS_SIMPLE(MedigunPanel, vgui::EditablePanel);

public:
	MedigunPanel(vgui::Panel *parent, const char *panelName);

private:
	MESSAGE_FUNC_PARAMS(OnMedigunInfoUpdate, "MedigunInfo", attributes);

	bool alive;
	float level;
	TFMedigun medigun;
	TFResistType resistType;
	TFTeam team;
};

MedigunInfo::MedigunInfo(std::string name) : Module(name) {
	enabled = new ConVar("statusspec_mediguninfo_enabled", "0", FCVAR_NONE, "enable medigun info", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<MedigunInfo>("Medigun Info")->ToggleEnabled(var, pOldValue, flOldValue); });
	reload_settings = new ConCommand("statusspec_mediguninfo_reload_settings", []() { g_ModuleManager->GetModule<MedigunInfo>("Medigun Info")->ReloadSettings(); }, "reload settings for the medigun info HUD from the resource file", FCVAR_NONE);
}

bool MedigunInfo::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_iItemDefinitionIndex" })) {
		PRINT_TAG();
		Warning("Required property m_iItemDefinitionIndex for CWeaponMedigun for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_bChargeRelease" })) {
		PRINT_TAG();
		Warning("Required property m_bChargeRelease for CWeaponMedigun for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_nChargeResistType" })) {
		PRINT_TAG();
		Warning("Required property m_nChargeResistType for CWeaponMedigun for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CWeaponMedigun", { "m_flChargeLevel" })) {
		PRINT_TAG();
		Warning("Required property m_flChargeLevel for CWeaponMedigun for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", name.c_str());

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

void MedigunInfo::ReloadSettings() {
	mainPanel->LoadControlSettings("Resource/UI/MedigunInfo.res");
}

void MedigunInfo::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!mainPanel) {
			try {
				vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

				if (viewport) {
					mainPanel = new MainPanel(viewport, "MedigunInfo");
				}
				else {
					Warning("Could not initialize the panel!\n");
					var->SetValue(0);
				}
			}
			catch (bad_pointer) {
				Warning("Could not initialize the panel!\n");
				var->SetValue(0);
			}
		}

		if (mainPanel) {
			mainPanel->SetEnabled(true);
		}
	}
	else {
		if (mainPanel) {
			delete mainPanel;
			mainPanel = nullptr;
		}
	}
}

MedigunInfo::MainPanel::MainPanel(vgui::Panel *parent, const char *panelName) : vgui::EditablePanel(parent, panelName) {
	LoadControlSettings("Resource/UI/MedigunInfo.res");

	g_pVGui->AddTickSignal(GetVPanel());
}

void MedigunInfo::MainPanel::ApplySettings(KeyValues *inResourceData) {
	vgui::EditablePanel::ApplySettings(inResourceData);

	int alignScreenWide, alignScreenTall;
	g_pVGuiSurface->GetScreenSize(alignScreenWide, alignScreenTall);

	int wide, tall;
	GetSize(wide, tall);

	GetPos(redBaseX, redBaseY);
	ComputePos(inResourceData->GetString("red_base_x"), redBaseX, wide, alignScreenWide, true);
	ComputePos(inResourceData->GetString("red_base_y"), redBaseY, tall, alignScreenTall, false);

	GetPos(bluBaseX, bluBaseY);
	ComputePos(inResourceData->GetString("blu_base_x"), bluBaseX, wide, alignScreenWide, true);
	ComputePos(inResourceData->GetString("blu_base_y"), bluBaseY, tall, alignScreenTall, false);

	redOffsetX = inResourceData->GetInt("red_offset_x");
	redOffsetY = inResourceData->GetInt("red_offset_y");
	bluOffsetX = inResourceData->GetInt("blu_offset_x");
	bluOffsetY = inResourceData->GetInt("blu_offset_y");
}

void MedigunInfo::MainPanel::OnTick() {
	size_t bluMediguns = 0;
	size_t redMediguns = 0;

	for (Player player : Player::Iterable()) {
		if (player.GetClass() == TFClass_Medic && (player.GetTeam() == TFTeam_Blue || player.GetTeam() == TFTeam_Red)) {
			for (int weaponIndex = 0; weaponIndex < MAX_WEAPONS; weaponIndex++) {
				C_BaseCombatWeapon *weapon = player.GetWeapon(weaponIndex);

				if (weapon && Entities::CheckEntityBaseclass(weapon, "WeaponMedigun")) {
					MedigunPanel *medigunPanel;

					if (player.GetTeam() == TFTeam_Red) {
						redMediguns++;

						if (redMediguns > redMedigunPanels.size()) {
							medigunPanel = new MedigunPanel(this, "MedigunPanel");
							redMedigunPanels.push_back(medigunPanel);
						}
						else {
							medigunPanel = redMedigunPanels.at(redMediguns - 1);
						}

						medigunPanel->SetPos(redBaseX + (redOffsetX * (redMediguns - 1)), redBaseY + (redOffsetY * (redMediguns - 1)));
					}
					else if (player.GetTeam() == TFTeam_Blue) {
						bluMediguns++;

						if (bluMediguns > bluMedigunPanels.size()) {
							medigunPanel = new MedigunPanel(this, "MedigunPanel");
							bluMedigunPanels.push_back(medigunPanel);
						}
						else {
							medigunPanel = bluMedigunPanels.at(bluMediguns - 1);
						}

						medigunPanel->SetPos(bluBaseX + (bluOffsetX * (bluMediguns - 1)), bluBaseY + (bluOffsetY * (bluMediguns - 1)));
					}

					KeyValues *medigunInfo = new KeyValues("MedigunInfo");

					int itemDefinitionIndex = *Entities::GetEntityProp<int *>(weapon, { "m_iItemDefinitionIndex" });
					TFMedigun type = TFMedigun_Unknown;
					if (itemDefinitionIndex == 29 || itemDefinitionIndex == 211 || itemDefinitionIndex == 663 || itemDefinitionIndex == 796 || itemDefinitionIndex == 805 || itemDefinitionIndex == 885 || itemDefinitionIndex == 894 || itemDefinitionIndex == 903 || itemDefinitionIndex == 912 || itemDefinitionIndex == 961 || itemDefinitionIndex == 970) {
						type = TFMedigun_MediGun;
					}
					else if (itemDefinitionIndex == 35) {
						type = TFMedigun_Kritzkrieg;
					}
					else if (itemDefinitionIndex == 411) {
						type = TFMedigun_QuickFix;
					}
					else if (itemDefinitionIndex == 998) {
						type = TFMedigun_Vaccinator;
					}

					medigunInfo->SetBool("alive", player.IsAlive());
					medigunInfo->SetFloat("level", *Entities::GetEntityProp<float *>(weapon, { "m_flChargeLevel" }));
					medigunInfo->SetInt("medigun", type);
					medigunInfo->SetInt("resistType", *Entities::GetEntityProp<int *>(weapon, { "m_nChargeResistType" }));
					medigunInfo->SetInt("team", player.GetTeam());

					PostMessage(medigunPanel, medigunInfo);

					break;
				}
			}
		}
	}

	while (redMediguns < redMedigunPanels.size()) {
		delete redMedigunPanels.back();
		redMedigunPanels.pop_back();
	}
	
	while (bluMediguns < bluMedigunPanels.size()) {
		delete bluMedigunPanels.back();
		bluMedigunPanels.pop_back();
	}
}

MedigunInfo::MedigunPanel::MedigunPanel(vgui::Panel *parent, const char *panelName) : vgui::EditablePanel(parent, panelName) {}

void MedigunInfo::MedigunPanel::OnMedigunInfoUpdate(KeyValues *attributes) {
	if (alive != attributes->GetBool("alive") || medigun != attributes->GetInt("medigun") || resistType != attributes->GetInt("resistType") || team != attributes->GetInt("team")) {
		LoadControlSettings("Resource/UI/MedigunInfoSingle.res", nullptr, nullptr, attributes);
	}

	alive = attributes->GetBool("alive");
	level = attributes->GetFloat("percentage");
	medigun = (TFMedigun) attributes->GetInt("medigun");
	resistType = (TFResistType) attributes->GetInt("resistType");
	team = (TFTeam) attributes->GetInt("team");

	SetDialogVariable("redcharge", int(floor(level * 100.0f)));
	SetDialogVariable("redcharges", int(floor(level * 4.0f)));
	SetDialogVariable("redcharge1", int(floor(level * 400.0f) - 0.0f) < 0 ? 0 : int(floor(level * 400.0f) - 0.0f));
	SetDialogVariable("redcharge2", int(floor(level * 400.0f) - 100.0f) < 0 ? 0 : int(floor(level * 400.0f) - 100.0f));
	SetDialogVariable("redcharge3", int(floor(level * 400.0f) - 200.0f) < 0 ? 0 : int(floor(level * 400.0f) - 200.0f));
	SetDialogVariable("redcharge4", int(floor(level * 400.0f) - 300.0f) < 0 ? 0 : int(floor(level * 400.0f) - 300.0f));
}