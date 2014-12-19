/*
 *  customtextures.h
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

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"
#include "../modules.h"

typedef struct TextureReplacement_s {
	std::string group;
	std::string replacement;
} TextureReplacement_t;

class CustomTextures : public Module {
public:
	CustomTextures(std::string name);

	static bool CheckDependencies(std::string name);
private:
	KeyValues *textureConfig;
	std::map<std::string, Replacement> textureReplacements;

	ConVar *enabled;
	ConCommand *load_replacement_group;
	ConCommand *unload_replacement_group;
	void LoadReplacementGroup(const CCommand &command);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void UnloadReplacementGroup(const CCommand &command);
};