/*
 *  loadouticons.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "loadouticons.h"

#define SHOW_SLOT_ICON(slot) DrawSlotIcon(player, loadoutInfo[player].slot, iconsWide, iconSize);

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

LoadoutIcons::LoadoutIcons() {
	filter_active_color = Color(255, 255, 255, 255);
	filter_nonactive_color = Color(127, 127, 127, 255);
	itemSchema = new ItemSchema();

	enabled = new ConVar("statusspec_loadouticons_enabled", "0", FCVAR_NONE, "enable loadout icons");
	filter_active = new ConCommand("statusspec_loadouticons_filter_active", LoadoutIcons::SetFilter, "the RGBA filter applied to the icon for an active item", FCVAR_NONE, LoadoutIcons::GetCurrentFilter);
	filter_nonactive = new ConCommand("statusspec_loadouticons_filter_nonactive", LoadoutIcons::SetFilter, "the RGBA filter applied to the icon for a nonactive item", FCVAR_NONE, LoadoutIcons::GetCurrentFilter);
	nonloadout = new ConVar("statusspec_loadouticons_nonloadout", "0", FCVAR_NONE, "enable loadout icons for nonloadout items");
	only_active = new ConVar("statusspec_loadouticons_only_active", "0", FCVAR_NONE, "only display loadout icons for the active weapon");
}

LoadoutIcons::~LoadoutIcons() {
	delete itemSchema;
}

bool LoadoutIcons::IsEnabled() {
	return enabled->GetBool();
}

void LoadoutIcons::InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel) {
	std::string originPanelName = g_pVGuiPanel->GetName(ifromPanel);

	if (originPanelName.substr(0, 11).compare("playerpanel") == 0 && strcmp(params->GetName(), "DialogVariables") == 0) {
		const char *playerName = params->GetString("playername", NULL);
		
		if (playerName) {
			for (int i = 0; i <= MAX_PLAYERS; i++) {
				Player player = i;
			
				if (!player) {
					continue;
				}
			
				if (strcmp(playerName, player.GetName()) == 0) {
					playerPanels[originPanelName] = player;

					break;
				}
			}
		}
	}
}

void LoadoutIcons::Paint(vgui::VPANEL vguiPanel) {
	const char *panelName = g_pVGuiPanel->GetName(vguiPanel);
	
	if (strcmp(panelName, "loadouticons") == 0) {
		vgui::VPANEL playerPanel = g_pVGuiPanel->GetParent(vguiPanel);
		const char *playerPanelName = g_pVGuiPanel->GetName(playerPanel);
		
		if (playerPanels.find(playerPanelName) == playerPanels.end()) {
			return;
		}
		
		Player player = playerPanels[playerPanelName];
		
		int iconsWide, iconsTall;
		
		g_pVGuiPanel->GetSize(vguiPanel, iconsWide, iconsTall);
		
		int iconSize = iconsTall;
		iconsWide = 0;

		if (loadoutInfo[player].tfclass == TFClass_Engineer) {
			SHOW_SLOT_ICON(primary);
			SHOW_SLOT_ICON(secondary);
			SHOW_SLOT_ICON(melee);
			SHOW_SLOT_ICON(pda);
			
			if (nonloadout->GetBool()) {
				SHOW_SLOT_ICON(pda2);
				SHOW_SLOT_ICON(building);
			}
		}
		else if (loadoutInfo[player].tfclass == TFClass_Spy) {
			SHOW_SLOT_ICON(secondary);
			SHOW_SLOT_ICON(building);
			SHOW_SLOT_ICON(melee);
			
			if (nonloadout->GetBool()) {
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

void LoadoutIcons::PreEntityUpdate() {
	loadoutInfo.clear();
}

void LoadoutIcons::ProcessEntity(IClientEntity* entity) {
	if (!Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_EconEntity")) {
		return;
	}

	int itemDefinitionIndex = *MAKE_PTR(int*, entity, Entities::pCEconEntity__m_iItemDefinitionIndex);

	Player player = ENTITY_INDEX_FROM_ENTITY_OFFSET(entity, Entities::pCEconEntity__m_hOwnerEntity);
	TFClassType tfclass = player.GetClass();
	int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(player.GetEntity(), Entities::pCTFPlayer__m_hActiveWeapon);

	const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");
			
	KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
	if (classUses) {
		const char *classUse = classUses->GetString(tfclassNames[tfclass].c_str(), "");

		if (std::find(std::begin(itemSlots), std::end(itemSlots), classUse) != std::end(itemSlots)) {
			itemSlot = classUse;
		}
	}

	if (activeWeapon == entity->entindex()) {
		loadoutInfo[player].activeWeaponSlot = itemDefinitionIndex;
	}
			
	if (strcmp(itemSlot, "primary") == 0) {
		loadoutInfo[player].primary = itemDefinitionIndex;
	}
	else if (strcmp(itemSlot, "secondary") == 0) {
		loadoutInfo[player].secondary = itemDefinitionIndex;
	}
	else if (strcmp(itemSlot, "melee") == 0) {
		loadoutInfo[player].melee = itemDefinitionIndex;
	}
	else if (strcmp(itemSlot, "pda") == 0) {
		loadoutInfo[player].pda = itemDefinitionIndex;
	}
	else if (strcmp(itemSlot, "pda2") == 0) {
		loadoutInfo[player].pda2 = itemDefinitionIndex;
	}
	else if (strcmp(itemSlot, "building") == 0) {
		loadoutInfo[player].building = itemDefinitionIndex;
	}
	else if (strcmp(itemSlot, "head") == 0 || strcmp(itemSlot, "misc") == 0) {
		for (int slot = 0; slot < MAX_COSMETIC_SLOTS; slot++) {
			if (loadoutInfo[player].cosmetic[slot] == -1) {
				loadoutInfo[player].cosmetic[slot] = itemDefinitionIndex;
				break;
			}
		}
	}
	else if (strcmp(itemSlot, "taunt") == 0) {
		for (int slot = 0; slot < MAX_TAUNT_SLOTS; slot++) {
			if (loadoutInfo[player].taunt[slot] == -1) {
				loadoutInfo[player].taunt[slot] = itemDefinitionIndex;
				break;
			}
		}
	}
	else if (strcmp(itemSlot, "action") == 0) {
		loadoutInfo[player].action = itemDefinitionIndex;
	}
				
	const char *itemIcon = itemSchema->GetItemKeyData(itemDefinitionIndex, "image_inventory");
	Paint::InitializeTexture(itemIcon);
	itemIconTextures[itemDefinitionIndex] = itemIcon;
}

void LoadoutIcons::PostEntityUpdate() {
	for (auto iterator = loadoutInfo.begin(); iterator != loadoutInfo.end(); iterator++) {
		Player player = iterator->first;
			
		if (!player) {
			continue;
		}
		
		TFClassType tfclass = player.GetClass();
		int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(player.GetEntity(), Entities::pCTFPlayer__m_hActiveWeapon);
		
		loadoutInfo[player].tfclass = tfclass;

		for (int i = 0; i < MAX_WEAPONS; i++) {
			int weapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(player.GetEntity(), Entities::pCTFPlayer__m_hMyWeapons[i]);
			IClientEntity *weaponEntity = Interfaces::pClientEntityList->GetClientEntity(weapon);
			
			if (!weaponEntity || !Entities::CheckClassBaseclass(weaponEntity->GetClientClass(), "DT_EconEntity")) {
				continue;
			}
				
			int itemDefinitionIndex = *MAKE_PTR(int*, weaponEntity, Entities::pCEconEntity__m_iItemDefinitionIndex);
			
			const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");
				
			KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
			if (classUses) {
				const char *classUse = classUses->GetString(tfclassNames[tfclass].c_str(), "");

				if (std::find(std::begin(itemSlots), std::end(itemSlots), classUse) != std::end(itemSlots)) {
					itemSlot = classUse;
				}
			}

			if (activeWeapon == i) {
				loadoutInfo[player].activeWeaponSlot = itemDefinitionIndex;
			}
				
			if (strcmp(itemSlot, "primary") == 0) {
				loadoutInfo[player].primary = itemDefinitionIndex;
			}
			else if (strcmp(itemSlot, "secondary") == 0) {
				loadoutInfo[player].secondary = itemDefinitionIndex;
			}
			else if (strcmp(itemSlot, "melee") == 0) {
				loadoutInfo[player].melee = itemDefinitionIndex;
			}
			else if (strcmp(itemSlot, "pda") == 0) {
				loadoutInfo[player].pda = itemDefinitionIndex;
			}
			else if (strcmp(itemSlot, "pda2") == 0) {
				loadoutInfo[player].pda2 = itemDefinitionIndex;
			}
			else if (strcmp(itemSlot, "building") == 0) {
				loadoutInfo[player].building = itemDefinitionIndex;
			}
			else if (strcmp(itemSlot, "head") == 0 || strcmp(itemSlot, "misc") == 0) {
				for (int slot = 0; slot < MAX_COSMETIC_SLOTS; slot++) {
					if (loadoutInfo[player].cosmetic[slot] == -1) {
						loadoutInfo[player].cosmetic[slot] = itemDefinitionIndex;
						break;
					}
				}
			}
			else if (strcmp(itemSlot, "taunt") == 0) {
				for (int slot = 0; slot < MAX_TAUNT_SLOTS; slot++) {
					if (loadoutInfo[player].taunt[slot] == -1) {
						loadoutInfo[player].taunt[slot] = itemDefinitionIndex;
						break;
					}
				}
			}
			else if (strcmp(itemSlot, "action") == 0) {
				loadoutInfo[player].action = itemDefinitionIndex;
			}
		}
	}
}

void LoadoutIcons::DrawSlotIcon(Player player, int weapon, int &width, int size) {
	if (only_active->GetBool()) {
		if (weapon != -1 && loadoutInfo[player].activeWeaponSlot == weapon) {
			Paint::DrawTexture(itemIconTextures[weapon], width, 0, size, size, filter_active_color);
			width += size;
		}
	}
	else {
		if (weapon != -1) {
			if (loadoutInfo[player].activeWeaponSlot == weapon) {
				Paint::DrawTexture(itemIconTextures[weapon], width, 0, size, size, filter_active_color);
			}
			else {
				Paint::DrawTexture(itemIconTextures[weapon], width, 0, size, size, filter_nonactive_color);
			}
		}

		width += size;
	}
}

int LoadoutIcons::GetCurrentFilter(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	if (stricmp(command.c_str(), "statusspec_loadouticons_filter_active") == 0) {
		V_snprintf(commands[0], COMMAND_COMPLETION_ITEM_LENGTH, "%s %i %i %i %i", command.c_str(), g_LoadoutIcons->filter_active_color.r(), g_LoadoutIcons->filter_active_color.g(), g_LoadoutIcons->filter_active_color.b(), g_LoadoutIcons->filter_active_color.a());
		
		return 1;
	}
	else if (stricmp(command.c_str(), "statusspec_loadouticons_filter_nonactive") == 0) {
		V_snprintf(commands[0], COMMAND_COMPLETION_ITEM_LENGTH, "%s %i %i %i %i", command.c_str(), g_LoadoutIcons->filter_nonactive_color.r(), g_LoadoutIcons->filter_nonactive_color.g(), g_LoadoutIcons->filter_nonactive_color.b(), g_LoadoutIcons->filter_nonactive_color.a());

		return 1;
	}
	else {
		return 0;
	}
}

void LoadoutIcons::SetFilter(const CCommand &command) {
	if (command.ArgC() == 0) {
		if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_active")) {
			Msg("The current active loadout item icon filter is rgba(%i, %i, %i, %i).\n", g_LoadoutIcons->filter_active_color.r(), g_LoadoutIcons->filter_active_color.g(), g_LoadoutIcons->filter_active_color.b(), g_LoadoutIcons->filter_active_color.a());
			return;
		}
		else if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_nonactive")) {
			Msg("The current nonactive loadout item icon filter is rgba(%i, %i, %i, %i).\n", g_LoadoutIcons->filter_nonactive_color.r(), g_LoadoutIcons->filter_nonactive_color.g(), g_LoadoutIcons->filter_nonactive_color.b(), g_LoadoutIcons->filter_nonactive_color.a());
			return;
		}
	}
	else if (command.ArgC() < 4 || !IsInteger(command.Arg(1)) || !IsInteger(command.Arg(2)) || !IsInteger(command.Arg(3)) || !IsInteger(command.Arg(4)))
	{
		Warning("Usage: %s <red> <green> <blue> <alpha>\n", command.Arg(0));
		return;
	}

	int red = ColorRangeRestrict(std::stoi(command.Arg(1)));
	int green = ColorRangeRestrict(std::stoi(command.Arg(2)));
	int blue = ColorRangeRestrict(std::stoi(command.Arg(3)));
	int alpha = ColorRangeRestrict(std::stoi(command.Arg(4)));

	if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_active")) {
		g_LoadoutIcons->filter_active_color.SetColor(red, green, blue, alpha);
		Msg("Set active loadout item icon filter to rgba(%i, %i, %i, %i).\n", red, green, blue, alpha);
	}
	else if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_nonactive")) {
		g_LoadoutIcons->filter_nonactive_color.SetColor(red, green, blue, alpha);
		Msg("Set nonactive loadout item icon filter to rgba(%i, %i, %i, %i).\n", red, green, blue, alpha);
	}
	else {
		Warning("Unrecognized command!\n");
	}
}