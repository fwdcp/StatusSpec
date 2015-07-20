/*
 *  custommaterials.h
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
class IMaterial;
class KeyValues;

class CustomMaterials : public Module {
public:
	CustomMaterials();

	static bool CheckDependencies();
private:
	IMaterial *FindMaterialOverride(char const *pMaterialName, const char *pTextureGroupName, bool complain = true, const char *pComplainPrefix = NULL);

	struct Replacement {
		std::string group;
		std::string replacement;
	};

	int findMaterialHook;
	KeyValues *materialConfig;
	std::map<std::string, Replacement> materialReplacements;

	ConVar *enabled;
	ConCommand *load_replacement_group;
	ConCommand *reload_settings;
	ConCommand *unload_replacement_group;
	void LoadReplacementGroup(const CCommand &command);
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void UnloadReplacementGroup(const CCommand &command);
};