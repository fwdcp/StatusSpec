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
ConVar loadout_icons_enabled("statusspec_loadout_icons_enabled", "0", 0, "enable loadout icons");
ConVar loadout_icons_nonloadout("statusspec_loadout_icons_nonloadout", "0", 0, "enable loadout icons for nonloadout items");
ConVar status_icons_enabled("statusspec_status_icons_enabled", "0", 0, "enable status icons");
ConVar status_icons_max("statusspec_status_icons_max", "5", 0, "max number of status icons to be rendered");

void (__fastcall *origSendMessage)(void* thisptr, int edx, vgui::VPANEL, KeyValues *, vgui::VPANEL);
void (__fastcall *origPaintTraverse)(void* thisptr, int edx, vgui::VPANEL, bool, bool);

#define SHOW_SLOT_ICON(slot) \
	if (playerInfo[i].slot != -1) { \
		g_pVGuiSurface->DrawSetTexture(m_iTextureItemIcon[playerInfo[i].slot]); \
		if (playerInfo[i].activeWeaponSlot.compare(#slot) == 0) { \
			g_pVGuiSurface->DrawSetColor(loadout_active_filter.r(), loadout_active_filter.g(), loadout_active_filter.b(), loadout_active_filter.a()); \
		} \
		else { \
			g_pVGuiSurface->DrawSetColor(loadout_nonactive_filter.r(), loadout_nonactive_filter.g(), loadout_nonactive_filter.b(), loadout_nonactive_filter.a()); \
		} \
		g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize); \
	} \
	 \
	iconsWide += iconSize;

inline bool IsInteger(const std::string &s)
{
   if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

   char *p;
   strtol(s.c_str(), &p, 10);

   return (*p == 0);
}

inline int ColorRangeRestrict(int color) {
	if (color < 0) return 0;
	else if (color > 255) return 255;
	else return color;
}

CON_COMMAND(statusspec_loadout_filter_nonactive, "the RGBA filter applied to the icon when the item is not active") {
	if (args.ArgC() < 4 || !IsInteger(args.Arg(1)) || !IsInteger(args.Arg(2)) || !IsInteger(args.Arg(3)) || !IsInteger(args.Arg(4)))
	{
		Msg("Usage: statusspec_loadout_filter_nonactive <red> <green> <blue> <alpha>\n");
		return;
	}
	
	int red = ColorRangeRestrict(std::stoi(args.Arg(1)));
	int green = ColorRangeRestrict(std::stoi(args.Arg(2)));
	int blue = ColorRangeRestrict(std::stoi(args.Arg(3)));
	int alpha = ColorRangeRestrict(std::stoi(args.Arg(4)));
	
	loadout_nonactive_filter.SetColor(red, green, blue, alpha);
	Msg("Set nonactive loadout item icon filter to rgba(%i, %i, %i, %i).", red, green, blue, alpha);
}

CON_COMMAND(statusspec_loadout_filter_active, "the RGBA filter applied to the icon when the item is active") {
	if (args.ArgC() < 4 || !IsInteger(args.Arg(1)) || !IsInteger(args.Arg(2)) || !IsInteger(args.Arg(3)) || !IsInteger(args.Arg(4)))
	{
		Msg("Usage: statusspec_loadout_filter_active <red> <green> <blue> <alpha>\n");
		return;
	}
	
	int red = ColorRangeRestrict(std::stoi(args.Arg(1)));
	int green = ColorRangeRestrict(std::stoi(args.Arg(2)));
	int blue = ColorRangeRestrict(std::stoi(args.Arg(3)));
	int alpha = ColorRangeRestrict(std::stoi(args.Arg(4)));
	
	loadout_active_filter.SetColor(red, green, blue, alpha);
	Msg("Set nonactive loadout icon filter to rgba(%i, %i, %i, %i).", red, green, blue, alpha);
}

int FindOrCreateTexture(const char *textureFile) {
	int textureId = g_pVGuiSurface->DrawGetTextureId(textureFile);
	
	if (textureId == -1) {
		textureId = g_pVGuiSurface->CreateNewTextureID();
		g_pVGuiSurface->DrawSetTextureFile(textureId, textureFile, 0, false);
	}

	return textureId;
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

void UpdateEntities() {
	int iEntCount = Interfaces::pClientEntityList->GetHighestEntityIndex();
	IClientEntity *cEntity;
	
	playerInfo.clear();

	for (int i = 0; i < iEntCount; i++) {
		cEntity = Interfaces::pClientEntityList->GetClientEntity(i);
		
		if (!cEntity) {
			continue;
		}
		
		if (Interfaces::GetGameResources()->IsConnected(i)) {
			int tfclass = *MAKE_PTR(int*, cEntity, WSOffsets::pCTFPlayer__m_iClass);
			int team = *MAKE_PTR(int*, cEntity, WSOffsets::pCTFPlayer__m_iTeamNum);
			uint32_t playerCond = *MAKE_PTR(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCond);
			uint32_t condBits = *MAKE_PTR(uint32_t*, cEntity, WSOffsets::pCTFPlayer___condition_bits);
			uint32_t playerCondEx = *MAKE_PTR(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCondEx);
			uint32_t playerCondEx2 = *MAKE_PTR(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCondEx2);
			
			if (team != TFTeam_Red && team != TFTeam_Blue) {
				continue;
			}
			
			playerInfo[i].tfclass = tfclass;
			playerInfo[i].team = team;
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
		else if (loadout_icons_enabled.GetBool() && WSOffsets::CheckClassBaseclass(cEntity->GetClientClass(), "DT_EconEntity")) {
			int player = ENTITY_INDEX_FROM_ENTITY_OFFSET(cEntity, WSOffsets::pCEconEntity__m_hOwnerEntity);
			IClientEntity *playerEntity = Interfaces::pClientEntityList->GetClientEntity(player);
			int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, WSOffsets::pCTFPlayer__m_hActiveWeapon);
			
			int itemDefinitionIndex = *MAKE_PTR(int*, cEntity, WSOffsets::pCEconEntity__m_iItemDefinitionIndex);
			
			const char *itemType = itemSchema->GetItemKeyValue(itemDefinitionIndex, "item_slot");

			if (activeWeapon == i) {
				playerInfo[player].activeWeaponSlot = itemType;
			}
			
			if (strcmp(itemType, "primary") == 0) {
				playerInfo[player].primary = itemDefinitionIndex;
			}
			else if (strcmp(itemType, "secondary") == 0) {
				playerInfo[player].secondary = itemDefinitionIndex;
			}
			else if (strcmp(itemType, "melee") == 0) {
				playerInfo[player].melee = itemDefinitionIndex;
			}
			else if (strcmp(itemType, "pda") == 0) {
				playerInfo[player].pda = itemDefinitionIndex;
			}
			else if (strcmp(itemType, "pda2") == 0) {
				playerInfo[player].pda2 = itemDefinitionIndex;
			}
			else if (strcmp(itemType, "building") == 0) {
				playerInfo[player].building = itemDefinitionIndex;
			}
			else if (strcmp(itemType, "head") == 0 || strcmp(itemType, "misc") == 0) {
				for (int slot = 0; slot < 3; slot++) {
					if (playerInfo[player].cosmetic[slot] == -1) {
						playerInfo[player].cosmetic[slot] = itemDefinitionIndex;
						break;
					}
				}
			}
			else if (strcmp(itemType, "action") == 0) {
				playerInfo[player].action = itemDefinitionIndex;
			}
			
			m_iTextureItemIcon[itemDefinitionIndex] = FindOrCreateTexture(itemSchema->GetItemKeyValue(itemDefinitionIndex, "image_inventory"));
		}
	}
}

void __fastcall hookedSendMessage(vgui::IPanel *thisPtr, int edx, vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	origSendMessage(thisPtr, edx, vguiPanel, params, ifromPanel);
	
	const char *ifromPanelName = g_pVGuiPanel->GetName(ifromPanel);
	
	if (playerPanelName.compare(0, 11, ifromPanelName, 0, 11) == 0 && strcmp(params->GetName(), "DialogVariables") == 0) {
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
	const char* panelName = g_pVGuiPanel->GetName(vguiPanel);

	if (strcmp(panelName, "MatSystemTopPanel") == 0 && (status_icons_enabled.GetBool() || loadout_icons_enabled.GetBool())) {
		UpdateEntities();
	}
	
	origPaintTraverse(thisPtr, edx, vguiPanel, forceRepaint, allowForce);

	if (Interfaces::pEngineClient->IsDrawingLoadingImage() || !Interfaces::pEngineClient->IsInGame() || !Interfaces::pEngineClient->IsConnected() || Interfaces::pEngineClient->Con_IsVisible())
		return;
	
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallBulletResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBulletResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
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
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallBlastResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBlastResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
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
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureResistShieldBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			
			playerWide += iconSize;
			iconsWide += iconSize;
			
			icons++;
		}
		else if (CheckCondition(playerInfo[i].conditions, TFCond_SmallFireResist)) {
			g_pVGuiPanel->SetSize(vguiPanel, iconsWide + iconsTall, iconsTall);
			g_pVGuiPanel->SetSize(playerPanel, playerWide + iconsTall, playerTall);
			
			if (playerInfo[i].team == TFTeam_Red) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistRed);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureFireResistBlu);
				g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
				g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
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
				g_pVGuiSurface->DrawSetTexture(m_iTextureBuffBannerRed);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBuffBannerBlu);
			}
			
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
				g_pVGuiSurface->DrawSetTexture(m_iTextureBattalionsBackupRed);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureBattalionsBackupBlu);
			}
			
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
				g_pVGuiSurface->DrawSetTexture(m_iTextureConcherorRed);
			}
			else if (playerInfo[i].team == TFTeam_Blue) {
				g_pVGuiSurface->DrawSetTexture(m_iTextureConcherorBlu);
			}
			
			g_pVGuiSurface->DrawSetColor(255, 255, 255, 255);
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
			g_pVGuiSurface->DrawTexturedRect(iconsWide, 0, iconsWide + iconSize, iconSize);
			
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
	else if (strcmp(panelName, "MatSystemTopPanel") == 0) {
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
	
	m_iTextureUbercharged = FindOrCreateTexture(TEXTURE_UBERCHARGEICON);
	m_iTextureCritBoosted = FindOrCreateTexture(TEXTURE_CRITBOOSTICON);
	m_iTextureResistShieldRed = FindOrCreateTexture(TEXTURE_RESISTSHIELDRED);
	m_iTextureResistShieldBlu = FindOrCreateTexture(TEXTURE_RESISTSHIELDBLU);
	m_iTextureBulletResistRed = FindOrCreateTexture(TEXTURE_BULLETRESISTRED);
	m_iTextureBlastResistRed = FindOrCreateTexture(TEXTURE_BLASTRESISTRED);
	m_iTextureFireResistRed = FindOrCreateTexture(TEXTURE_FIRERESISTRED);
	m_iTextureBulletResistBlu = FindOrCreateTexture(TEXTURE_BULLETRESISTBLU);
	m_iTextureBlastResistBlu = FindOrCreateTexture(TEXTURE_BLASTRESISTBLU);
	m_iTextureFireResistBlu = FindOrCreateTexture(TEXTURE_FIRERESISTBLU);
	m_iTextureBuffBannerRed = FindOrCreateTexture(TEXTURE_BUFFBANNERRED);
	m_iTextureBuffBannerBlu = FindOrCreateTexture(TEXTURE_BUFFBANNERBLU);
	m_iTextureBattalionsBackupRed = FindOrCreateTexture(TEXTURE_BATTALIONSBACKUPRED);
	m_iTextureBattalionsBackupBlu = FindOrCreateTexture(TEXTURE_BATTALIONSBACKUPBLU);
	m_iTextureConcherorRed = FindOrCreateTexture(TEXTURE_CONCHERORRED);
	m_iTextureConcherorBlu = FindOrCreateTexture(TEXTURE_CONCHERORBLU);
	m_iTextureJarated = FindOrCreateTexture(TEXTURE_JARATE);
	m_iTextureMilked = FindOrCreateTexture(TEXTURE_MADMILK);
	m_iTextureMarkedForDeath = FindOrCreateTexture(TEXTURE_MARKEDFORDEATH);
	m_iTextureBleeding = FindOrCreateTexture(TEXTURE_BLEEDING);
	m_iTextureOnFire = FindOrCreateTexture(TEXTURE_ONFIRE);
	
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
