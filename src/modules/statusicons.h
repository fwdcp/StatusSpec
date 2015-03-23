/*
 *  statusicons.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "cdll_int.h"
#include "vgui/VGUI.h"

#include "../modules.h"

class ConVar;
class IConVar;

namespace vgui {
	class EditablePanel;
};

class StatusIcons : public Module {
public:
	StatusIcons(std::string name);

	static bool CheckDependencies(std::string name);

	void FrameHook(ClientFrameStage_t curStage);
private:
	int frameHook;

	void ClearIcons(vgui::VPANEL statusIconsPanel);
	void DisableHUD();
	void DisplayIcon(vgui::EditablePanel *panel, const char *iconTexture);
	void DisplayIcons(vgui::VPANEL playerPanel);

	ConVar *delta_x;
	ConVar *delta_y;
	ConVar *enabled;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};