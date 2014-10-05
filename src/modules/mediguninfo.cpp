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

inline void StartAnimationSequence(const char *sequenceName) {
	// Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence(sequenceName);
}

MedigunInfo::MedigunInfo() {
	dynamicMeterSettings = new KeyValues("MedigunInfoDynamicMeters");
	dynamicMeterSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/mediguninfodynamicmeters.res", "mod");
	mainPanel = vgui::INVALID_PANEL;

	dynamic_meters = new ConVar("statusspec_mediguninfo_dynamic_meters", "0", FCVAR_NONE, "enable charge meters to change based on medigun");
	enabled = new ConVar("statusspec_mediguninfo_enabled", "0", FCVAR_NONE, "enable medigun info", MedigunInfo::ToggleEnabled);
	individual_charge_meters = new ConVar("statusspec_mediguninfo_individual_charge_meters", "1", FCVAR_NONE, "enable individual charge meters (for Vaccinator)");
	reload_settings = new ConCommand("statusspec_mediguninfo_reload_settings", MedigunInfo::ReloadSettings, "reload settings for the medigun info HUD from the resource file", FCVAR_NONE);
}

bool MedigunInfo::IsEnabled() {
	return enabled->GetBool();
}

void MedigunInfo::Paint(vgui::VPANEL vguiPanel) {
	if (g_pVGui->HandleToPanel(mainPanel) == vguiPanel) {
		panels["MedigunInfo"]->SetVisible(true);
	
		static bool redChargeReady = false;
		static bool redChargeReleased = false;

		if (medigunInfo.find(TFTeam_Red) != medigunInfo.end()) {
			panels["MedigunInfoRedBackground"]->SetVisible(true);

			const char *redname;

			switch(medigunInfo[TFTeam_Red].itemDefinitionIndex) {
				case 29:	// Medi Gun
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
					redname = "Medi Gun";
					break;
				}
				case 35:	// Kritzkrieg
				{
					redname = "Kritzkrieg";
					break;
				}
				case 411:	// Quick-Fix
				{
					redname = "Quick-Fix";
					break;
				}
				case 998:	// Vaccinator
				{
					redname = "Vaccinator";
					break;
				}
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

			switch(medigunInfo[TFTeam_Red].itemDefinitionIndex) {
				case 998:	// Vaccinator
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
							StartAnimationSequence("MedigunInfoRedChargeReleased");
						}

						redChargeReleased = true;
						redChargeReady = false;
					}
					else if (medigunInfo[TFTeam_Red].chargeLevel >= 0.25f) {
						if (!redChargeReady) {
							StartAnimationSequence("MedigunInfoRedChargeReady");
						}
					
						redChargeReleased = false;
						redChargeReady = true;
					}
					else {
						if (redChargeReleased || redChargeReady) {
							StartAnimationSequence("MedigunInfoRedChargeStop");
						}
					
						redChargeReleased = false;
						redChargeReady = false;
					}

					break;
				}
				case 29:	// Medi Gun
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
				case 35:	// Kritzkrieg
				case 411:	// Quick-Fix
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
							StartAnimationSequence("MedigunInfoRedChargeReleased");
						}

						redChargeReleased = true;
						redChargeReady = false;
					}
					else if (medigunInfo[TFTeam_Red].chargeLevel >= 1.0f) {
						if (!redChargeReady) {
							StartAnimationSequence("MedigunInfoRedChargeReady");
						}
					
						redChargeReleased = false;
						redChargeReady = true;
					}
					else {
						if (redChargeReleased || redChargeReady) {
							StartAnimationSequence("MedigunInfoRedChargeStop");
						}
					
						redChargeReleased = false;
						redChargeReady = false;
					}

					break;
				}
			}

			switch(medigunInfo[TFTeam_Red].itemDefinitionIndex) {
				case 29:	// Medi Gun
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
					((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_UBERCHARGE);

					panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

					break;
				}
				case 35:	// Kritzkrieg
				{
					((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_CRITBOOST);

					panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

					break;
				}
				case 411:	// Quick-Fix
				{
					((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_MEGAHEALRED);

					panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

					break;
				}
				case 998:	// Vaccinator
				{
					switch(medigunInfo[TFTeam_Red].chargeResistType) {
						case TFResistType_Bullet:
							((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_BULLETRESISTRED);

							panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

							break;
						case TFResistType_Explosive:
							((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_BLASTRESISTRED);

							panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

							break;
						case TFResistType_Fire:
							((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_FIRERESISTRED);

							panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(true);

							break;
						default:
							((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);

							panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(false);

							break;
					}

					break;
				}
				default:
				{
					((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);

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
			((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);
		
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

				StartAnimationSequence("MedigunInfoRedChargeNormal");
			}
		}
	
		static bool bluChargeReady = false;
		static bool bluChargeReleased = false;

		if (medigunInfo.find(TFTeam_Blue) != medigunInfo.end()) {
			panels["MedigunInfoBluBackground"]->SetVisible(true);
			
			const char *bluname;

			switch(medigunInfo[TFTeam_Blue].itemDefinitionIndex) {
				case 29:	// Medi Gun
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
					bluname = "Medi Gun";
					break;
				}
				case 35:	// Kritzkrieg
				{
					bluname = "Kritzkrieg";
					break;
				}
				case 411:	// Quick-Fix
				{
					bluname = "Quick-Fix";
					break;
				}
				case 998:	// Vaccinator
				{
					bluname = "Vaccinator";
					break;
				}
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

			switch(medigunInfo[TFTeam_Blue].itemDefinitionIndex) {
				case 998:	// Vaccinator
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
							StartAnimationSequence("MedigunInfoBluChargeReleased");
						}

						bluChargeReleased = true;
						bluChargeReady = false;
					}
					else if (medigunInfo[TFTeam_Blue].chargeLevel >= 0.25f) {
						if (!bluChargeReady) {
							StartAnimationSequence("MedigunInfoBluChargeReady");
						}
					
						bluChargeReleased = false;
						bluChargeReady = true;
					}
					else {
						if (bluChargeReleased || bluChargeReady) {
							StartAnimationSequence("MedigunInfoBluChargeStop");
						}
					
						bluChargeReleased = false;
						bluChargeReady = false;
					}

					break;
				}
				case 29:	// Medi Gun
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
				case 35:	// Kritzkrieg
				case 411:	// Quick-Fix
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
							StartAnimationSequence("MedigunInfoBluChargeReleased");
						}

						bluChargeReleased = true;
						bluChargeReady = false;
					}
					else if (medigunInfo[TFTeam_Blue].chargeLevel >= 1.0f) {
						if (!bluChargeReady) {
							StartAnimationSequence("MedigunInfoBluChargeReady");
						}
					
						bluChargeReleased = false;
						bluChargeReady = true;
					}
					else {
						if (bluChargeReleased || bluChargeReady) {
							StartAnimationSequence("MedigunInfoBluChargeStop");
						}
					
						bluChargeReleased = false;
						bluChargeReady = false;
					}

					break;
				}
			}

			switch(medigunInfo[TFTeam_Blue].itemDefinitionIndex) {
				case 29:	// Medi Gun
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
					((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_UBERCHARGE);

					panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

					break;
				}
				case 35:	// Kritzkrieg
				{
					((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_CRITBOOST);

					panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

					break;
				}
				case 411:	// Quick-Fix
				{
					((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_MEGAHEALBLU);

					panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

					break;
				}
				case 998:	// Vaccinator
				{
					switch(medigunInfo[TFTeam_Blue].chargeResistType) {
						case TFResistType_Bullet:
							((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_BULLETRESISTBLU);

							panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

							break;
						case TFResistType_Explosive:
							((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_BLASTRESISTBLU);

							panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

							break;
						case TFResistType_Fire:
							((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_FIRERESISTBLU);

							panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(true);

							break;
						default:
							((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);

							panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(false);

							break;
					}

					break;
				}
				default:
				{
					((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);

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
			((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);
		
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

				StartAnimationSequence("MedigunInfoBluChargeNormal");
			}
		}
	}
}

void MedigunInfo::PreEntityUpdate() {
	medigunInfo.clear();
}

void MedigunInfo::ProcessEntity(IClientEntity* entity) {
	Player player = entity;

	if (!player) {
		return;
	}

	TFTeam team = player.GetTeam();

	if (team != TFTeam_Red && team != TFTeam_Blue) {
		return;
	}

	if (!player.IsAlive()) {
		return;
	}

	for (int i = 0; i < MAX_WEAPONS; i++) {
		int weapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(player.GetEntity(), Entities::pCTFPlayer__m_hMyWeapons[i]);
		IClientEntity *weaponEntity = Interfaces::pClientEntityList->GetClientEntity(weapon);

		if (!weaponEntity || !Entities::CheckClassBaseclass(weaponEntity->GetClientClass(), "DT_WeaponMedigun")) {
			continue;
		}

		Medigun_t medigun;

		medigun.itemDefinitionIndex = *MAKE_PTR(int*, weaponEntity, Entities::pCEconEntity__m_iItemDefinitionIndex);
		medigun.chargeRelease = *MAKE_PTR(bool*, weaponEntity, Entities::pCWeaponMedigun__m_bChargeRelease);
		medigun.chargeResistType = *MAKE_PTR(int*, weaponEntity, Entities::pCWeaponMedigun__m_nChargeResistType);
		medigun.chargeLevel = *MAKE_PTR(float*, weaponEntity, Entities::pCWeaponMedigun__m_flChargeLevel);

		if (medigunInfo.find(team) == medigunInfo.end() || medigunInfo[team].chargeLevel <= 0.0f && medigun.chargeLevel >= 0.0f) {
			medigunInfo[team] = medigun;
		}
	}
}

void MedigunInfo::InitHud() {
	if (mainPanel == vgui::INVALID_PANEL) {
		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		if (viewport) {
			vgui::EditablePanel *medigunInfoPanel = new vgui::EditablePanel(viewport, "MedigunInfo");
			panels["MedigunInfo"] = medigunInfoPanel;
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), medigunInfoPanel);

			panels["MedigunInfoBackground"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBackground");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBackground"]);

			panels["MedigunInfoRedBackground"] = new vgui::ScalableImagePanel(medigunInfoPanel, "MedigunInfoRedBackground");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedBackground"]);

			panels["MedigunInfoRedNameLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedNameLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedNameLabel"]);

			panels["MedigunInfoRedChargeMeter"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter"]);

			panels["MedigunInfoRedChargeMeter1"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter1");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter1"]);

			panels["MedigunInfoRedChargeMeter2"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter2");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter2"]);

			panels["MedigunInfoRedChargeMeter3"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter3");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter3"]);

			panels["MedigunInfoRedChargeMeter4"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter4");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter4"]);

			panels["MedigunInfoRedChargeLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedChargeLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeLabel"]);

			panels["MedigunInfoRedIndividualChargesLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedIndividualChargesLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedIndividualChargesLabel"]);

			panels["MedigunInfoRedChargeAdvantageLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoRedChargeAdvantageLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeAdvantageLabel"]);

			panels["MedigunInfoRedChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoRedChargeTypeIcon");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeTypeIcon"]);

			panels["MedigunInfoBluBackground"] = new vgui::ScalableImagePanel(medigunInfoPanel, "MedigunInfoBluBackground");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluBackground"]);

			panels["MedigunInfoBluNameLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluNameLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluNameLabel"]);

			panels["MedigunInfoBluChargeMeter"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter"]);

			panels["MedigunInfoBluChargeMeter1"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter1");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter1"]);

			panels["MedigunInfoBluChargeMeter2"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter2");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter2"]);

			panels["MedigunInfoBluChargeMeter3"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter3");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter3"]);

			panels["MedigunInfoBluChargeMeter4"] = new vgui::ImageProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter4");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter4"]);

			panels["MedigunInfoBluChargeLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluChargeLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeLabel"]);

			panels["MedigunInfoBluIndividualChargesLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluIndividualChargesLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluIndividualChargesLabel"]);

			panels["MedigunInfoBluChargeAdvantageLabel"] = new vgui::VariableLabel(medigunInfoPanel, "MedigunInfoBluChargeAdvantageLabel", "");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeAdvantageLabel"]);

			panels["MedigunInfoBluChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBluChargeTypeIcon");
			g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeTypeIcon"]);

			medigunInfoPanel->LoadControlSettings("Resource/UI/MedigunInfo.res");

			mainPanel = g_pVGui->PanelToHandle(medigunInfoPanel->GetVPanel());
		}
	}
}

void MedigunInfo::ReloadSettings() {
	if (g_MedigunInfo->panels.find("MedigunInfo") != g_MedigunInfo->panels.end()) {
		((vgui::EditablePanel *) g_MedigunInfo->panels["MedigunInfo"])->LoadControlSettings("Resource/UI/MedigunInfo.res");
	}

	g_MedigunInfo->dynamicMeterSettings = new KeyValues("MedigunInfoDynamicMeters");
	g_MedigunInfo->dynamicMeterSettings->LoadFromFile(Interfaces::pFileSystem, "resource/ui/mediguninfodynamicmeters.res", "mod");
}

void MedigunInfo::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	bool enabled = g_MedigunInfo->IsEnabled();

	if (enabled) {
		g_MedigunInfo->InitHud();
	}

	if (g_MedigunInfo->panels.find("MedigunInfo") != g_MedigunInfo->panels.end()) {
		g_MedigunInfo->panels["MedigunInfo"]->SetEnabled(enabled);
		g_MedigunInfo->panels["MedigunInfo"]->SetVisible(enabled);
	}
}