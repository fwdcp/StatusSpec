/*
 *  specguiorder.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <set>

#include "cdll_int.h"
#include "vgui/VGUI.h"

#include "../modules.h"
#include "../player.h"

class ConVar;
class IConVar;
class KeyValues;

class SpecGUIOrder : public Module {
public:
	SpecGUIOrder(std::string name);

	static bool CheckDependencies(std::string name);

	void FrameHook(ClientFrameStage_t curStage);
	void SetPosOverride(vgui::VPANEL vguiPanel, int x, int y);
private:
	std::set<Player> bluPlayers;
	int frameHook;
	int setPosHook;
	KeyValues *specguiSettings;
	std::set<Player> redPlayers;

	ConVar *enabled;
	ConVar *reverse_blu;
	ConVar *reverse_red;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};