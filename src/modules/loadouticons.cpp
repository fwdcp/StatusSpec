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

LoadoutIcons::LoadoutIcons() {
	filter_active_color = Color(255, 255, 255, 255);
	filter_inactive_color = Color(127, 127, 127, 255);
	frameHook = 0;
	itemSchema = new ItemSchema();

	enabled = new ConVar("statusspec_loadouticons_enabled", "0", FCVAR_NONE, "enable loadout icons", [](IConVar *var, const char *pOldValue, float flOldValue) { g_LoadoutIcons->ToggleEnabled(var, pOldValue, flOldValue); });
	filter_active = new ConCommand("statusspec_loadouticons_filter_active", [](const CCommand &command) { g_LoadoutIcons->SetFilter(command); }, "the RGBA filter applied to the icon for an active item", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_LoadoutIcons->GetCurrentFilter(partial, commands); });
	filter_inactive = new ConCommand("statusspec_loadouticons_filter_inactive", [](const CCommand &command) { g_LoadoutIcons->SetFilter(command); }, "the RGBA filter applied to the icon for an inactive item", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_LoadoutIcons->GetCurrentFilter(partial, commands); });
	nonloadout = new ConVar("statusspec_loadouticons_nonloadout", "0", FCVAR_NONE, "enable loadout icons for nonloadout items");
	only_active = new ConVar("statusspec_loadouticons_only_active", "0", FCVAR_NONE, "only display loadout icons for the active weapon");
}

void LoadoutIcons::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		loadoutInfo.clear();

		int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

		for (int i = 0; i < maxEntity; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity || !Entities::CheckClassBaseclass(entity->GetClientClass(), "DT_EconEntity")) {
				continue;
			}

			Player player = ENTITY_INDEX_FROM_ENTITY_OFFSET(entity, Entities::pCEconEntity__m_hOwnerEntity);
			
			if (!player) {
				continue;
			}

			int itemDefinitionIndex = *MAKE_PTR(int*, entity, Entities::pCEconEntity__m_iItemDefinitionIndex);

			if (itemIconTextures.find(itemDefinitionIndex) == itemIconTextures.end()) {
				std::string itemIcon = "../";
				itemIcon += itemSchema->GetItemKeyData(itemDefinitionIndex, "image_inventory");
				itemIconTextures[itemDefinitionIndex] = itemIcon;
			}

			const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");
			
			TFClassType tfclass = player.GetClass();
			KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
			if (classUses) {
				const char *classUse = classUses->GetString(tfclassNames[tfclass].c_str(), "");

				if (std::find(std::begin(itemSlots), std::end(itemSlots), classUse) != std::end(itemSlots)) {
					itemSlot = classUse;
				}
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

			int activeWeapon = ENTITY_INDEX_FROM_ENTITY_OFFSET(player.GetEntity(), Entities::pCTFPlayer__m_hActiveWeapon);
			if (activeWeapon == entity->entindex()) {
				loadoutInfo[player].activeWeaponSlot = itemDefinitionIndex;
			}
		}

		for (int i = 1; i <= MAX_PLAYERS; i++) {
			Player player = i;

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

				if (activeWeapon == weapon) {
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

		if (Interfaces::GetClientMode() && Interfaces::GetClientMode()->GetViewport()) {
			vgui::VPANEL viewport = Interfaces::GetClientMode()->GetViewport()->GetVPanel();

			for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport); i++) {
				vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport, i);

				if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
					for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
						vgui::VPANEL playerPanel = g_pVGuiPanel->GetChild(specgui, i);

						if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
							for (int i = 0; i < g_pVGuiPanel->GetChildCount(playerPanel); i++) {
								vgui::VPANEL loadoutIconsVPanel = g_pVGuiPanel->GetChild(playerPanel, i);

								if (strcmp(g_pVGuiPanel->GetName(loadoutIconsVPanel), "LoadoutIcons") == 0) {
									vgui::EditablePanel *loadoutIcons = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(loadoutIconsVPanel, "ClientDLL"));

									if (loadoutIcons) {
										loadoutIcons->SetEnabled(true);
										loadoutIcons->SetVisible(true);

										if (loadoutIcons->GetChildCount() == 0) {
											InitIcons(loadoutIcons);
										}

										DisplayIcons(playerPanel);
									}

									break;
								}
							}
						}
					}

					break;
				}
			}
		}
	}
}

void LoadoutIcons::DisableHUD() {
	if (Interfaces::GetClientMode() && Interfaces::GetClientMode()->GetViewport()) {
		vgui::VPANEL viewport = Interfaces::GetClientMode()->GetViewport()->GetVPanel();

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(viewport); i++) {
			vgui::VPANEL specgui = g_pVGuiPanel->GetChild(viewport, i);

			if (strcmp(g_pVGuiPanel->GetName(specgui), "specgui") == 0) {
				for (int i = 0; i < g_pVGuiPanel->GetChildCount(specgui); i++) {
					vgui::VPANEL playerPanel = g_pVGuiPanel->GetChild(specgui, i);

					if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
						for (int i = 0; i < g_pVGuiPanel->GetChildCount(playerPanel); i++) {
							vgui::VPANEL loadoutIconsVPanel = g_pVGuiPanel->GetChild(playerPanel, i);

							if (strcmp(g_pVGuiPanel->GetName(loadoutIconsVPanel), "LoadoutIcons") == 0) {
								g_pVGuiPanel->SetEnabled(loadoutIconsVPanel, false);
								g_pVGuiPanel->SetVisible(loadoutIconsVPanel, false);

								break;
							}
						}
					}
				}

				break;
			}
		}
	}
}

void LoadoutIcons::DisplayIcon(vgui::ImagePanel *panel, int itemDefinitionIndex, bool active) {
	if (panel) {
		if (active) {
			panel->SetDrawColor(filter_active_color);
		}
		else {
			panel->SetDrawColor(filter_inactive_color);
		}

		if (itemDefinitionIndex != -1) {
			panel->SetVisible(true);
			panel->SetImage(itemIconTextures[itemDefinitionIndex].c_str());
		}
		else {
			panel->SetVisible(false);
			panel->SetImage("dev/null");
		}
	}
}

void LoadoutIcons::DisplayIcons(vgui::VPANEL playerPanel) {
	if (strcmp(g_pVGuiPanel->GetClassName(playerPanel), "CTFPlayerPanel") == 0) {
		vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel *>(g_pVGuiPanel->GetPanel(playerPanel, "ClientDLL"));

		if (panel) {
			vgui::HPanel panelHandle = g_pVGui->PanelToHandle(playerPanel);

			KeyValues *dialogVariables = panel->GetDialogVariables();

			if (dialogVariables) {
				const char *name = dialogVariables->GetString("playername");

				for (int i = 1; i <= MAX_PLAYERS; i++) {
					Player player = i;

					if (player && strcmp(player.GetName(), name) == 0) {
						if (only_active->GetBool()) {
							DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem1"]), loadoutInfo[player].activeWeaponSlot, true);
						}
						else {
							if (loadoutInfo[player].tfclass == TFClass_Engineer) {
								if (nonloadout->GetBool()) {
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem1"]), loadoutInfo[player].primary, loadoutInfo[player].primary == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem2"]), loadoutInfo[player].secondary, loadoutInfo[player].secondary == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem3"]), loadoutInfo[player].melee, loadoutInfo[player].melee == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem4"]), loadoutInfo[player].pda, loadoutInfo[player].pda == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem5"]), loadoutInfo[player].pda2, loadoutInfo[player].pda2 == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem6"]), loadoutInfo[player].building, loadoutInfo[player].building == loadoutInfo[player].activeWeaponSlot);
								}
								else {
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem1"]), loadoutInfo[player].primary, loadoutInfo[player].primary == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem2"]), loadoutInfo[player].secondary, loadoutInfo[player].secondary == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem3"]), loadoutInfo[player].melee, loadoutInfo[player].melee == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem4"]), loadoutInfo[player].pda, loadoutInfo[player].pda == loadoutInfo[player].activeWeaponSlot);
									HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem5"]));
									HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem6"]));
								}
							}
							else if (loadoutInfo[player].tfclass == TFClass_Spy) {
								if (nonloadout->GetBool()) {
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem1"]), loadoutInfo[player].secondary, loadoutInfo[player].secondary == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem2"]), loadoutInfo[player].building, loadoutInfo[player].building == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem3"]), loadoutInfo[player].melee, loadoutInfo[player].melee == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem4"]), loadoutInfo[player].pda, loadoutInfo[player].pda == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem5"]), loadoutInfo[player].pda2, loadoutInfo[player].pda2 == loadoutInfo[player].activeWeaponSlot);
									HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem6"]));
								}
								else {
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem1"]), loadoutInfo[player].secondary, loadoutInfo[player].secondary == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem2"]), loadoutInfo[player].building, loadoutInfo[player].building == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem3"]), loadoutInfo[player].melee, loadoutInfo[player].melee == loadoutInfo[player].activeWeaponSlot);
									DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem4"]), loadoutInfo[player].pda2, loadoutInfo[player].pda2 == loadoutInfo[player].activeWeaponSlot);
									HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem5"]));
									HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem6"]));
								}
							}
							else {
								DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem1"]), loadoutInfo[player].primary, loadoutInfo[player].primary == loadoutInfo[player].activeWeaponSlot);
								DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem2"]), loadoutInfo[player].secondary, loadoutInfo[player].secondary == loadoutInfo[player].activeWeaponSlot);
								DisplayIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem3"]), loadoutInfo[player].melee, loadoutInfo[player].melee == loadoutInfo[player].activeWeaponSlot);
								HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem4"]));
								HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem5"]));
								HideIcon(dynamic_cast<vgui::ImagePanel *>(loadoutIconPanels[panelHandle]["LoadoutIconsItem6"]));
							}
						}

						break;
					}
				}
			}
		}
	}
}

void LoadoutIcons::HideIcon(vgui::ImagePanel *panel) {
	if (panel) {
		panel->SetVisible(false);
		panel->SetImage("../dev/null");
	}
}

void LoadoutIcons::InitIcons(vgui::EditablePanel *panel) {
	panel->LoadControlSettings("Resource/UI/LoadoutIcons.res");

	vgui::HPanel panelHandle = g_pVGui->PanelToHandle(panel->GetVParent());

	for (int i = 0; i < g_pVGuiPanel->GetChildCount(panel->GetVPanel()); i++) {
		vgui::VPANEL vpanel = g_pVGuiPanel->GetChild(panel->GetVPanel(), i);
		const char *name = g_pVGuiPanel->GetName(vpanel);
		vgui::Panel *panel = g_pVGuiPanel->GetPanel(vpanel, "ClientDLL");

		loadoutIconPanels[panelHandle][name] = panel;
	}
}

int LoadoutIcons::GetCurrentFilter(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	std::stringstream ss(partial);
	std::string command;
	std::getline(ss, command, ' ');

	if (stricmp(command.c_str(), "statusspec_loadouticons_filter_active") == 0) {
		V_snprintf(commands[0], COMMAND_COMPLETION_ITEM_LENGTH, "%s %i %i %i %i", command.c_str(), filter_active_color.r(), filter_active_color.g(), filter_active_color.b(), filter_active_color.a());

		return 1;
	}
	else if (stricmp(command.c_str(), "statusspec_loadouticons_filter_inactive") == 0) {
		V_snprintf(commands[0], COMMAND_COMPLETION_ITEM_LENGTH, "%s %i %i %i %i", command.c_str(), filter_inactive_color.r(), filter_inactive_color.g(), filter_inactive_color.b(), filter_inactive_color.a());

		return 1;
	}
	else {
		return 0;
	}
}

void LoadoutIcons::SetFilter(const CCommand &command) {
	if (command.ArgC() == 0) {
		if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_active")) {
			Msg("The current active loadout item icon filter is rgba(%i, %i, %i, %i).\n", filter_active_color.r(), filter_active_color.g(), filter_active_color.b(), filter_active_color.a());
			return;
		}
		else if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_inactive")) {
			Msg("The current inactive loadout item icon filter is rgba(%i, %i, %i, %i).\n", filter_inactive_color.r(), filter_inactive_color.g(), filter_inactive_color.b(), filter_inactive_color.a());
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
		filter_active_color.SetColor(red, green, blue, alpha);
		Msg("Set active loadout item icon filter to rgba(%i, %i, %i, %i).\n", red, green, blue, alpha);
	}
	else if (stricmp(command.Arg(0), "statusspec_loadouticons_filter_inactive")) {
		filter_inactive_color.SetColor(red, green, blue, alpha);
		Msg("Set inactive loadout item icon filter to rgba(%i, %i, %i, %i).\n", red, green, blue, alpha);
	}
	else {
		Warning("Unrecognized command!\n");
	}
}

void LoadoutIcons::ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue) {
	if (enabled->GetBool()) {
		if (!frameHook) {
			frameHook = Funcs::AddHook_IBaseClientDLL_FrameStageNotify(Interfaces::pClientDLL, SH_MEMBER(this, &LoadoutIcons::FrameHook), true);
		}
	}
	else {
		DisableHUD();

		if (frameHook) {
			if (Funcs::RemoveHook(frameHook)) {
				frameHook = 0;
			}
		}
	}
}