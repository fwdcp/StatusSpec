/*
 *  mediguninfo.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

class ConCommand;
class ConVar;
class KeyValues;

namespace vgui {
	class Panel;
};

#include "cdll_int.h"

#include "../modules.h"
#include "../tfdefs.h"

typedef struct Medigun_s {
	TFMedigun type;
	float chargeLevel;
	TFResistType chargeResistType;
	bool chargeRelease;
} Medigun_t;

class MedigunInfo : public Module {
public:
	MedigunInfo(std::string name);

	static bool CheckDependencies(std::string name);
private:
	void FrameHook(ClientFrameStage_t curStage);

	KeyValues *dynamicMeterSettings;
	int frameHook;
	std::map<TFTeam, Medigun_t> medigunInfo;
	std::map<std::string, vgui::Panel *> panels;

	void InitHud();
	void Paint();
	
	ConVar *dynamic_meters;
	ConVar *enabled;
	ConVar *individual_charge_meters;
	ConCommand *reload_settings;
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};