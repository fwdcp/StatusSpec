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

ConVar force_refresh_specgui("statusspec_force_specgui_refresh", "0", 0, "whether to force the spectator GUI to refresh");
ConVar status_icons_enabled("statusspec_status_icons_enabled", "0", 0, "enable status icons");
ConVar status_icons_max("statusspec_status_icons_max", "5", 0, "max number of status icons to be rendered");

void (__fastcall *origSendMessage)(void* thisptr, int edx, vgui::VPANEL, KeyValues *, vgui::VPANEL);
void (__fastcall *origPaintTraverse)(void* thisptr, int edx, vgui::VPANEL, bool, bool);

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

void UpdateEntities() {
	int iEntCount = Interfaces::pClientEntityList->GetHighestEntityIndex();
	IClientEntity *cEntity;
	
	playerInfo.clear();

	for (int i = 0; i < iEntCount; i++) {
		cEntity = Interfaces::pClientEntityList->GetClientEntity(i);
		
		// Ensure valid player entity
		if (cEntity == NULL || !(Interfaces::GetGameResources()->IsConnected(i))) {
			continue;
		}
		
		// get our stuff directly from entity data
		int team = *MakePtr(int*, cEntity, WSOffsets::pCTFPlayer__m_iTeamNum);
		uint32_t playerCond = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCond);
		uint32_t condBits = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer___condition_bits);
		uint32_t playerCondEx = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCondEx);
		uint32_t playerCondEx2 = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCondEx2);
		
		if (team != TEAM_RED && team != TEAM_BLU) {
			continue;
		}
		
		playerInfo[i].team = team;
		playerInfo[i].conditions[0] = playerCond|condBits;
		playerInfo[i].conditions[1] = playerCondEx;
		playerInfo[i].conditions[2] = playerCondEx2;
	}
}

void __fastcall hookedSendMessage(vgui::IPanel *thisPtr, int edx, vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	origSendMessage(thisPtr, edx, vguiPanel, params, ifromPanel);
	
	const char *ifromPanelName = g_pVGuiPanel->GetName(ifromPanel);
	
	if (playerPanelName.compare(0, 11, ifromPanelName, 0, 11) && strcmp(params->GetName(), "DialogVariables") == 0) {
		const char *playerName = params->GetString("playername");
		
		int iEntCount = Interfaces::pClientEntityList->GetHighestEntityIndex();
		IClientEntity *cEntity;
		
		for (int i = 0; i < iEntCount; i++) {
			cEntity = Interfaces::pClientEntityList->GetClientEntity(i);
			
			if (cEntity == NULL || !(Interfaces::GetGameResources()->IsConnected(i))) {
				continue;
			}
			
			if (strcmp(playerName, Interfaces::GetGameResources()->GetPlayerName(i)) == 0) {
				playerPanels[g_pVGuiPanel->GetName(ifromPanel)] = i;
				break;
			}
		}
	}
}
	
void __fastcall hookedPaintTraverse(vgui::IPanel *thisPtr, int edx, vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (status_icons_enabled.GetBool()) {
		UpdateEntities();
	}
	
	origPaintTraverse(thisPtr, edx, vguiPanel, forceRepaint, allowForce);

	if (Interfaces::pEngineClient->IsDrawingLoadingImage() || !Interfaces::pEngineClient->IsInGame() || !Interfaces::pEngineClient->IsConnected() || Interfaces::pEngineClient->Con_IsVisible())
		return;
	
	const char* panelName = g_pVGuiPanel->GetName(vguiPanel);
	if (strcmp(panelName, "specgui") == 0) {
		specguiPanel = g_pVGui->PanelToHandle(vguiPanel);
	}
	else if (strcmp(panelName, "statusicons") == 0) {
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureUbercharged);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureCritBoosted);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallBulletResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
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
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallBlastResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
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
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallFireResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
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
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBuffBannerRed);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBuffBannerBlu);
			}
			
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBattalionsBackupRed);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBattalionsBackupBlu);
			}
			
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			if (playerInfo[i].team == TEAM_RED) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureConcherorRed);
			}
			else if (playerInfo[i].team == TEAM_BLU) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureConcherorBlu);
			}
			
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureJarated);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureMilked);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureMarkedForDeath);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureBleeding);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
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
			
			g_pVGuiSurface->DrawSetTexture(m_iTextureOnFire);
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconSize, iconSize);
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		
		if (icons >= maxIcons) {
			return;
		}
	}
	else if (panelName[0] == 'M' && panelName[3] == 'S' && panelName[9] == 'T' && panelName[12] == 'P') {
		if (force_refresh_specgui.GetBool() && specguiPanel) {
			g_pVGuiPanel->SendMessage(g_pVGui->HandleToPanel(specguiPanel), performlayoutCommand, g_pVGui->HandleToPanel(specguiPanel));
		}
	}
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
	if (!Interfaces::Load(interfaceFactory, gameServerFactory))
	{
		Warning("Unable to load required libraries for %s!", PLUGIN_DESC);
		return false;
	}
	
	itemSchema = new ItemSchema();
	
	m_iTextureUbercharged = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureCritBoosted = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureResistShieldRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureResistShieldBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBulletResistRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBlastResistRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureFireResistRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBulletResistBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBlastResistBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureFireResistBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBuffBannerRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBuffBannerBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBattalionsBackupRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBattalionsBackupBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureConcherorRed = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureConcherorBlu = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureJarated = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureMilked = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureMarkedForDeath = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureBleeding = g_pVGuiSurface->CreateNewTextureID();
	m_iTextureOnFire = g_pVGuiSurface->CreateNewTextureID();
	
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureUbercharged, TEXTURE_UBERCHARGEICON, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureCritBoosted, TEXTURE_CRITBOOSTICON, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureResistShieldRed, TEXTURE_RESISTSHIELDRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureResistShieldBlu, TEXTURE_RESISTSHIELDBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBulletResistRed, TEXTURE_BULLETRESISTRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBlastResistRed, TEXTURE_BLASTRESISTRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureFireResistRed, TEXTURE_FIRERESISTRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBulletResistBlu, TEXTURE_BULLETRESISTBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBlastResistBlu, TEXTURE_BLASTRESISTBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureFireResistBlu, TEXTURE_FIRERESISTBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBuffBannerRed, TEXTURE_BUFFBANNERRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBuffBannerBlu, TEXTURE_BUFFBANNERBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBattalionsBackupRed, TEXTURE_BATTALIONSBACKUPRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBattalionsBackupBlu, TEXTURE_BATTALIONSBACKUPBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureConcherorRed, TEXTURE_CONCHERORRED, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureConcherorBlu, TEXTURE_CONCHERORBLU, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureJarated, TEXTURE_JARATE, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureMilked, TEXTURE_MADMILK, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureMarkedForDeath, TEXTURE_MARKEDFORDEATH, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureBleeding, TEXTURE_BLEEDING, 0, false);
	g_pVGuiSurface->DrawSetTextureFile(m_iTextureOnFire, TEXTURE_ONFIRE, 0, false);
	
	performlayoutCommand = new KeyValues("Command", "Command", "performlayout");
	
	// hook SendMessage
	origSendMessage = (void (__fastcall *)(void *, int, vgui::VPANEL, KeyValues *, vgui::VPANEL))
	HookVFunc(*(DWORD**)g_pVGuiPanel, Index_SendMessage, (DWORD*) &hookedSendMessage);
	
	// hook PaintTraverse
	origPaintTraverse = (void (__fastcall *)(void *, int, vgui::VPANEL, bool, bool))
	HookVFunc(*(DWORD**)g_pVGuiPanel, Index_PaintTraverse, (DWORD*) &hookedPaintTraverse);
	
	// get offsets
	WSOffsets::PrepareOffsets();
	
	// register CVars
	ConVar_Register(0);
	
	// ready to go
	Msg("%s loaded!\n", PLUGIN_DESC);
	return true;
}

void StatusSpecPlugin::Unload(void)
{
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