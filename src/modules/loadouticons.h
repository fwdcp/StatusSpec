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

#if defined _WIN32
#define strtoull _strtoui64
#endif

#define MAX_COSMETIC_SLOTS 3

typedef struct Loadout_s {
	TFClassType tfclass;
	int primary;
	int secondary;
	int melee;
	int pda;
	int pda2;
	int building;
	int cosmetic[MAX_COSMETIC_SLOTS];
	int action;
	std::string activeWeaponSlot;
} Loadout_t;

class LoadoutIcons {
public:
	LoadoutIcons();
	~LoadoutIcons();

	bool IsEnabled();

	void InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);
	void Paint(vgui::VPANEL vguiPanel);
	void Update();
private:
	Color filter_active_color;
	Color filter_nonactive_color;
	std::map<int, std::string> itemIconTextures;
	ItemSchema* itemSchema;
	std::map<int, Loadout_t> loadoutInfo;
	std::map<std::string, int> playerPanels;

	ConVar* enabled;
	ConCommand* filter_active;
	ConCommand* filter_nonactive;
	ConVar* nonloadout;
	static int GetCurrentFilter(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
	static void SetFilter(const CCommand &command);
};

extern LoadoutIcons *g_LoadoutIcons;