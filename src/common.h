/**
 *  common.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "Color.h"
#include "dbg.h"
#include "steam/steamclientpublic.h"

class ConCommand;

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

typedef struct Replacement {
	std::string group;
	std::string replacement;
} Replacement;

typedef struct ColorConCommand {
	Color color;
	ConCommand *command;
} ColorConCommand;

inline void FindAndReplaceInString(std::string &str, const std::string &find, const std::string &replace) {
	if (find.empty())
		return;

	size_t start_pos = 0;

	while ((start_pos = str.find(find, start_pos)) != std::string::npos) {
		str.replace(start_pos, find.length(), replace);
		start_pos += replace.length();
	}
}

inline float ChangeScale(float currentValue, float currentMin, float currentMax, float newMin, float newMax) {
	float deltaScaler = ((newMax - newMin) / (currentMax - currentMin));
	float newDelta = ((currentValue - currentMin) * deltaScaler);
	float newValue = newMin + newDelta;

	return newValue;
}

inline int ColorRangeRestrict(int color) {
	if (color < 0) return 0;
	else if (color > 255) return 255;
	else return color;
}

inline bool IsInteger(const std::string &s) {
	if (s.empty() || !isdigit(s[0])) return false;

	char *p;
	strtoull(s.c_str(), &p, 10);

	return (*p == 0);
}

inline CSteamID ConvertTextToSteamID(std::string textID) {
	if (IsInteger(textID)) {
		uint64_t steamID = strtoull(textID.c_str(), nullptr, 10);

		return CSteamID(steamID);
	}

	return CSteamID();
}

inline std::string ConvertTreeToString(std::vector<std::string> tree) {
	std::stringstream ss;
	std::string string;

	for (auto iterator = tree.begin(); iterator != tree.end(); ++iterator) {
		ss << ">";
		ss << *iterator;
	}

	ss >> string;

	return string;
}

inline std::string GetVGUITexturePath(std::string normalTexturePath) {
	std::string path = "../";
	path += normalTexturePath;

	return path;
}

#define PLUGIN_VERSION "0.24.0"
#define PRINT_TAG() ConColorMsg(Color(0, 153, 153, 255), "[StatusSpec] ")