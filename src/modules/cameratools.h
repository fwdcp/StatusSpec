/*
 *  cameratools.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../stdafx.h"

#include <algorithm>
#include <array>
#include <vector>

#include "convar.h"
#include "vgui/IScheme.h"
#include "vgui_controls/EditablePanel.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"
#include "../tfdefs.h"

class CameraTools : public Module {
public:
	CameraTools(std::string name);

	static bool CheckDependencies(std::string name);
private:
	KeyValues *specguiSettings;

	ConCommand *spec_player;
	ConVar *spec_player_alive;
	ConCommand *spec_pos;
	void SpecPlayer(const CCommand &command);
	void SpecPosition(const CCommand &command);
};