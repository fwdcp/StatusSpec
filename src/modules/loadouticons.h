/*
 *  loadouticons.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include <map>
#include <iomanip>
#include <sstream>
#include <string>

#include "Color.h"
#include "convar.h"
#include "KeyValues.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"
#include "../itemschema.h"
#include "../modules.h"
#include "../player.h"
#include "../tfdefs.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

#define MAX_COSMETIC_SLOTS 3
#define MAX_TAUNT_SLOTS 8

typedef struct Loadout_s {
	Loadout_s() {
		tfclass = TFClass_Unknown;
		primary = -1;
		secondary = -1;
		melee = -1;
		pda = -1;
		pda2 = -1;
		building = -1;
		std::fill(&cosmetic[0], &cosmetic[MAX_COSMETIC_SLOTS], -1);
		std::fill(&taunt[0], &taunt[MAX_TAUNT_SLOTS], -1);
		action = -1;
		activeWeaponSlot = -1;
	};

	TFClassType tfclass;
	int primary;
	int secondary;
	int melee;
	int pda;
	int pda2;
	int building;
	int cosmetic[MAX_COSMETIC_SLOTS];
	int taunt[MAX_TAUNT_SLOTS];
	int action;
	int activeWeaponSlot;
} Loadout_t;

class LoadoutIcons : public Module {
public:
	LoadoutIcons(std::string name);

	static bool CheckDependencies(std::string name);

	void FrameHook(ClientFrameStage_t curStage);
private:
	Color filter_active_color;
	Color filter_inactive_color;
	int frameHook;
	std::map<int, std::string> itemIconTextures;
	ItemSchema *itemSchema;
	std::map<Player, Loadout_t> loadoutInfo;
	std::map<vgui::HPanel, std::map<std::string, vgui::Panel *>> loadoutIconPanels;

	void DisableHUD();
	void DisplayIcon(vgui::ImagePanel *panel, int itemDefinitionIndex, bool active);
	void DisplayIcons(vgui::VPANEL playerPanel);
	void HideIcon(vgui::ImagePanel *panel);
	void InitIcons(vgui::EditablePanel *panel);

	ConVar *enabled;
	ConCommand *filter_active;
	ConCommand *filter_inactive;
	ConVar *nonloadout;
	ConVar *only_active;
	int GetCurrentFilter(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	void SetFilter(const CCommand &command);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};