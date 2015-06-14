/*
 *  loadouticons.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "loadouticons.h"

#include <iomanip>
#include <map>

#include "cbase.h"
#include "c_baseentity.h"
#include "convar.h"
#include "icliententitylist.h"
#include "iclientmode.h"
#include "KeyValues.h"
#include "shareddefs.h"
#include "tier3/tier3.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Panel.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../itemschema.h"
#include "../modules.h"

LoadoutIcons::LoadoutIcons(std::string name) : Module(name) {
	filter_active_color = Color(255, 255, 255, 255);
	filter_inactive_color = Color(127, 127, 127, 255);
	frameHook = 0;
	itemSchema = new ItemSchema();

	enabled = new ConVar("statusspec_loadouticons_enabled", "0", FCVAR_NONE, "enable loadout icons", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<LoadoutIcons>("Loadout Icons")->ToggleEnabled(var, pOldValue, flOldValue); });
	filter_active = new ConCommand("statusspec_loadouticons_filter_active", [](const CCommand &command) { g_ModuleManager->GetModule<LoadoutIcons>("Loadout Icons")->SetFilter(command); }, "the RGBA filter applied to the icon for an active item", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<LoadoutIcons>("Loadout Icons")->GetCurrentFilter(partial, commands); });
	filter_inactive = new ConCommand("statusspec_loadouticons_filter_inactive", [](const CCommand &command) { g_ModuleManager->GetModule<LoadoutIcons>("Loadout Icons")->SetFilter(command); }, "the RGBA filter applied to the icon for an inactive item", FCVAR_NONE, [](const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])->int { return g_ModuleManager->GetModule<LoadoutIcons>("Loadout Icons")->GetCurrentFilter(partial, commands); });
	nonloadout = new ConVar("statusspec_loadouticons_nonloadout", "0", FCVAR_NONE, "enable loadout icons for nonloadout items");
	only_active = new ConVar("statusspec_loadouticons_only_active", "0", FCVAR_NONE, "only display loadout icons for the active weapon");
}

bool LoadoutIcons::CheckDependencies(std::string name) {
	bool ready = true;

	if (!Interfaces::pClientDLL) {
		PRINT_TAG();
		Warning("Required interface IBaseClientDLL for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::pClientEntityList) {
		PRINT_TAG();
		Warning("Required interface IClientEntityList for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Interfaces::vguiLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required VGUI library for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGui) {
		PRINT_TAG();
		Warning("Required interface vgui::IVGui for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!g_pVGuiPanel) {
		PRINT_TAG();
		Warning("Required interface vgui::IPanel for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CEconEntity", { "m_hOwnerEntity" })) {
		PRINT_TAG();
		Warning("Required property m_hOwnerEntity for CEconEntity for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CEconEntity", { "m_iItemDefinitionIndex" })) {
		PRINT_TAG();
		Warning("Required property m_hOwnerEntity for CEconEntity for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_hActiveWeapon" })) {
		PRINT_TAG();
		Warning("Required property m_hActiveWeapon for CTFPlayer for module %s not available!\n", name.c_str());

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

	if (!ItemSchema::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required item schema helper class for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		Warning("Required player helper class for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::classRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player class retrieval for module %s not available!\n", name.c_str());

		ready = false;
	}

	if (!Player::nameRetrievalAvailable) {
		PRINT_TAG();
		Warning("Required player name retrieval for module %s not available!\n", name.c_str());

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

void LoadoutIcons::FrameHook(ClientFrameStage_t curStage) {
	if (curStage == FRAME_NET_UPDATE_END) {
		loadoutInfo.clear();

		int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

		for (int i = 0; i < maxEntity; i++) {
			IClientEntity *entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity || !Entities::CheckEntityBaseclass(entity, "EconEntity")) {
				continue;
			}

			Player player = Entities::GetEntityProp<EHANDLE *>(entity, { "m_hOwnerEntity" })->GetEntryIndex();
			
			if (!player) {
				continue;
			}

			int itemDefinitionIndex = *Entities::GetEntityProp<int *>(entity, { "m_iItemDefinitionIndex" });

			if (itemIconTextures.find(itemDefinitionIndex) == itemIconTextures.end()) {
				itemIconTextures[itemDefinitionIndex] = std::string("../") + itemSchema->GetItemKeyData(itemDefinitionIndex, "image_inventory");
			}

			const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");
			
			TFClassType tfclass = player.GetClass();
			KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
			if (classUses) {
				const char *classUse = classUses->GetString(TFDefinitions::classNames.find(tfclass)->second.c_str(), "");

				if (std::find(std::begin(TFDefinitions::itemSlots), std::end(TFDefinitions::itemSlots), classUse) != std::end(TFDefinitions::itemSlots)) {
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

			int activeWeapon = Entities::GetEntityProp<EHANDLE *>(player.GetEntity(), { "m_hActiveWeapon" })->GetEntryIndex();
			if (activeWeapon == entity->entindex()) {
				loadoutInfo[player].activeWeaponSlot = itemDefinitionIndex;
			}
		}

		for (Player player : Player::Iterable()) {
			TFClassType tfclass = player.GetClass();
			int activeWeapon = Entities::GetEntityProp<EHANDLE *>(player.GetEntity(), { "m_hActiveWeapon" })->GetEntryIndex();

			loadoutInfo[player].tfclass = tfclass;

			for (int i = 0; i < MAX_WEAPONS; i++) {
				std::stringstream ss;
				std::string arrayIndex;
				ss << std::setfill('0') << std::setw(3) << i;
				ss >> arrayIndex;

				IClientEntity *weapon = Entities::GetEntityProp<EHANDLE *>(player.GetEntity(), { "m_hMyWeapons", arrayIndex })->Get();

				if (!weapon || !Entities::CheckEntityBaseclass(weapon, "EconEntity")) {
					continue;
				}

				int itemDefinitionIndex = *Entities::GetEntityProp<int *>(weapon, { "m_iItemDefinitionIndex" });

				const char *itemSlot = itemSchema->GetItemKeyData(itemDefinitionIndex, "item_slot");

				KeyValues *classUses = itemSchema->GetItemKey(itemDefinitionIndex, "used_by_classes");
				if (classUses) {
					const char *classUse = classUses->GetString(TFDefinitions::classNames.find(tfclass)->second.c_str(), "");

					if (std::find(std::begin(TFDefinitions::itemSlots), std::end(TFDefinitions::itemSlots), classUse) != std::end(TFDefinitions::itemSlots)) {
						itemSlot = classUse;
					}
				}

				if (activeWeapon == weapon->entindex()) {
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
										else {
											DisplayIcons(playerPanel);
										}
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

				for (Player player : Player::Iterable()) {
					if (player.GetName().compare(name) == 0) {
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

	if (panel->GetChildCount() == 0) {
		vgui::EditablePanel *editablePanel = new vgui::EditablePanel(panel, "LoadoutIcons");
		editablePanel->LoadControlSettings("Resource/UI/LoadoutIcons.res");

		vgui::HPanel panelHandle = g_pVGui->PanelToHandle(panel->GetVParent());

		while (editablePanel->GetChildCount() > 0) {
			vgui::Panel *childPanel = editablePanel->GetChild(0);
			loadoutIconPanels[panelHandle][childPanel->GetName()] = childPanel;

			childPanel->SetParent(panel);
		}

		delete editablePanel;

		panel->LoadControlSettings("Resource/UI/LoadoutIcons.res");
	}
	else {
		vgui::HPanel panelHandle = g_pVGui->PanelToHandle(panel->GetVParent());

		for (int i = 0; i < g_pVGuiPanel->GetChildCount(panel->GetVPanel()); i++) {
			vgui::VPANEL vpanel = g_pVGuiPanel->GetChild(panel->GetVPanel(), i);
			const char *name = g_pVGuiPanel->GetName(vpanel);
			vgui::Panel *panel = g_pVGuiPanel->GetPanel(vpanel, "ClientDLL");

			loadoutIconPanels[panelHandle][name] = panel;
		}
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