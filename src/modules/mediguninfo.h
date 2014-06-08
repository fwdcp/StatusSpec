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
#include <string>

#include "vgui_controls/AnimationController.h"
#include "convar.h"
#include "vgui_controls/EditablePanel.h"
#include "ehandle.h"
#include "icliententity.h"
#include "iclientmode.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"

#include "../entities.h"
#include "../enums.h"
#include "../hooks.h"
#include "../ifaces.h"

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
	int itemDefinitionIndex;
	float chargeLevel;
	int chargeResistType;
	bool chargeRelease;
} Medigun_t;

class MedigunInfo {
public:
	void Paint(vgui::VPANEL vguiPanel);
	void Update();
private:
	MedigunInfo();
	~MedigunInfo();

	std::map<TFTeam, Medigun_t> medigunInfo;
	
	static vgui::HPanel mainPanel;
	static std::map<std::string, vgui::Panel *> panels;

	static ConVar charge_label_text;
	static ConVar enabled;
	static ConVar individual_charges_label_text;
	static ConCommand reload_settings;
	static ConCommand set_progress_bar_direction;
	static void ReloadSettings();
	static void SetProgressBarDirection(const CCommand &command);
	static void ToggleState(IConVar *var, const char *pOldValue, float flOldValue);
};

extern MedigunInfo *g_MedigunInfo;