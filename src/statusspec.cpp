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

SH_DECL_HOOK1_void(IBaseClientDLL, FrameStageNotify, SH_NOATTRIB, 0, ClientFrameStage_t);
SH_DECL_HOOK1(IGameResources, GetPlayerName, SH_NOATTRIB, 0, const char *, int);
SH_DECL_HOOK3_void(IPanel, PaintTraverse, SH_NOATTRIB, 0, VPANEL, bool, bool);
SH_DECL_HOOK3_void(IPanel, SendMessage, SH_NOATTRIB, 0, VPANEL, KeyValues *, VPANEL);
SH_DECL_HOOK2(IVEngineClient, GetPlayerInfo, SH_NOATTRIB, 0, bool, int, player_info_t *);

std::map<int, std::string> itemIconTextures;

ConVar loadout_icons_enabled("statusspec_loadout_icons_enabled", "0", 0, "enable loadout icons");
ConVar loadout_icons_nonloadout("statusspec_loadout_icons_nonloadout", "0", 0, "enable loadout icons for nonloadout items");
ConVar status_icons_enabled("statusspec_status_icons_enabled", "0", 0, "enable status icons");
ConVar status_icons_max("statusspec_status_icons_max", "5", 0, "max number of status icons to be rendered");

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
			int tfclass = *MAKE_PTR(int*, cEntity, Entities::pCTFPlayer__m_iClass);
			int team = *MAKE_PTR(int*, cEntity, Entities::pCTFPlayer__m_iTeamNum);
			uint32_t playerCond = *MAKE_PTR(uint32_t*, cEntity, Entities::pCTFPlayer__m_nPlayerCond);
			uint32_t condBits = *MAKE_PTR(uint32_t*, cEntity, Entities::pCTFPlayer___condition_bits);
			uint32_t playerCondEx = *MAKE_PTR(uint32_t*, cEntity, Entities::pCTFPlayer__m_nPlayerCondEx);
			uint32_t playerCondEx2 = *MAKE_PTR(uint32_t*, cEntity, Entities::pCTFPlayer__m_nPlayerCondEx2);
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
		else if (Entities::CheckClassBaseclass(cEntity->GetClientClass(), "DT_EconEntity")) {
			int player = ENTITY_INDEX_FROM_ENTITY_OFFSET(cEntity, Entities::pCEconEntity__m_hOwnerEntity);
			IClientEntity *playerEntity = Interfaces::pClientEntityList->GetClientEntity(player);
			int itemDefinitionIndex = *MAKE_PTR(int*, cEntity, Entities::pCEconEntity__m_iItemDefinitionIndex);

			if (loadout_icons_enabled.GetBool()) {
				int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, Entities::pCTFPlayer__m_hActiveWeapon);

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
		}
	}
	
	if (loadout_icons_enabled.GetBool()) {
		for (std::map<int, Player>::iterator iterator = playerInfo.begin(); iterator != playerInfo.end(); iterator++) {
			int player = iterator->first;
			IClientEntity *playerEntity = Interfaces::pClientEntityList->GetClientEntity(player);
			
			if (!playerEntity) {
				continue;
			}
			
			int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, Entities::pCTFPlayer__m_hActiveWeapon);
			
			for (int i = 0; i < MAX_WEAPONS; i++) {
				int weapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(playerEntity, Entities::pCTFPlayer__m_hMyWeapons[i]);
				IClientEntity *weaponEntity = Interfaces::pClientEntityList->GetClientEntity(weapon);
			
				if (!weaponEntity || !Entities::CheckClassBaseclass(weaponEntity->GetClientClass(), "DT_EconEntity")) {
					continue;
				}
				
				int itemDefinitionIndex = *MAKE_PTR(int*, weaponEntity, Entities::pCEconEntity__m_iItemDefinitionIndex);
			
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

void Hook_IBaseClientDLL_FrameStageNotify(ClientFrameStage_t curStage) {
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

	if (curStage == FRAME_RENDER_START) {
		UpdateEntities();

		if (g_MedigunInfo) {
			g_MedigunInfo->Update();
		}
	}

	RETURN_META(MRES_IGNORED);
}

const char * Hook_IGameResources_GetPlayerName(int client) {
	if (g_PlayerAlias) {
		RETURN_META_VALUE(MRES_SUPERCEDE, g_PlayerAlias->GetPlayerNameOverride(client));
	}
	else {
		RETURN_META_VALUE(MRES_IGNORED, "");
	}
}
	
void Hook_IPanel_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) {
	if (g_AntiFreeze) {
		g_AntiFreeze->Paint(vguiPanel);
	}

	if (g_MedigunInfo) {
		g_MedigunInfo->Paint(vguiPanel);
	}

	const char* panelName = g_pVGuiPanel->GetName(vguiPanel);

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
	if (g_PlayerAlias) {
		RETURN_META_VALUE(MRES_SUPERCEDE, g_PlayerAlias->GetPlayerInfoOverride(ent_num, pinfo));
	}
	else {
		RETURN_META_VALUE(MRES_IGNORED, false);
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
	if (!Interfaces::Load(interfaceFactory, gameServerFactory)) {
		Warning("[%s] Unable to load required libraries!\n", PLUGIN_DESC);
		return false;
	}

	if (!Entities::PrepareOffsets()) {
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
	
	SH_ADD_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
	SH_ADD_HOOK(IPanel, PaintTraverse, g_pVGuiPanel, Hook_IPanel_PaintTraverse, true);
	SH_ADD_HOOK(IPanel, SendMessage, g_pVGuiPanel, Hook_IPanel_SendMessage, true);
	SH_ADD_HOOK(IVEngineClient, GetPlayerInfo, Interfaces::pEngineClient, Hook_IVEngineClient_GetPlayerInfo, false);
	
	ConVar_Register();
	
	Msg("%s loaded!\n", PLUGIN_DESC);
	return true;
}

void StatusSpecPlugin::Unload(void)
{
	SH_REMOVE_HOOK(IBaseClientDLL, FrameStageNotify, Interfaces::pClientDLL, Hook_IBaseClientDLL_FrameStageNotify, false);
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