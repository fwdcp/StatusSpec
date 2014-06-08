/*
 *  statusspec.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusspec.h"

#define SHOW_SLOT_ICON(slot) \
	if (playerInfo[i].slot != -1) { \
		if (playerInfo[i].activeWeaponSlot.compare(#slot) == 0) { \
			Paint::DrawTexture(itemIconTextures[playerInfo[i].slot], iconsWide, 0, iconSize, iconSize, loadout_active_filter); \
		} \
		else { \
			Paint::DrawTexture(itemIconTextures[playerInfo[i].slot], iconsWide, 0, iconSize, iconSize, loadout_nonactive_filter); \
		} \
	} \
	 \
	iconsWide += iconSize;

SourceHook::Impl::CSourceHookImpl g_SourceHook;
SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

SH_DECL_HOOK1(IGameResources, GetPlayerName, SH_NOATTRIB, 0, const char *, int);
SH_DECL_HOOK3_void(IPanel, PaintTraverse, SH_NOATTRIB, 0, VPANEL, bool, bool);
SH_DECL_HOOK3_void(IPanel, SendMessage, SH_NOATTRIB, 0, VPANEL, KeyValues *, VPANEL);
SH_DECL_HOOK2(IVEngineClient, GetPlayerInfo, SH_NOATTRIB, 0, bool, int, player_info_t *);

std::map<int, std::string> itemIconTextures;

ConVar loadout_icons_enabled("statusspec_loadout_icons_enabled", "0", 0, "enable loadout icons");
ConVar loadout_icons_nonloadout("statusspec_loadout_icons_nonloadout", "0", 0, "enable loadout icons for nonloadout items");
ConVar medigun_info_charge_label_text("statusspec_medigun_info_charge_label_text", "%charge%%", 0, "text for charge label in medigun info ('%charge%' is replaced with the current charge percentage number)");
ConVar medigun_info_enabled("statusspec_medigun_info_enabled", "0", 0, "enable medigun info");
ConVar medigun_info_individual_charges_label_text("statusspec_medigun_info_individual_charges_label_text", "%charges%", 0, "text for individual charges label (for Vaccinator) in medigun info ('%charges%' is replaced with the current number of charges)");
ConVar player_aliases_enabled("statusspec_player_alias_enabled", "0", 0, "enable player aliases");
ConVar status_icons_enabled("statusspec_status_icons_enabled", "0", 0, "enable status icons");
ConVar status_icons_max("statusspec_status_icons_max", "5", 0, "max number of status icons to be rendered");

inline int ColorRangeRestrict(int color) {
	if (color < 0) return 0;
	else if (color > 255) return 255;
	else return color;
}

inline void FindAndReplaceInString(std::string &str, const std::string &find, const std::string &replace) {
	if (find.empty())
        return;

    size_t start_pos = 0;

    while((start_pos = str.find(find, start_pos)) != std::string::npos) {
        str.replace(start_pos, find.length(), replace);
        start_pos += replace.length();
    }
}

inline bool IsInteger(const std::string &s) {
   if (s.empty() || !isdigit(s[0])) return false;

   char *p;
   strtoull(s.c_str(), &p, 10);

   return (*p == 0);
}

inline void StartAnimationSequence(const char *sequenceName) {
	Interfaces::GetClientMode()->GetViewportAnimationController()->StartAnimationSequence(sequenceName);
}

bool CheckCondition(uint32_t conditions[3], int condition) {
	if (condition < 32) {
		if (conditions[0] & (1 << condition)) {
			return true;
		}
	}
	else if (condition < 64) {
		if (conditions[1] & (1 << (condition - 32))) {
			return true;
		}
	}
	else if (condition < 96) {
		if (conditions[2] & (1 << (condition - 64))) {
			return true;
		}
	}
	
	return false;
}

CSteamID ConvertTextToSteamID(std::string textID) {
	if (textID.substr(0, 6).compare("STEAM_") == 0 && std::count(textID.begin(), textID.end(), ':') == 2) {
		std::stringstream ss(textID);
		std::string universe;
		std::string server;
		std::string authID;
		std::getline(ss, universe, ':');
		std::getline(ss, server, ':');
		std::getline(ss, authID, ':');

		if (IsInteger(server) && IsInteger(authID)) {
			uint32_t accountID = (2 * strtoul(authID.c_str(), NULL, 10)) + strtoul(server.c_str(), NULL, 10);

			static EUniverse universe = k_EUniverseInvalid;

			if (universe == k_EUniverseInvalid) {
				universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
			}

			return CSteamID(accountID, universe, k_EAccountTypeIndividual);
		}

		return CSteamID();
	}
	else if (IsInteger(textID)) {
		uint64_t steamID = strtoull(textID.c_str(), NULL, 10);

		return CSteamID(steamID);
	}

	return CSteamID();
}

void DisplayMedigunInfo() {
	if (panels.find("MedigunInfo") == panels.end()) {
		Panel *viewport = Interfaces::GetClientMode()->GetViewport();

		vgui::EditablePanel *medigunInfoPanel = new vgui::EditablePanel(viewport, "MedigunInfo");
		panels["MedigunInfo"] = medigunInfoPanel;
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), medigunInfoPanel);
		
		panels["MedigunInfoBackground"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBackground");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBackground"]);
		
		panels["MedigunInfoRedBackground"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoRedBackground");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedBackground"]);
		
		panels["MedigunInfoRedNameLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoRedNameLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedNameLabel"]);
		
		panels["MedigunInfoRedChargeMeter"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter"]);
		
		panels["MedigunInfoRedChargeMeter1"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter1");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter1"]);
		
		panels["MedigunInfoRedChargeMeter2"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter2");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter2"]);
		
		panels["MedigunInfoRedChargeMeter3"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter3");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter3"]);
		
		panels["MedigunInfoRedChargeMeter4"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoRedChargeMeter4");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeMeter4"]);
		
		panels["MedigunInfoRedChargeLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoRedChargeLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeLabel"]);
		
		panels["MedigunInfoRedIndividualChargesLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoRedIndividualChargesLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedIndividualChargesLabel"]);
		
		panels["MedigunInfoRedChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoRedChargeTypeIcon");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoRedChargeTypeIcon"]);
		
		panels["MedigunInfoBluBackground"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBluBackground");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluBackground"]);
		
		panels["MedigunInfoBluNameLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoBluNameLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluNameLabel"]);
		
		panels["MedigunInfoBluChargeMeter"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter"]);
		
		panels["MedigunInfoBluChargeMeter1"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter1");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter1"]);
		
		panels["MedigunInfoBluChargeMeter2"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter2");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter2"]);
		
		panels["MedigunInfoBluChargeMeter3"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter3");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter3"]);
		
		panels["MedigunInfoBluChargeMeter4"] = new vgui::ContinuousProgressBar(medigunInfoPanel, "MedigunInfoBluChargeMeter4");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeMeter4"]);
		
		panels["MedigunInfoBluChargeLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoBluChargeLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeLabel"]);
		
		panels["MedigunInfoBluIndividualChargesLabel"] = new vgui::Label(medigunInfoPanel, "MedigunInfoBluIndividualChargesLabel", "");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluIndividualChargesLabel"]);
		
		panels["MedigunInfoBluChargeTypeIcon"] = new vgui::ImagePanel(medigunInfoPanel, "MedigunInfoBluChargeTypeIcon");
		g_pVGuiPanel->Init(g_pVGui->AllocPanel(), panels["MedigunInfoBluChargeTypeIcon"]);

		medigunInfoPanel->LoadControlSettings("Resource/UI/MedigunInfo.res");
	}
	
	if (medigun_info_enabled.GetBool()) {
		panels["MedigunInfo"]->SetVisible(true);
		panels["MedigunInfoBackground"]->SetVisible(true);
		panels["MedigunInfoRedBackground"]->SetVisible(true);
		panels["MedigunInfoBluBackground"]->SetVisible(true);
	
		static bool redChargeReady = false;
		static bool redChargeReleased = false;

		if (medigunInfo.find(TFTeam_Red) != medigunInfo.end()) {
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
					std::string redIndividualChargesLabelText = medigun_info_individual_charges_label_text.GetString();
					FindAndReplaceInString(redIndividualChargesLabelText, "%charges%", std::to_string(static_cast<long long>(floor(medigunInfo[TFTeam_Red].chargeLevel * 4.0f))));

					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter1"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter2"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 1.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter3"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 2.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoRedChargeMeter4"])->SetProgress((medigunInfo[TFTeam_Red].chargeLevel * 4.0f) - 3.0f);
					((vgui::Label *) panels["MedigunInfoRedChargeLabel"])->SetText("");
					((vgui::Label *) panels["MedigunInfoRedIndividualChargesLabel"])->SetText(redIndividualChargesLabelText.c_str());
				
					panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
					panels["MedigunInfoRedChargeMeter1"]->SetVisible(true);
					panels["MedigunInfoRedChargeMeter2"]->SetVisible(true);
					panels["MedigunInfoRedChargeMeter3"]->SetVisible(true);
					panels["MedigunInfoRedChargeMeter4"]->SetVisible(true);
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
					std::string redChargeLabelText = medigun_info_charge_label_text.GetString();
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
					std::string bluIndividualChargesLabelText = medigun_info_individual_charges_label_text.GetString();
					FindAndReplaceInString(bluIndividualChargesLabelText, "%charges%", std::to_string(static_cast<long long>(floor(medigunInfo[TFTeam_Blue].chargeLevel * 4.0f))));

					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter"])->SetProgress(0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter1"])->SetProgress((medigunInfo[TFTeam_Blue].chargeLevel * 4.0f) - 0.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter2"])->SetProgress((medigunInfo[TFTeam_Blue].chargeLevel * 4.0f) - 1.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter3"])->SetProgress((medigunInfo[TFTeam_Blue].chargeLevel * 4.0f) - 2.0f);
					((vgui::ContinuousProgressBar *) panels["MedigunInfoBluChargeMeter4"])->SetProgress((medigunInfo[TFTeam_Blue].chargeLevel * 4.0f) - 3.0f);
					((vgui::Label *) panels["MedigunInfoBluChargeLabel"])->SetText("");
					((vgui::Label *) panels["MedigunInfoBluIndividualChargesLabel"])->SetText(bluIndividualChargesLabelText.c_str());
				
					panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
					panels["MedigunInfoBluChargeMeter1"]->SetVisible(true);
					panels["MedigunInfoBluChargeMeter2"]->SetVisible(true);
					panels["MedigunInfoBluChargeMeter3"]->SetVisible(true);
					panels["MedigunInfoBluChargeMeter4"]->SetVisible(true);
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
					std::string bluChargeLabelText = medigun_info_charge_label_text.GetString();
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
	else {
		panels["MedigunInfo"]->SetVisible(false);
		panels["MedigunInfoBackground"]->SetVisible(false);
		panels["MedigunInfoRedBackground"]->SetVisible(false);
		panels["MedigunInfoRedNameLabel"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter1"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter2"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter3"]->SetVisible(false);
		panels["MedigunInfoRedChargeMeter4"]->SetVisible(false);
		panels["MedigunInfoRedChargeLabel"]->SetVisible(false);
		panels["MedigunInfoRedIndividualChargesLabel"]->SetVisible(false);
		panels["MedigunInfoRedChargeTypeIcon"]->SetVisible(false);
		panels["MedigunInfoBluBackground"]->SetVisible(false);
		panels["MedigunInfoBluNameLabel"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter1"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter2"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter3"]->SetVisible(false);
		panels["MedigunInfoBluChargeMeter4"]->SetVisible(false);
		panels["MedigunInfoBluChargeLabel"]->SetVisible(false);
		panels["MedigunInfoBluIndividualChargesLabel"]->SetVisible(false);
		panels["MedigunInfoBluChargeTypeIcon"]->SetVisible(false);
	}
}

CSteamID GetClientSteamID(int client) {
	player_info_t playerInfo;

	if (SH_CALL(Interfaces::pEngineClient, &IVEngineClient::GetPlayerInfo)(client, &playerInfo))
	{
		if (playerInfo.friendsID)
		{
			static EUniverse universe = k_EUniverseInvalid;

			if (universe == k_EUniverseInvalid) {
				universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
			}

			return CSteamID(playerInfo.friendsID, 1, universe, k_EAccountTypeIndividual);
		}
	}

	return CSteamID();
}

void UpdateEntities() {
	static IGameResources* gameResources = NULL;
	static int getPlayerNameHook;

	if (gameResources != Interfaces::GetGameResources()) {
		if (getPlayerNameHook) {
			SH_REMOVE_HOOK_ID(getPlayerNameHook);
			getPlayerNameHook = 0;
		}

		gameResources = Interfaces::GetGameResources();
		
		if (gameResources) {
			getPlayerNameHook = SH_ADD_HOOK(IGameResources, GetPlayerName, gameResources, Hook_IGameResources_GetPlayerName, true);
		}
	}

	int iEntCount = Interfaces::pClientEntityList->GetHighestEntityIndex();
	IClientEntity *cEntity;
	
	playerInfo.clear();
	medigunInfo.clear();

	for (int i = 0; i < iEntCount; i++) {
		cEntity = Interfaces::pClientEntityList->GetClientEntity(i);
		
		if (!cEntity) {
			continue;
		}
		
		if (Interfaces::GetGameResources()->IsConnected(i)) {
			int tfclass = *MAKE_PTR(int*, cEntity, Offsets::pCTFPlayer__m_iClass);
			int team = *MAKE_PTR(int*, cEntity, Offsets::pCTFPlayer__m_iTeamNum);
			uint32_t playerCond = *MAKE_PTR(uint32_t*, cEntity, Offsets::pCTFPlayer__m_nPlayerCond);
			uint32_t condBits = *MAKE_PTR(uint32_t*, cEntity, Offsets::pCTFPlayer___condition_bits);
			uint32_t playerCondEx = *MAKE_PTR(uint32_t*, cEntity, Offsets::pCTFPlayer__m_nPlayerCondEx);
			uint32_t playerCondEx2 = *MAKE_PTR(uint32_t*, cEntity, Offsets::pCTFPlayer__m_nPlayerCondEx2);
			Vector origin = cEntity->GetAbsOrigin();
			QAngle angles = cEntity->GetAbsAngles();
			
			if (team != TFTeam_Red && team != TFTeam_Blue) {
				continue;
			}

			playerInfo[i].tfclass = (TFClassType) tfclass;
			playerInfo[i].team = (TFTeam) team;
			playerInfo[i].conditions[0] = playerCond|condBits;
			playerInfo[i].conditions[1] = playerCondEx;
			playerInfo[i].conditions[2] = playerCondEx2;
			playerInfo[i].primary = -1;
			playerInfo[i].secondary = -1;
			playerInfo[i].melee = -1;
			playerInfo[i].pda = -1;
			playerInfo[i].pda2 = -1;
			playerInfo[i].building = -1;
			std::fill(playerInfo[i].cosmetic, playerInfo[i].cosmetic + MAX_COSMETIC_SLOTS, -1);
			playerInfo[i].action = -1;
		}
		else if (Offsets::CheckClassBaseclass(cEntity->GetClientClass(), "DT_EconEntity")) {
			int player = ENTITY_INDEX_FROM_ENTITY_OFFSET(cEntity, Offsets::pCEconEntity__m_hOwnerEntity);
			IClientEntity *playerEntity = Interfaces::pClientEntityList->GetClientEntity(player);
			int itemDefinitionIndex = *MAKE_PTR(int*, cEntity, Offsets::pCEconEntity__m_iItemDefinitionIndex);

			if (loadout_icons_enabled.GetBool()) {
				int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, Offsets::pCTFPlayer__m_hActiveWeapon);

				const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");
			
				KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
				if (classUses) {
					const char *classUse = classUses->GetString(tfclassNames[playerInfo[player].tfclass].c_str(), "");

					if (std::find(std::begin(itemSlots), std::end(itemSlots), classUse) != std::end(itemSlots)) {
						itemSlot = classUse;
					}
				}

				if (activeWeapon == i) {
					playerInfo[player].activeWeaponSlot = itemSlot;
				}
			
				if (strcmp(itemSlot, "primary") == 0) {
					playerInfo[player].primary = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "secondary") == 0) {
					playerInfo[player].secondary = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "melee") == 0) {
					playerInfo[player].melee = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "pda") == 0) {
					playerInfo[player].pda = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "pda2") == 0) {
					playerInfo[player].pda2 = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "building") == 0) {
					playerInfo[player].building = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "head") == 0 || strcmp(itemSlot, "misc") == 0) {
					for (int slot = 0; slot < 3; slot++) {
						if (playerInfo[player].cosmetic[slot] == -1) {
							playerInfo[player].cosmetic[slot] = itemDefinitionIndex;
							break;
						}
					}
				}
				else if (strcmp(itemSlot, "action") == 0) {
					playerInfo[player].action = itemDefinitionIndex;
				}
				
				const char *itemIcon = itemSchema->GetItemKeyData(itemDefinitionIndex, "image_inventory");
				Paint::InitializeTexture(itemIcon);
				itemIconTextures[itemDefinitionIndex] = itemIcon;
			}

			if (medigun_info_enabled.GetBool() && Offsets::CheckClassBaseclass(cEntity->GetClientClass(), "DT_WeaponMedigun")) {
				TFTeam team = (TFTeam) *MAKE_PTR(int*, playerEntity, Offsets::pCTFPlayer__m_iTeamNum);

				medigunInfo[team].itemDefinitionIndex = itemDefinitionIndex;
				medigunInfo[team].chargeRelease = *MAKE_PTR(bool*, cEntity, Offsets::pCWeaponMedigun__m_bChargeRelease);
				medigunInfo[team].chargeResistType = *MAKE_PTR(int*, cEntity, Offsets::pCWeaponMedigun__m_nChargeResistType);
				medigunInfo[team].chargeLevel = *MAKE_PTR(float*, cEntity, Offsets::pCWeaponMedigun__m_flChargeLevel);
			}
		}
	}
	
	if (loadout_icons_enabled.GetBool()) {
		for (std::map<int, Player>::iterator iterator = playerInfo.begin(); iterator != playerInfo.end(); iterator++) {
			int player = iterator->first;
			IClientEntity *playerEntity = Interfaces::pClientEntityList->GetClientEntity(player);
			
			if (!playerEntity) {
				continue;
			}
			
			int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, Offsets::pCTFPlayer__m_hActiveWeapon);
			
			for (int i = 0; i < MAX_WEAPONS; i++) {
				int weapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, Offsets::pCTFPlayer__m_hMyWeapons[i]);
				IClientEntity *weaponEntity = Interfaces::pClientEntityList->GetClientEntity(weapon);
			
				if (!weaponEntity || !Offsets::CheckClassBaseclass(weaponEntity->GetClientClass(), "DT_EconEntity")) {
					continue;
				}
				
				int itemDefinitionIndex = *MAKE_PTR(int*, weaponEntity, Offsets::pCEconEntity__m_iItemDefinitionIndex);
			
				const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");
				
				KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
				if (classUses) {
					const char *classUse = classUses->GetString(tfclassNames[playerInfo[player].tfclass].c_str(), "");

					if (std::find(std::begin(itemSlots), std::end(itemSlots), classUse) != std::end(itemSlots)) {
						itemSlot = classUse;
					}
				}

				if (activeWeapon == i) {
					playerInfo[player].activeWeaponSlot = itemSlot;
				}
				
				if (strcmp(itemSlot, "primary") == 0) {
					playerInfo[player].primary = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "secondary") == 0) {
					playerInfo[player].secondary = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "melee") == 0) {
					playerInfo[player].melee = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "pda") == 0) {
					playerInfo[player].pda = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "pda2") == 0) {
					playerInfo[player].pda2 = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "building") == 0) {
					playerInfo[player].building = itemDefinitionIndex;
				}
				else if (strcmp(itemSlot, "head") == 0 || strcmp(itemSlot, "misc") == 0) {
					for (int slot = 0; slot < 3; slot++) {
						if (playerInfo[player].cosmetic[slot] == -1) {
							playerInfo[player].cosmetic[slot] = itemDefinitionIndex;
							break;
						}
					}
				}
				else if (strcmp(itemSlot, "action") == 0) {
					playerInfo[player].action = itemDefinitionIndex;
				}
			}
		}
	}
}

CON_COMMAND(statusspec_loadout_filter_active, "the RGBA filter applied to the icon when the item is active") {
	if (args.ArgC() < 4 || !IsInteger(args.Arg(1)) || !IsInteger(args.Arg(2)) || !IsInteger(args.Arg(3)) || !IsInteger(args.Arg(4)))
	{
		Warning("Usage: statusspec_loadout_filter_active <red> <green> <blue> <alpha>\n");
		return;
	}
	
	int red = ColorRangeRestrict(std::stoi(args.Arg(1)));
	int green = ColorRangeRestrict(std::stoi(args.Arg(2)));
	int blue = ColorRangeRestrict(std::stoi(args.Arg(3)));
	int alpha = ColorRangeRestrict(std::stoi(args.Arg(4)));
	
	loadout_active_filter.SetColor(red, green, blue, alpha);
	Msg("Set nonactive loadout icon filter to rgba(%i, %i, %i, %i).\n", red, green, blue, alpha);
}

CON_COMMAND(statusspec_loadout_filter_nonactive, "the RGBA filter applied to the icon when the item is not active") {
	if (args.ArgC() < 4 || !IsInteger(args.Arg(1)) || !IsInteger(args.Arg(2)) || !IsInteger(args.Arg(3)) || !IsInteger(args.Arg(4)))
	{
		Warning("Usage: statusspec_loadout_filter_nonactive <red> <green> <blue> <alpha>\n");
		return;
	}
	
	int red = ColorRangeRestrict(std::stoi(args.Arg(1)));
	int green = ColorRangeRestrict(std::stoi(args.Arg(2)));
	int blue = ColorRangeRestrict(std::stoi(args.Arg(3)));
	int alpha = ColorRangeRestrict(std::stoi(args.Arg(4)));
	
	loadout_nonactive_filter.SetColor(red, green, blue, alpha);
	Msg("Set nonactive loadout item icon filter to rgba(%i, %i, %i, %i).\n", red, green, blue, alpha);
}

CON_COMMAND(statusspec_medigun_info_reload_settings, "reload settings for the medigun info HUD from the resource file") {
	((vgui::EditablePanel *) panels["MedigunInfo"])->LoadControlSettings("Resource/UI/MedigunInfo.res");
}

CON_COMMAND(statusspec_player_alias_get, "get an alias for a player") {
	if (args.ArgC() < 1)
	{
		Warning("Usage: statusspec_player_alias_get <steamid>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(args.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	if (playerAliases.find(playerSteamID) != playerAliases.end()) {
		Msg("Steam ID %llu has an associated alias '%s'.\n", playerSteamID.ConvertToUint64(), playerAliases[playerSteamID].c_str());
	}
	else {
		Msg("Steam ID %llu does not have an associated alias.\n", playerSteamID.ConvertToUint64());
	}
}

CON_COMMAND(statusspec_player_alias_remove, "remove an alias for a player") {
	if (args.ArgC() < 1)
	{
		Warning("Usage: statusspec_player_alias_remove <steamid>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(args.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	playerAliases.erase(playerSteamID);
	Msg("Alias associated with Steam ID %llu erased.\n", playerSteamID.ConvertToUint64());
}

CON_COMMAND(statusspec_player_alias_set, "set an alias for a player") {
	if (args.ArgC() < 2)
	{
		Warning("Usage: statusspec_player_alias_set <steamid> <alias>\n");
		return;
	}

	CSteamID playerSteamID = ConvertTextToSteamID(args.Arg(1));

	if (!playerSteamID.IsValid()) {
		Warning("The Steam ID entered is invalid.\n");
		return;
	}

	playerAliases[playerSteamID] = args.Arg(2);
	Msg("Steam ID %llu has been associated with alias '%s'.\n", playerSteamID.ConvertToUint64(), playerAliases[playerSteamID].c_str());
}

CON_COMMAND(statusspec_utility_set_progress_bar_direction, "set the progress direction for a StatusSpec progress bar") {
	if (args.ArgC() < 2)
	{
		Warning("Usage: statusspec_utility_set_progress_bar_direction <name> <direction>\n");
		return;
	}

	if (panels.find(args.Arg(1)) == panels.end()) {
		Warning("Invalid panel specified!\n");
		return;
	}

	vgui::ProgressBar *progressBar = dynamic_cast<vgui::ProgressBar *>(panels[args.Arg(1)]);

	if (!progressBar) {
		Warning("Panel is not a progress bar!\n");
		return;
	}

	if (stricmp(args.Arg(2), "east") == 0) {
		progressBar->SetProgressDirection(vgui::ProgressBar::PROGRESS_EAST);
		Msg("Direction of progress bar %s set to east.\n", args.Arg(1));
		return;
	}
	else if (stricmp(args.Arg(2), "west") == 0) {
		progressBar->SetProgressDirection(vgui::ProgressBar::PROGRESS_WEST);
		Msg("Direction of progress bar %s set to west.\n", args.Arg(1));
		return;
	}
	else if (stricmp(args.Arg(2), "north") == 0) {
		progressBar->SetProgressDirection(vgui::ProgressBar::PROGRESS_NORTH);
		Msg("Direction of progress bar %s set to north.\n", args.Arg(1));
		return;
	}
	else if (stricmp(args.Arg(2), "south") == 0) {
		progressBar->SetProgressDirection(vgui::ProgressBar::PROGRESS_SOUTH);
		Msg("Direction of progress bar %s set to south.\n", args.Arg(1));
		return;
	}
	else {
		Warning("Invalid direction! (Valid directions are east, west, north, south.)\n");
		return;
	}
}

const char * Hook_IGameResources_GetPlayerName(int client) {
	if (player_aliases_enabled.GetBool()) {
		CSteamID playerSteamID = GetClientSteamID(client);

		if (playerAliases.find(playerSteamID) != playerAliases.end()) {
			RETURN_META_VALUE(MRES_SUPERCEDE, playerAliases[playerSteamID].c_str());
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, "");
}
	
void Hook_IPanel_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (g_AntiFreeze) {
		g_AntiFreeze->Paint(vguiPanel);
	}

	const char* panelName = g_pVGuiPanel->GetName(vguiPanel);
	vgui::HPanel panelHandle = g_pVGui->PanelToHandle(vguiPanel);

	static vgui::HPanel topPanelHandle = vgui::INVALID_PANEL;

	if (topPanelHandle == vgui::INVALID_PANEL && strcmp(panelName, "MatSystemTopPanel") == 0) {
		topPanelHandle = panelHandle;
	}

	if (topPanelHandle == panelHandle) {
		UpdateEntities();
		DisplayMedigunInfo();
	}

	if (Interfaces::pEngineClient->IsDrawingLoadingImage() || !Interfaces::pEngineClient->IsInGame() || !Interfaces::pEngineClient->IsConnected() || Interfaces::pEngineClient->Con_IsVisible())
		return;
	
	if (strcmp(panelName, "statusicons") == 0) {
		vgui::VPANEL playerPanel = g_pVGuiPanel->GetParent(vguiPanel);
		int iconsWide, iconsTall, playerWide, playerTall;
		
		g_pVGuiPanel->GetSize(vguiPanel, iconsWide, iconsTall);
		g_pVGuiPanel->GetSize(playerPanel, playerWide, playerTall);
		
		playerWide -= iconsWide;
		iconsWide -= iconsWide;
		
		int iconSize = iconsTall;
		int icons = 0;
		int maxIcons = status_icons_max.GetInt();
		
		g_pVGuiPanel->SetSize(vguiPanel, iconsWide, iconsTall);
		g_pVGuiPanel->SetSize(playerPanel, playerWide, playerTall);
		
		if (!status_icons_enabled.GetBool()) {
			return;
		}
		
		const char *playerPanelName = g_pVGuiPanel->GetName(playerPanel);
		
		if (playerPanels.find(playerPanelName) == playerPanels.end()) {
			return;
		}
		
		int i = playerPanels[playerPanelName];
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_Ubercharged)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_UBERCHARGE, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_Kritzkrieged)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_CRITBOOST, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_MegaHeal)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_MEGAHEALRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_MEGAHEALBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_UberBulletResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDRED, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BULLETRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDBLU, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BULLETRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallBulletResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BULLETRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BULLETRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_UberBlastResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDRED, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BLASTRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDBLU, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_BLASTRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallBlastResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BLASTRESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BLASTRESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_UberFireResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDRED, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_FIRERESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_RESISTSHIELDBLU, iconsWide, 0, iconSize, iconSize);
				Paint::DrawTexture(TEXTURE_FIRERESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallFireResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_FIRERESISTRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_FIRERESISTBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_Buffed)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BUFFBANNERRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BUFFBANNERBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_DefenseBuffed)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_BATTALIONSBACKUPRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_BATTALIONSBACKUPBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_RegenBuffed)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				Paint::DrawTexture(TEXTURE_CONCHERORRED, iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				Paint::DrawTexture(TEXTURE_CONCHERORBLU, iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_Jarated)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_JARATE, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_Milked)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_MADMILK, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_MarkedForDeath) || CheckCondition(playerInfo[i].conditions, TFCond_MarkedForDeathSilent)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_MARKFORDEATH, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_Bleeding)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_BLEEDING, iconsWide, 0, iconSize, iconSize, Color(255, 0, 0, 0));
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
		
		if (CheckCondition(playerInfo[i].conditions, TFCond_OnFire)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			Paint::DrawTexture(TEXTURE_FIRE, iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
	}
	else if (strcmp(panelName, "loadouticons") == 0) {
		if (!loadout_icons_enabled.GetBool()) {
			return;
		}

		vgui::VPANEL playerPanel = g_pVGuiPanel->GetParent(vguiPanel);
		const char *playerPanelName = g_pVGuiPanel->GetName(playerPanel);
		
		if (playerPanels.find(playerPanelName) == playerPanels.end()) {
			return;
		}
		
		int i = playerPanels[playerPanelName];
		
		int iconsWide, iconsTall;
		
		g_pVGuiPanel->GetSize(vguiPanel, iconsWide, iconsTall);
		
		int iconSize = iconsTall;
		iconsWide = 0;

		if (playerInfo[i].tfclass == TFClass_Engineer) {
			SHOW_SLOT_ICON(primary);
			SHOW_SLOT_ICON(secondary);
			SHOW_SLOT_ICON(melee);
			SHOW_SLOT_ICON(pda);
			
			if (loadout_icons_nonloadout.GetBool()) {
				SHOW_SLOT_ICON(pda2);
				SHOW_SLOT_ICON(building);
			}
		}
		else if (playerInfo[i].tfclass == TFClass_Spy) {
			SHOW_SLOT_ICON(secondary);
			SHOW_SLOT_ICON(building);
			SHOW_SLOT_ICON(melee);
			
			if (loadout_icons_nonloadout.GetBool()) {
				SHOW_SLOT_ICON(pda);
			}
			
			SHOW_SLOT_ICON(pda2);
		}
		else {
			SHOW_SLOT_ICON(primary);
			SHOW_SLOT_ICON(secondary);
			SHOW_SLOT_ICON(melee);
		}
	}
}

void Hook_IPanel_SendMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	std::string originPanelName = g_pVGuiPanel->GetName(ifromPanel);
	std::string destinationPanelName = g_pVGuiPanel->GetName(vguiPanel);
	
	if (strcmp(params->GetName(), "DialogVariables") == 0) {
		const char *playerName = params->GetString("playername", NULL);
		
		if (playerName) {
			int iEntCount = Interfaces::pClientEntityList->GetHighestEntityIndex();
			IClientEntity *cEntity;
		
			for (int i = 0; i < iEntCount; i++) {
				cEntity = Interfaces::pClientEntityList->GetClientEntity(i);
			
				if (cEntity == NULL || !(Interfaces::GetGameResources()->IsConnected(i))) {
					continue;
				}
			
				if (strcmp(playerName, Interfaces::GetGameResources()->GetPlayerName(i)) == 0) {
					if (originPanelName.substr(0, 11).compare("playerpanel") == 0) {
						playerPanels[originPanelName] = i;
					}

					break;
				}
			}
		}
	}
}

bool Hook_IVEngineClient_GetPlayerInfo(int ent_num, player_info_t *pinfo) {
	bool result = SH_CALL(Interfaces::pEngineClient, &IVEngineClient::GetPlayerInfo)(ent_num, pinfo);

	if (player_aliases_enabled.GetBool()) {
		CSteamID playerSteamID = GetClientSteamID(ent_num);

		if (playerAliases.find(playerSteamID) != playerAliases.end()) {
			V_strcpy_safe(pinfo->name, playerAliases[playerSteamID].c_str());

			RETURN_META_VALUE(MRES_SUPERCEDE, result);
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, result);

}

// The plugin is a static singleton that is exported as an interface
StatusSpecPlugin g_StatusSpecPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(StatusSpecPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_StatusSpecPlugin);

StatusSpecPlugin::StatusSpecPlugin()
{
}

StatusSpecPlugin::~StatusSpecPlugin()
{
}

bool StatusSpecPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	if (!Interfaces::Load(interfaceFactory, gameServerFactory)) {
		Warning("[%s] Unable to load required libraries!\n", PLUGIN_DESC);
		return false;
	}

	if (!Offsets::PrepareOffsets()) {
		Warning("[%s] Unable to determine proper offsets!\n", PLUGIN_DESC);
		return false;
	}
	
	itemSchema = new ItemSchema();
	
	Paint::InitializeTexture(TEXTURE_NULL);
	Paint::InitializeTexture(TEXTURE_UBERCHARGE);
	Paint::InitializeTexture(TEXTURE_CRITBOOST);
	Paint::InitializeTexture(TEXTURE_MEGAHEALRED);
	Paint::InitializeTexture(TEXTURE_MEGAHEALBLU);
	Paint::InitializeTexture(TEXTURE_RESISTSHIELDRED);
	Paint::InitializeTexture(TEXTURE_RESISTSHIELDBLU);
	Paint::InitializeTexture(TEXTURE_BULLETRESISTRED);
	Paint::InitializeTexture(TEXTURE_BLASTRESISTRED);
	Paint::InitializeTexture(TEXTURE_FIRERESISTRED);
	Paint::InitializeTexture(TEXTURE_BULLETRESISTBLU);
	Paint::InitializeTexture(TEXTURE_BLASTRESISTBLU);
	Paint::InitializeTexture(TEXTURE_FIRERESISTBLU);
	Paint::InitializeTexture(TEXTURE_BUFFBANNERRED);
	Paint::InitializeTexture(TEXTURE_BUFFBANNERBLU);
	Paint::InitializeTexture(TEXTURE_BATTALIONSBACKUPRED);
	Paint::InitializeTexture(TEXTURE_BATTALIONSBACKUPBLU);
	Paint::InitializeTexture(TEXTURE_CONCHERORRED);
	Paint::InitializeTexture(TEXTURE_CONCHERORBLU);
	Paint::InitializeTexture(TEXTURE_JARATE);
	Paint::InitializeTexture(TEXTURE_MADMILK);
	Paint::InitializeTexture(TEXTURE_MARKFORDEATH);
	Paint::InitializeTexture(TEXTURE_BLEEDING);
	Paint::InitializeTexture(TEXTURE_FIRE);
	
	SH_ADD_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_ADD_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_ADD_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);
	
	ConVar_Register();
	
	Msg("%s loaded!\n", PLUGIN_DESC);
	return true;
}

void StatusSpecPlugin::Unload(void)
{
	SH_REMOVE_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_REMOVE_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_REMOVE_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);

	ConVar_Unregister();
	Interfaces::Unload();
}

void StatusSpecPlugin::Pause(void) {}
void StatusSpecPlugin::UnPause(void) {}
const char *StatusSpecPlugin::GetPluginDescription(void) { return PLUGIN_DESC; }
void StatusSpecPlugin::LevelInit(char const *pMapName) {}
void StatusSpecPlugin::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {}
void StatusSpecPlugin::GameFrame(bool simulating) {}
void StatusSpecPlugin::LevelShutdown(void) {}
void StatusSpecPlugin::ClientActive(edict_t *pEntity) {}
void StatusSpecPlugin::ClientDisconnect(edict_t *pEntity) {}
void StatusSpecPlugin::ClientPutInServer(edict_t *pEntity, char const *playername) {}
void StatusSpecPlugin::SetCommandClient(int index) {}
void StatusSpecPlugin::ClientSettingsChanged(edict_t *pEdict) {}
PLUGIN_RESULT StatusSpecPlugin::ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) { return PLUGIN_CONTINUE; }
PLUGIN_RESULT StatusSpecPlugin::ClientCommand(edict_t *pEntity, const CCommand &args) { return PLUGIN_CONTINUE; }
PLUGIN_RESULT StatusSpecPlugin::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) { return PLUGIN_CONTINUE; }
void StatusSpecPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {}
void StatusSpecPlugin::OnEdictAllocated(edict_t *edict) {}
void StatusSpecPlugin::OnEdictFreed(const edict_t *edict) {}