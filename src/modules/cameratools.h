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

#include "convar.h"

#include "../modules.h"

class CCommand;
class ConCommand;
class ConVar;
class KeyValues;

class CameraTools : public Module {
public:
	CameraTools(std::string name);

	static bool CheckDependencies(std::string name);
private:
	KeyValues *specguiSettings;

	class HLTVCameraOverride;

	ConCommand *spec_player;
	ConVar *spec_player_alive;
	ConCommand *spec_pos;
	void SpecPlayer(const CCommand &command);
	void SpecPosition(const CCommand &command);
};