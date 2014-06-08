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
#include "KeyValues.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"

#include "../ifaces.h"

#define SPEC_GUI_NAME "specgui"
#define TOP_PANEL_NAME "MatSystemTopPanel"

class AntiFreeze {
public:
	void Paint(vgui::VPANEL vguiPanel);
private:
	static KeyValues* performLayoutCommand;
	static vgui::HPanel specguiPanel;
	static vgui::HPanel topPanel;

	static ConVar enabled;
	static void ToggleState(IConVar *var, const char *pOldValue, float flOldValue);
};

extern AntiFreeze *g_AntiFreeze;