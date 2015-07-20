/*
 *  customtextures.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../common.h"
#include "../modules.h"

class CCommand;
class ConCommand;
class ConVar;
class IConVar;
class KeyValues;

class CustomTextures : public Module {
public:
	CustomTextures();

	static bool CheckDependencies();
private:
	struct Replacement {
		std::string group;
		std::string replacement;
	};

	KeyValues *textureConfig;
	std::map<std::string, Replacement> textureReplacements;

	ConVar *enabled;
	ConCommand *load_replacement_group;
	ConCommand *reload_settings;
	ConCommand *unload_replacement_group;
	void LoadReplacementGroup(const CCommand &command);
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void UnloadReplacementGroup(const CCommand &command);
};