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

#include "../stdafx.h"

#include <array>
#include <list>
#include <map>
#include <string>

#include "convar.h"
#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"
#include "../tfdefs.h"

class SpecGUIOrder : public Module {
public:
	SpecGUIOrder(std::string name);

	static bool CheckDependencies(std::string name);

	void FrameHook(ClientFrameStage_t curStage);
	void SetPosOverride(vgui::VPANEL vguiPanel, int x, int y);
private:
	std::list<Player> bluPlayers;
	int frameHook;
	int setPosHook;
	KeyValues *specguiSettings;
	std::list<Player> redPlayers;

	ConVar *enabled;
	ConVar *reverse_blu;
	ConVar *reverse_red;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};