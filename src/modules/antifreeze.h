/*
 *  antifreeze.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include <string>

#include "convar.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "KeyValues.h"

#include "../ifaces.h"

#define PERFORM_LAYOUT_COMMAND new KeyValues("Command", "Command", "performlayout")
#define SPEC_GUI_NAME "specgui"
#define TOP_PANEL_NAME "MatSystemTopPanel"

class AntiFreeze {
public:
	AntiFreeze();

	bool IsEnabled();

	void Paint(vgui::VPANEL vguiPanel);
private:
	vgui::HPanel specguiPanel;
	vgui::HPanel topPanel;

	ConVar* enabled;
};

extern AntiFreeze *g_AntiFreeze;