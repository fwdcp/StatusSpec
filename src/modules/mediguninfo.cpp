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

#include "convar.h"
#include "iclientmode.h"
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

	virtual void OnTick();
private:
	std::vector<MedigunPanel> bluMedigunPanels;
	std::vector<MedigunPanel> redMedigunPanels;
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
	g_pVGui->AddTickSignal(GetVPanel());
}