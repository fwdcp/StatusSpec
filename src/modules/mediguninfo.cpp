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

inline void FindAndReplaceInString(std::string &str, const std::string &find, const std::string &replace) {
	if (find.empty())
        return;

    size_t start_pos = 0;

    while((start_pos = str.find(find, start_pos)) != std::string::npos) {
        str.replace(start_pos, find.length(), replace);
        start_pos += replace.length();
    }
}

inline void StartAnimationSequence(const char *sequenceName) {
	Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence(sequenceName);
}

MedigunInfo::MedigunInfo() {
	mainPanel = vgui::INVALID_PANEL;

	charge_label_text = new ConVar("statusspec_mediguninfo_charge_label_text", "%charge%%", FCVAR_PRINTABLEONLY, "text for charge label in medigun info ('%charge%' is replaced with the current charge percentage number)");
	enabled = new ConVar("statusspec_mediguninfo_enabled", "0", FCVAR_NONE, "enable medigun info");
	individual_charge_meters = new ConVar("statusspec_mediguninfo_individual_charge_meters", "1", FCVAR_NONE, "enable individual charge meters (for Vaccinator)");
	individual_charges_label_text = new ConVar("statusspec_mediguninfo_individual_charges_label_text", "%charges%", FCVAR_PRINTABLEONLY, "text for individual charges label (for Vaccinator) in medigun info ('%charges%' is replaced with the current number of charges)");
	reload_settings = new ConCommand("statusspec_mediguninfo_reload_settings", MedigunInfo::ReloadSettings, "reload settings for the medigun info HUD from the resource file", FCVAR_NONE);
}

MedigunInfo::~MedigunInfo() {
	for (auto panel = panels.begin(); panel != panels.end(); ++panel) {
		vgui::VPANEL vPanel = panel->second->GetVPanel();
		panel->second->SetVisible(false);
		panel->second->SetEnabled(false);
		g_pVGui->FreePanel(vPanel);
		delete panel->second;
		panels.erase(panel);
	}
}

bool MedigunInfo::IsEnabled() {
	return enabled->GetBool();
}

void MedigunInfo::InitHud() {
	if (mainPanel == vgui::INVALID_PANEL) {
		for (auto panel = panels.begin(); panel != panels.end(); ++panel) {
			vgui::VPANEL vPanel = panel->second->GetVPanel();
			panel->second->SetVisible(false);
			panel->second->SetEnabled(false);
			g_pVGui->FreePanel(vPanel);
			delete panel->second;
			panels.erase(panel);
		}

		mainPanel = vgui::INVALID_PANEL;

		vgui::Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		vgui::EditablePanel *medigunInfoPanel = new vgui::EditablePanel(viewport, "MedigunInfo");
		panels["MedigunInfo"] = medigunInfoPanel;
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), medigunInfoPanel);
		
		panels["MedigunInfoBackground"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBackground");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBackground"]);
		
		panels["MedigunInfoRedBackground"] = new vgui::ScalableImagePanel(medigunInfoPanel, "MedigunInfoRedBackground");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedBackground"]);
		
		panels["MedigunInfoRedNameLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoRedNameLabel", "");
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
		
		panels["MedigunInfoRedChargeLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoRedChargeLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeLabel"]);
		
		panels["MedigunInfoRedIndividualChargesLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoRedIndividualChargesLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedIndividualChargesLabel"]);
		
		panels["MedigunInfoRedChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoRedChargeTypeIcon");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeTypeIcon"]);
		
		panels["MedigunInfoBluBackground"] = new vgui::ScalableImagePanel(medigunInfoPanel, "MedigunInfoBluBackground");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluBackground"]);
		
		panels["MedigunInfoBluNameLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoBluNameLabel", "");
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
		
		panels["MedigunInfoBluChargeLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoBluChargeLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeLabel"]);
		
		panels["MedigunInfoBluIndividualChargesLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoBluIndividualChargesLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluIndividualChargesLabel"]);
		
		panels["MedigunInfoBluChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBluChargeTypeIcon");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeTypeIcon"]);

		medigunInfoPanel->LoadControlSettings("Resource/UI/MedigunInfo.res");
	
		mainPanel = g_pVGui->PanelToHandle(medigunInfoPanel->GetVPanel());
	}
	
	panels["MedigunInfo"]->SetVisible(true);
}

void MedigunInfo::NoPaint(vgui::VPANEL vguiPanel) {
	if (g_pVGui->HandleToPanel(mainPanel) == vguiPanel) {
		panels["MedigunInfo"]->SetVisible(false);
	}
}

void MedigunInfo::Paint(vgui::VPANEL vguiPanel) {
	if (g_pVGui->HandleToPanel(mainPanel) == vguiPanel) {
		panels["MedigunInfo"]->SetVisible(true);
	
		static bool redChargeReady = false;
		static bool redChargeReleased = false;

		if (medigunInfo.find(TFTeam_Red) != medigunInfo.end()) {
			panels["MedigunInfoRedBackground"]->SetVisible(true);

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
					((vgui::Label *) panels["MedigunInfoRedNameLabel"])->SetText("Medi Gun");

					break;
				}
				case 35:	// Kritzkrieg
				{
					((vgui::Label *) panels["MedigunInfoRedNameLabel"])->SetText("Kritzkrieg");

					break;
				}
				case 411:	// Quick-Fix
				{
					((vgui::Label *) panels["MedigunInfoRedNameLabel"])->SetText("Quick-Fix");

					break;
				}
				case 998:	// Vaccinator
				{
					((vgui::Label *) panels["MedigunInfoRedNameLabel"])->SetText("Vaccinator");

					break;
				}
				default:
				{
					((vgui::Label *) panels["MedigunInfoRedNameLabel"])->SetText("Unknown");
					break;
				}
			}
			panels["MedigunInfoRedNameLabel"]->SetVisible(true);
			
			switch(medigunInfo[TFTeam_Red].itemDefinitionIndex) {
				case 998:	// Vaccinator
				{
					std::string redIndividualChargesLabelText = individual_charges_label_text->GetString();
					FindAndReplaceInString(redIndividualChargesLabelText, "%charges%", std::to_string(static_cast<long long>(floor(medigunInfo[TFTeam_Red].chargeLevel * 4.0f))));

					if (individual_charge_meters->GetBool()) {
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter1"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter2"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 1.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter3"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 2.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter4"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 3.0f);

						panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
						panels["MedigunInfoRedChargeMeter1"]->SetVisible(true);
						panels["MedigunInfoRedChargeMeter2"]->SetVisible(true);
						panels["MedigunInfoRedChargeMeter3"]->SetVisible(true);
						panels["MedigunInfoRedChargeMeter4"]->SetVisible(true);
					}
					else {
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter"])->SetProgress(medigunInfo[TFTeam_Red].chargeLevel);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter1"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter2"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter3"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter4"])->SetProgress(0.0f);

						panels["MedigunInfoRedChargeMeter"]->SetVisible(true);
						panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
						panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
						panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
						panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
					}

					((vgui::Label *) panels["MedigunInfoRedChargeLabel"])->SetText("");
					((vgui::Label *) panels["MedigunInfoRedIndividualChargesLabel"])->SetText(redIndividualChargesLabelText.c_str());

					panels["MedigunInfoRedChargeLabel"]->SetVisible(false);
					panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(true);

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
					std::string redChargeLabelText = charge_label_text->GetString();
					FindAndReplaceInString(redChargeLabelText, "%charge%", std::to_string(static_cast<long long>(floor(medigunInfo[TFTeam_Red].chargeLevel * 100))));
				
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter"])->SetProgress(medigunInfo[TFTeam_Red].chargeLevel);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter1"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter2"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter3"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter4"])->SetProgress(0.0f);
					((vgui::Label *) panels["MedigunInfoRedChargeLabel"])->SetText(redChargeLabelText.c_str());
					((vgui::Label *) panels["MedigunInfoRedIndividualChargesLabel"])->SetText("");
				
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
			((vgui::Label *) panels["MedigunInfoRedNameLabel"])->SetText("N/A");
			((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter1"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter2"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter3"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter4"])->SetProgress(0.0f);
			((vgui::Label *) panels["MedigunInfoRedChargeLabel"])->SetText("");
			((vgui::Label *) panels["MedigunInfoRedIndividualChargesLabel"])->SetText("");
			((vgui::ImagePanel *) panels["MedigunInfoRedChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);
		
			panels["MedigunInfoRedNameLabel"]->SetVisible(true);
			panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
			panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
			panels["MedigunInfoRedChargeLabel"]->SetVisible(false);
			panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(false);
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
					((vgui::Label *) panels["MedigunInfoBluNameLabel"])->SetText("Medi Gun");

					break;
				}
				case 35:	// Kritzkrieg
				{
					((vgui::Label *) panels["MedigunInfoBluNameLabel"])->SetText("Kritzkrieg");

					break;
				}
				case 411:	// Quick-Fix
				{
					((vgui::Label *) panels["MedigunInfoBluNameLabel"])->SetText("Quick-Fix");

					break;
				}
				case 998:	// Vaccinator
				{
					((vgui::Label *) panels["MedigunInfoBluNameLabel"])->SetText("Vaccinator");

					break;
				}
				default:
				{
					((vgui::Label *) panels["MedigunInfoBluNameLabel"])->SetText("Unknown");
					break;
				}
			}
			panels["MedigunInfoBluNameLabel"]->SetVisible(true);
			
			switch(medigunInfo[TFTeam_Blue].itemDefinitionIndex) {
				case 998:	// Vaccinator
				{
					std::string bluIndividualChargesLabelText = individual_charges_label_text->GetString();
					FindAndReplaceInString(bluIndividualChargesLabelText, "%charges%", std::to_string(static_cast<long long>(floor(medigunInfo[TFTeam_Blue].chargeLevel * 4.0f))));

					if (individual_charge_meters->GetBool()) {
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter1"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter2"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 1.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter3"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 2.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter4"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 3.0f);

						panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
						panels["MedigunInfoBluChargeMeter1"]->SetVisible(true);
						panels["MedigunInfoBluChargeMeter2"]->SetVisible(true);
						panels["MedigunInfoBluChargeMeter3"]->SetVisible(true);
						panels["MedigunInfoBluChargeMeter4"]->SetVisible(true);
					}
					else {
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter"])->SetProgress(medigunInfo[TFTeam_Red].chargeLevel);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter1"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter2"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter3"])->SetProgress(0.0f);
						((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter4"])->SetProgress(0.0f);

						panels["MedigunInfoBluChargeMeter"]->SetVisible(true);
						panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
						panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
						panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
						panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
					}

					((vgui::Label *) panels["MedigunInfoBluChargeLabel"])->SetText("");
					((vgui::Label *) panels["MedigunInfoBluIndividualChargesLabel"])->SetText(bluIndividualChargesLabelText.c_str());

					panels["MedigunInfoBluChargeLabel"]->SetVisible(false);
					panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(true);

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
					std::string bluChargeLabelText = charge_label_text->GetString();
					FindAndReplaceInString(bluChargeLabelText, "%charge%", std::to_string(static_cast<long long>(floor(medigunInfo[TFTeam_Blue].chargeLevel * 100))));
				
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter"])->SetProgress(medigunInfo[TFTeam_Blue].chargeLevel);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter1"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter2"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter3"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter4"])->SetProgress(0.0f);
					((vgui::Label *) panels["MedigunInfoBluChargeLabel"])->SetText(bluChargeLabelText.c_str());
					((vgui::Label *) panels["MedigunInfoBluIndividualChargesLabel"])->SetText("");
				
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
			((vgui::Label *) panels["MedigunInfoBluNameLabel"])->SetText("N/A");
			((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter1"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter2"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter3"])->SetProgress(0.0f);
			((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter4"])->SetProgress(0.0f);
			((vgui::Label *) panels["MedigunInfoBluChargeLabel"])->SetText("");
			((vgui::Label *) panels["MedigunInfoBluIndividualChargesLabel"])->SetText("");
			((vgui::ImagePanel *) panels["MedigunInfoBluChargeTypeIcon"])->SetImage(VGUI_TEXTURE_NULL);
		
			panels["MedigunInfoBluNameLabel"]->SetVisible(true);
			panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
			panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
			panels["MedigunInfoBluChargeLabel"]->SetVisible(false);
			panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(false);
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
	if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_TFPlayer")) {
		return;
	}

	TFTeam team = (TFTeam) *MAKE_PTR(int*, entity, Entities::pCTFPlayer__m_iTeamNum);

	if (team != TFTeam_Red && team != TFTeam_Blue) {
		return;
	}

	if (!Interfaces::GetGameResources()->IsAlive(entity->entindex())) {
		return;
	}

	for (int i = 0; i < MAX_WEAPONS; i++) {
		int weapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(entity, Entities::pCTFPlayer__m_hMyWeapons[i]);
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

void MedigunInfo::PostEntityUpdate() {
	InitHud();
}

void MedigunInfo::ReloadSettings() {
	if (g_MedigunInfo->panels.find("MedigunInfo") != g_MedigunInfo->panels.end()) {
		((vgui::EditablePanel *) g_MedigunInfo->panels["MedigunInfo"])->LoadControlSettings("Resource/UI/MedigunInfo.res");
	}
}