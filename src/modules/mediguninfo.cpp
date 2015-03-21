/*
 *  mediguninfo.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "mediguninfo.h"

#include <iomanip>

#include "cbase.h"
#include "c_baseentity.h"
#include "convar.h"
#include "filesystem.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "shareddefs.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include "../vgui_controls/ImageProgressBar.h"
#include "vgui_controls/ScalableImagePanel.h"
#include "../vgui_controls/VariableLabel.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

MedigunInfo::MedigunInfo(std::string name) : Module(name) {
	dynamicMeterSettings = new KeyValues("MedigunInfoDynamicMeters");
	dynamicMeterSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/mediguninfodynamicmeters.res", "mod");
	frameHook = 0;

	dynamic_meters = new ConVar("statusspec_mediguninfo_dynamic_meters", "0", FCVAR_NONE, "enable charge meters to change based on medigun");
	enabled = new ConVar("statusspec_mediguninfo_enabled", "0", FCVAR_NONE, "enable medigun info", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<MedigunInfo>("Medigun Info")->ToggleEnabled(var, pOldValue, flOldValue); });
	individual_charge_meters = new ConVar("statusspec_mediguninfo_individual_charge_meters", "1", FCVAR_NONE, "enable individual charge meters (for Vaccinator)");
	reload_settings = new ConCommand("statusspec_mediguninfo_reload_settings", []() { g_ModuleManager->GetModule<MedigunInfo>("Medigun Info")->ReloadSettings(); }, "reload settings for the medigun info HUD from the resource file", FCVAR_NONE);
}

bool MedigunInfo::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", name.c_str());

		ready = false;
	}

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

	for (int i = 0; i < MAX_WEAPONS; i++) {
		std::stringstream ss;
		std::string arrayIndex;
		ss << std::setfill('0') << std::setw(3) << i;
		ss >> arrayIndex;

		if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_hMyWeapons", arrayIndex })) {
			PRINT_TAG();
			Warning("Required property table m_hMyWeapons for CTFPlayer for module %s not available!\n", name.c_str());

			ready = false;

			break;
		}
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", name.c_str());

		ready = false;
	}

	try {
		Interfaces::GetClientMode();
	}
	catch (bad_pointer &e) {
		PRINT_TAG();
		Warning("Module %s requires IClientMode, which cannot be verified at this time!\n", name.c_str());
	}

	return ready;
}

void MedigunInfo::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		medigunInfo.clear();

		for (auto iterator = Player::begin(); iterator != Player::end(); ++iterator) {
			Player player = *iterator;

			TFTeam team = player.GetTeam();

			if (team != TFTeam_Red && team != TFTeam_Blue) {
				continue;
			}

			if (!player.IsAlive()) {
				continue;
			}

			for (int i = 0; i < MAX_WEAPONS; i++) {
				std::stringstream ss;
				std::string arrayIndex;
				ss << std::setfill('0') << std::setw(3) << i;
				ss >> arrayIndex;

				IClientEntity *weapon = Entities::GetEntityProp<EHANDLE *>(player.GetEntity(), { "m_hMyWeapons", arrayIndex })->Get();

				if (!weapon || !Entities::CheckEntityBaseclass(weapon, "WeaponMedigun")) {
					continue;
				}

				Medigun_t medigun;

				int itemDefinitionIndex = *Entities::GetEntityProp<int *>(weapon, { "m_iItemDefinitionIndex" });

				switch (itemDefinitionIndex) {
				case 29:
				case 211:
				case 663:
				case 796:
				case 805:
				case 885:
				case 894:
				case 903:
				case 912:
				case 961:
				case 970:
				{
					medigun.type = TFMedigun_MediGun;
					break;
				}
				case 35:
				{
					medigun.type = TFMedigun_Kritzkrieg;
					break;
				}
				case 411:
				{
					medigun.type = TFMedigun_QuickFix;
					break;
				}
				case 998:
				{
					medigun.type = TFMedigun_Vaccinator;
					break;
				}
				default:
				{
					medigun.type = TFMedigun_Unknown;
					break;
				}
				}

				medigun.chargeRelease = *Entities::GetEntityProp<bool *>(weapon, { "m_bChargeRelease" });
				medigun.chargeResistType = (TFResistType)*Entities::GetEntityProp<int *>(weapon, { "m_nChargeResistType" });
				medigun.chargeLevel = *Entities::GetEntityProp<float *>(weapon, { "m_flChargeLevel" });

				if (medigunInfo.find(team) == medigunInfo.end() || medigunInfo[team].chargeLevel <= 0.0f && medigun.chargeLevel >= 0.0f) {
					medigunInfo[team] = medigun;
				}
			}
		}

		Paint();
	}

	RETURN_META(MRES_IGNORED);
}

void MedigunInfo::InitHud() {
	if (panels.find("MedigunInfo") == panels.end()) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			vgui::EditablePanel *medigunInfoPanel = new vgui::EditablePanel(viewport, "MedigunInfo");
			panels["MedigunInfo"] = medigunInfoPanel;
			panels["MedigunInfoBackground"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBackground");
			panels["MedigunInfoRedBackground"] = new vgui::ScalableImagePanel(medigunInfoPanel, "MedigunInfoRedBackground");
			panels["MedigunInfoRedNameLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedNameLabel", "");
			panels["MedigunInfoRedChargeMeter"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter");
			panels["MedigunInfoRedChargeMeter1"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter1");
			panels["MedigunInfoRedChargeMeter2"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter2");
			panels["MedigunInfoRedChargeMeter3"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter3");
			panels["MedigunInfoRedChargeMeter4"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter4");
			panels["MedigunInfoRedChargeLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedChargeLabel", "");
			panels["MedigunInfoRedIndividualChargesLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedIndividualChargesLabel", "");
			panels["MedigunInfoRedChargeAdvantageLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedChargeAdvantageLabel", "");
			panels["MedigunInfoRedChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoRedChargeTypeIcon");
			panels["MedigunInfoBluBackground"] = new vgui::ScalableImagePanel(medigunInfoPanel, "MedigunInfoBluBackground");
			panels["MedigunInfoBluNameLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluNameLabel", "");
			panels["MedigunInfoBluChargeMeter"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter");
			panels["MedigunInfoBluChargeMeter1"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter1");
			panels["MedigunInfoBluChargeMeter2"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter2");
			panels["MedigunInfoBluChargeMeter3"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter3");
			panels["MedigunInfoBluChargeMeter4"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter4");
			panels["MedigunInfoBluChargeLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluChargeLabel", "");
			panels["MedigunInfoBluIndividualChargesLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluIndividualChargesLabel", "");
			panels["MedigunInfoBluChargeAdvantageLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluChargeAdvantageLabel", "");
			panels["MedigunInfoBluChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBluChargeTypeIcon");

			medigunInfoPanel->LoadControlSettings("Resource/UI/MedigunInfo.res");
		}
	}
}

void MedigunInfo::Paint() {
	panels["MedigunInfo"]->SetVisible(true);

	static bool redChargeReady = false;
	static bool redChargeReleased = false;

	if (medigunInfo.find(TFTeam_Red) != medigunInfo.end()) {
		panels["MedigunInfoRedBackground"]->SetVisible(true);

		const char *redname;

		switch (medigunInfo[TFTeam_Red].type) {
		case TFMedigun_MediGun:
		{
			redname = "Medi Gun";
			break;
		}
		case TFMedigun_Kritzkrieg:
		{
			redname = "Kritzkrieg";
			break;
		}
		case TFMedigun_QuickFix:
		{
			redname = "Quick-Fix";
			break;
		}
		case TFMedigun_Vaccinator:
		{
			redname = "Vaccinator";
			break;
		}
		case TFMedigun_Unknown:
		default:
		{
			redname = "Unknown";
			break;
		}
		}
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redname", redname);

		if (dynamic_meters->GetBool()) {
			KeyValues *medigunSettings = dynamicMeterSettings->FindKey(redname);

			if (medigunSettings) {
				panels["MedigunInfoRedChargeMeter"]->ApplySettings(medigunSettings);
				panels["MedigunInfoRedChargeMeter1"]->ApplySettings(medigunSettings);
				panels["MedigunInfoRedChargeMeter2"]->ApplySettings(medigunSettings);
				panels["MedigunInfoRedChargeMeter3"]->ApplySettings(medigunSettings);
				panels["MedigunInfoRedChargeMeter4"]->ApplySettings(medigunSettings);
			}
		}

		panels["MedigunInfoRedNameLabel"]->SetVisible(true);

		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge", int(floor(medigunInfo[TFTeam_Red].chargeLevel * 100.0f)));

		int redChargeAdvantage;

		if (medigunInfo.find(TFTeam_Blue) != medigunInfo.end()) {
			redChargeAdvantage = int(floor(medigunInfo[TFTeam_Red].chargeLevel * 100.0f)) - int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 100.0f));
		}
		else {
			redChargeAdvantage = int(floor(medigunInfo[TFTeam_Red].chargeLevel * 100.0f));
		}

		if (redChargeAdvantage > 0) {
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redadvantage", redChargeAdvantage);

			panels["MedigunInfoRedChargeAdvantageLabel"]->SetVisible(true);
		}
		else {
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redadvantage", 0);

			panels["MedigunInfoRedChargeAdvantageLabel"]->SetVisible(false);
		}

		switch (medigunInfo[TFTeam_Red].type) {
		case TFMedigun_Vaccinator:
		{
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharges", int(floor(medigunInfo[TFTeam_Red].chargeLevel * 4.0f)));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge1", int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 0.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 0.0f));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge2", int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 100.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 100.0f));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge3", int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 200.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 200.0f));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge4", int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 300.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Red].chargeLevel * 400.0f) - 300.0f));

			panels["MedigunInfoRedChargeLabel"]->SetVisible(false);
			panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(true);

			if (individual_charge_meters->GetBool()) {
				panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter1"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter2"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter3"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter4"]->SetVisible(true);
			}
			else {
				panels["MedigunInfoRedChargeMeter"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
			}

			if (medigunInfo[TFTeam_Red].chargeRelease) {
				if (!redChargeReleased) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeReleased");
				}

				redChargeReleased = true;
				redChargeReady = false;
			}
			else if (medigunInfo[TFTeam_Red].chargeLevel >= 0.25f) {
				if (!redChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeReady");
				}

				redChargeReleased = false;
				redChargeReady = true;
			}
			else {
				if (redChargeReleased || redChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeStop");
				}

				redChargeReleased = false;
				redChargeReady = false;
			}

			break;
		}
		case TFMedigun_MediGun:
		case TFMedigun_Kritzkrieg:
		case TFMedigun_QuickFix:
		case TFMedigun_Unknown:
		default:
		{
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharges", int(floor(medigunInfo[TFTeam_Red].chargeLevel)));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge1", 0);
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge2", 0);
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge3", 0);
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge4", 0);

			panels["MedigunInfoRedChargeLabel"]->SetVisible(false);
			panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(true);

			if (individual_charge_meters->GetBool()) {
				panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter1"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter2"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter3"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter4"]->SetVisible(true);
			}
			else {
				panels["MedigunInfoRedChargeMeter"]->SetVisible(true);
				panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
				panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
			}

			panels["MedigunInfoRedChargeMeter"]->SetVisible(true);
			panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
			panels["MedigunInfoRedChargeLabel"]->SetVisible(true);
			panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(false);

			if (medigunInfo[TFTeam_Red].chargeRelease) {
				if (!redChargeReleased) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeReleased");
				}

				redChargeReleased = true;
				redChargeReady = false;
			}
			else if (medigunInfo[TFTeam_Red].chargeLevel >= 1.0f) {
				if (!redChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeReady");
				}

				redChargeReleased = false;
				redChargeReady = true;
			}
			else {
				if (redChargeReleased || redChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeStop");
				}

				redChargeReleased = false;
				redChargeReady = false;
			}

			break;
		}
		}

		switch (medigunInfo[TFTeam_Red].type) {
		case TFMedigun_MediGun:
		{
			((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_UBERCHARGE).c_str());

			panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

			break;
		}
		case TFMedigun_Kritzkrieg:
		{
			((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_CRITBOOST).c_str());

			panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

			break;
		}
		case TFMedigun_QuickFix:
		{
			((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_MEGAHEALRED).c_str());

			panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

			break;
		}
		case TFMedigun_Vaccinator:
		{
			switch (medigunInfo[TFTeam_Red].chargeResistType) {
			case TFResistType_Bullet:
			{
				((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_BULLETRESISTRED).c_str());

				panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

				break;
			}
			case TFResistType_Explosive:
			{
				((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_BLASTRESISTRED).c_str());

				panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

				break;
			}
			case TFResistType_Fire:
			{
				((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_FIRERESISTRED).c_str());

				panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

				break;
			}
			default:
			{
				((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_NULL).c_str());

				panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(false);

				break;
			}
			}

			break;
		}
		case TFMedigun_Unknown:
		default:
		{
			((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_NULL).c_str());

			panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(false);

			break;
		}
		}
	}
	else {
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redname", "");
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharges", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge1", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge2", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge3", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redcharge4", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("redadvantage", 0);
		((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_NULL).c_str());

		panels["MedigunInfoRedNameLabel"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
		panels["MedigunInfoRedChargeLabel"]->SetVisible(false);
		panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(false);
		panels["MedigunInfoRedChargeAdvantageLabel"]->SetVisible(false);
		panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(false);

		if (redChargeReady || redChargeReleased) {
			redChargeReady = false;
			redChargeReleased = false;

			Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoRedChargeNormal");
		}
	}

	static bool bluChargeReady = false;
	static bool bluChargeReleased = false;

	if (medigunInfo.find(TFTeam_Blue) != medigunInfo.end()) {
		panels["MedigunInfoBluBackground"]->SetVisible(true);

		const char *bluname;

		switch (medigunInfo[TFTeam_Blue].type) {
		case TFMedigun_MediGun:
		{
			bluname = "Medi Gun";
			break;
		}
		case TFMedigun_Kritzkrieg:
		{
			bluname = "Kritzkrieg";
			break;
		}
		case TFMedigun_QuickFix:
		{
			bluname = "Quick-Fix";
			break;
		}
		case TFMedigun_Vaccinator:
		{
			bluname = "Vaccinator";
			break;
		}
		case TFMedigun_Unknown:
		default:
		{
			bluname = "Unknown";
			break;
		}
		}
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("bluname", bluname);

		if (dynamic_meters->GetBool()) {
			KeyValues *medigunSettings = dynamicMeterSettings->FindKey(bluname);

			if (medigunSettings) {
				panels["MedigunInfoBluChargeMeter"]->ApplySettings(medigunSettings);
				panels["MedigunInfoBluChargeMeter1"]->ApplySettings(medigunSettings);
				panels["MedigunInfoBluChargeMeter2"]->ApplySettings(medigunSettings);
				panels["MedigunInfoBluChargeMeter3"]->ApplySettings(medigunSettings);
				panels["MedigunInfoBluChargeMeter4"]->ApplySettings(medigunSettings);
			}
		}

		panels["MedigunInfoBluNameLabel"]->SetVisible(true);

		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge", int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 100.0f)));

		int bluChargeAdvantage;

		if (medigunInfo.find(TFTeam_Blue) != medigunInfo.end()) {
			bluChargeAdvantage = int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 100.0f)) - int(floor(medigunInfo[TFTeam_Red].chargeLevel * 100.0f));
		}
		else {
			bluChargeAdvantage = int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 100.0f));
		}

		if (bluChargeAdvantage > 0) {
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("bluadvantage", bluChargeAdvantage);

			panels["MedigunInfoBluChargeAdvantageLabel"]->SetVisible(true);
		}
		else {
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("bluadvantage", 0);

			panels["MedigunInfoBluChargeAdvantageLabel"]->SetVisible(false);
		}

		switch (medigunInfo[TFTeam_Blue].type) {
		case TFMedigun_Vaccinator:
		{
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharges", int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 4.0f)));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge1", int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 0.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 0.0f));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge2", int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 100.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 100.0f));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge3", int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 200.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 200.0f));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge4", int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 300.0f) < 0 ? 0 : int(floor(medigunInfo[TFTeam_Blue].chargeLevel * 400.0f) - 300.0f));

			panels["MedigunInfoBluChargeLabel"]->SetVisible(false);
			panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(true);

			if (individual_charge_meters->GetBool()) {
				panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter1"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter2"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter3"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter4"]->SetVisible(true);
			}
			else {
				panels["MedigunInfoBluChargeMeter"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
			}

			if (medigunInfo[TFTeam_Blue].chargeRelease) {
				if (!bluChargeReleased) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeReleased");
				}

				bluChargeReleased = true;
				bluChargeReady = false;
			}
			else if (medigunInfo[TFTeam_Blue].chargeLevel >= 0.25f) {
				if (!bluChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeReady");
				}

				bluChargeReleased = false;
				bluChargeReady = true;
			}
			else {
				if (bluChargeReleased || bluChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeStop");
				}

				bluChargeReleased = false;
				bluChargeReady = false;
			}

			break;
		}
		case TFMedigun_MediGun:
		case TFMedigun_Kritzkrieg:
		case TFMedigun_QuickFix:
		case TFMedigun_Unknown:
		default:
		{
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharges", int(floor(medigunInfo[TFTeam_Blue].chargeLevel)));
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge1", 0);
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge2", 0);
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge3", 0);
			((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge4", 0);

			panels["MedigunInfoBluChargeLabel"]->SetVisible(false);
			panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(true);

			if (individual_charge_meters->GetBool()) {
				panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter1"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter2"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter3"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter4"]->SetVisible(true);
			}
			else {
				panels["MedigunInfoBluChargeMeter"]->SetVisible(true);
				panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
				panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
			}

			panels["MedigunInfoBluChargeMeter"]->SetVisible(true);
			panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
			panels["MedigunInfoBluChargeLabel"]->SetVisible(true);
			panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(false);

			if (medigunInfo[TFTeam_Blue].chargeRelease) {
				if (!bluChargeReleased) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeReleased");
				}

				bluChargeReleased = true;
				bluChargeReady = false;
			}
			else if (medigunInfo[TFTeam_Blue].chargeLevel >= 1.0f) {
				if (!bluChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeReady");
				}

				bluChargeReleased = false;
				bluChargeReady = true;
			}
			else {
				if (bluChargeReleased || bluChargeReady) {
					Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeStop");
				}

				bluChargeReleased = false;
				bluChargeReady = false;
			}

			break;
		}
		}

		switch (medigunInfo[TFTeam_Blue].type) {
		case TFMedigun_MediGun:
		{
			((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_UBERCHARGE).c_str());

			panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

			break;
		}
		case TFMedigun_Kritzkrieg:
		{
			((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_CRITBOOST).c_str());

			panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

			break;
		}
		case TFMedigun_QuickFix:
		{
			((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_MEGAHEALBLU).c_str());

			panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

			break;
		}
		case TFMedigun_Vaccinator:
		{
			switch (medigunInfo[TFTeam_Blue].chargeResistType) {
			case TFResistType_Bullet:
			{
				((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_BULLETRESISTBLU).c_str());

				panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

				break;
			}
			case TFResistType_Explosive:
			{
				((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_BLASTRESISTBLU).c_str());

				panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

				break;
			}
			case TFResistType_Fire:
			{
				((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_FIRERESISTBLU).c_str());

				panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

				break;
			}
			default:
			{
				((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_NULL).c_str());

				panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(false);

				break;
			}
			}

			break;
		}
		case TFMedigun_Unknown:
		default:
		{
			((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_NULL).c_str());

			panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(false);

			break;
		}
		}
	}
	else {
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("bluname", "");
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharges", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge1", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge2", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge3", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("blucharge4", 0);
		((vgui::EditablePanel *) panels["MedigunInfo"])->SetDialogVariable("bluadvantage", 0);
		((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(GetVGUITexturePath(TEXTURE_NULL).c_str());

		panels["MedigunInfoBluNameLabel"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
		panels["MedigunInfoBluChargeLabel"]->SetVisible(false);
		panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(false);
		panels["MedigunInfoBluChargeAdvantageLabel"]->SetVisible(false);
		panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(false);

		if (bluChargeReady || bluChargeReleased) {
			bluChargeReady = false;
			bluChargeReleased = false;

			Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence("MedigunInfoBluChargeNormal");
		}
	}
}

void MedigunInfo::ReloadSettings() {
	if (panels.find("MedigunInfo") != panels.end()) {
		((vgui::EditablePanel *) panels["MedigunInfo"])->LoadControlSettings("Resource/UI/MedigunInfo.res");
	}

	dynamicMeterSettings = new KeyValues("MedigunInfoDynamicMeters");
	dynamicMeterSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/mediguninfodynamicmeters.res", "mod");
}

void MedigunInfo::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		InitHud();

		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &MedigunInfo::FrameHook), true);
		}

		if (panels.find("MedigunInfo") != panels.end()) {
			panels["MedigunInfo"]->SetEnabled(true);
			panels["MedigunInfo"]->SetVisible(true);
		}
	}
	else {
		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}

		if (panels.find("MedigunInfo") != panels.end()) {
			panels["MedigunInfo"]->SetEnabled(false);
			panels["MedigunInfo"]->SetVisible(false);
		}
	}
}