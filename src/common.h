/**
 *  common.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
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
#include "strtools.h"

class ConCommand;

#define GAME_PANEL_MODULE "ClientDLL"

inline void FindAndReplaceInString(std::string &str, const std::string &find, const std::string &replace) {
	if (find.empty())
		return;

	size_t start_pos = 0;

	while ((start_pos = str.find(find, start_pos)) != std::string::npos) {
		str.replace(start_pos, find.length(), replace);
		start_pos += replace.length();
	}
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

	for (std::string branch : tree) {
		ss << ">";
		ss << branch;
	}

	ss >> string;

	return string;
}

inline void GetPropIndexString(int index, char string[]) {
	V_snprintf(string, sizeof(string), "%03i", index);
}

inline std::string GetVGUITexturePath(std::string normalTexturePath) {
	std::string path = "../";
	path += normalTexturePath;

	return path;
}

#define PLUGIN_VERSION "0.25.1"
#define PRINT_TAG() ConColorMsg(Color(0, 153, 153, 255), "[StatusSpec] ")