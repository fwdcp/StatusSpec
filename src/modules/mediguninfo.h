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

#include "../stdafx.h"

#include <map>
#include <iomanip>
#include <string>

#include "convar.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Controls.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "../vgui_controls/ImageProgressBar.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui_controls/ScalableImagePanel.h"
#include "../vgui_controls/VariableLabel.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"
#include "../tfdefs.h"

#define VGUI_TEXTURE_NULL "replay/thumbnails/null"
#define VGUI_TEXTURE_UBERCHARGE "replay/thumbnails/ubercharge"
#define VGUI_TEXTURE_CRITBOOST "replay/thumbnails/critboost"
#define VGUI_TEXTURE_MEGAHEALRED "replay/thumbnails/megaheal_red"
#define VGUI_TEXTURE_MEGAHEALBLU "replay/thumbnails/megaheal_blue"
#define VGUI_TEXTURE_BULLETRESISTRED "replay/thumbnails/defense_buff_bullet_red"
#define VGUI_TEXTURE_BLASTRESISTRED "replay/thumbnails/defense_buff_explosion_red"
#define VGUI_TEXTURE_FIRERESISTRED "replay/thumbnails/defense_buff_fire_red"
#define VGUI_TEXTURE_BULLETRESISTBLU "replay/thumbnails/defense_buff_bullet_blue"
#define VGUI_TEXTURE_BLASTRESISTBLU "replay/thumbnails/defense_buff_explosion_blue"
#define VGUI_TEXTURE_FIRERESISTBLU "replay/thumbnails/defense_buff_fire_blue"

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

	void FrameHook(ClientFrameStage_t curStage);
private:
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