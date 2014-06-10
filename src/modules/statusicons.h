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

#include "../stdafx.h"

#include <cstdint>
#include <map>
#include <string>

#include "convar.h"
#include "vgui/IPanel.h"

#include "../entities.h"
#include "../enums.h"
#include "../ifaces.h"
#include "../paint.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

#define TEXTURE_NULL "vgui/replay/thumbnails/null"
#define TEXTURE_UBERCHARGE "vgui/replay/thumbnails/ubercharge"
#define TEXTURE_CRITBOOST "vgui/replay/thumbnails/critboost"
#define TEXTURE_MEGAHEALRED "vgui/replay/thumbnails/megaheal_red"
#define TEXTURE_MEGAHEALBLU "vgui/replay/thumbnails/megaheal_blue"
#define TEXTURE_RESISTSHIELDRED "vgui/replay/thumbnails/resist_shield"
#define TEXTURE_RESISTSHIELDBLU "vgui/replay/thumbnails/resist_shield_blue"
#define TEXTURE_BULLETRESISTRED "vgui/replay/thumbnails/defense_buff_bullet_red"
#define TEXTURE_BLASTRESISTRED "vgui/replay/thumbnails/defense_buff_explosion_red"
#define TEXTURE_FIRERESISTRED "vgui/replay/thumbnails/defense_buff_fire_red"
#define TEXTURE_BULLETRESISTBLU "vgui/replay/thumbnails/defense_buff_bullet_blue"
#define TEXTURE_BLASTRESISTBLU "vgui/replay/thumbnails/defense_buff_explosion_blue"
#define TEXTURE_FIRERESISTBLU "vgui/replay/thumbnails/defense_buff_fire_blue"
#define TEXTURE_BUFFBANNERRED "effects/soldier_buff_offense_red"
#define TEXTURE_BUFFBANNERBLU "effects/soldier_buff_offense_blue"
#define TEXTURE_BATTALIONSBACKUPRED "effects/soldier_buff_defense_red"
#define TEXTURE_BATTALIONSBACKUPBLU "effects/soldier_buff_defense_blue"
#define TEXTURE_CONCHERORRED "effects/soldier_buff_healonhit_red"
#define TEXTURE_CONCHERORBLU "effects/soldier_buff_healonhit_blue"
#define TEXTURE_JARATE "vgui/replay/thumbnails/jarated"
#define TEXTURE_MADMILK "vgui/bleed_drop"
#define TEXTURE_MARKFORDEATH "vgui/marked_for_death"
#define TEXTURE_BLEEDING "vgui/bleed_drop"
#define TEXTURE_FIRE "hud/leaderboard_class_pyro"

typedef struct Status_s {
	TFTeam team;
	uint32_t conditions[3];
} Status_t;

class StatusIcons {
public:
	StatusIcons();
	
	bool IsEnabled();

	void InterceptMessage(vgui::VPANEL vguiPanel, KeyValues *params, vgui::VPANEL ifromPanel);
	
	void NoPaint(vgui::VPANEL vguiPanel);
	void Paint(vgui::VPANEL vguiPanel);
	
	void PreEntityUpdate();
	void ProcessEntity(IClientEntity* entity);
private:
	std::map<std::string, int> playerPanels;
	std::map<int, Status_t> statusInfo;

	ConVar* enabled;
	ConVar* max_icons;
};

extern StatusIcons *g_StatusIcons;