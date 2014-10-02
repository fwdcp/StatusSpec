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

#include "../enums.h"
#include "../ifaces.h"
#include "../paint.h"
#include "../player.h"

class SpecGUIOrder {
public:
	SpecGUIOrder();

	bool IsEnabled();

	void InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);

	bool SetPosOverride(vgui::VPANEL vguiPanel, int &x, int &y);

	void PreEntityUpdate();
	void ProcessEntity(IClientEntity *entity);
	void PostEntityUpdate();
private:
	std::list<int> bluPlayers;
	std::map<std::string, int> playerPanels;
	KeyValues *specguiSettings;
	std::list<int> redPlayers;

	ConVar* enabled;
};

extern SpecGUIOrder *g_SpecGUIOrder;