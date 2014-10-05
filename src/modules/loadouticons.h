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
#include <sstream>
#include <string>

#include "Color.h"
#include "convar.h"
#include "vgui/IPanel.h"
#include "KeyValues.h"

#include "../entities.h"
#include "../enums.h"
#include "../ifaces.h"
#include "../itemschema.h"
#include "../paint.h"
#include "../player.h"

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

class LoadoutIcons {
public:
	LoadoutIcons();
	~LoadoutIcons();

	bool IsEnabled();

	void InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);

	void Paint(vgui::VPANEL vguiPanel);
	
	void PreEntityUpdate();
	void ProcessEntity(IClientEntity* entity);
	void PostEntityUpdate();
private:
	Color filter_active_color;
	Color filter_nonactive_color;
	std::map<int, std::string> itemIconTextures;
	ItemSchema* itemSchema;
	std::map<Player, Loadout_t> loadoutInfo;
	std::map<std::string, Player> playerPanels;

	void DrawSlotIcon(Player player, int weapon, int &width, int size);

	ConVar* enabled;
	ConCommand* filter_active;
	ConCommand* filter_nonactive;
	ConVar* nonloadout;
	ConVar *only_active;
	static int GetCurrentFilter(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void SetFilter(const CCommand &command);
};

extern LoadoutIcons *g_LoadoutIcons;