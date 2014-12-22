/*
 *  custommaterials.h
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
#include "../funcs.h"
#include "../modules.h"
#include "../ifaces.h"

class CustomMaterials : public Module {
public:
	CustomMaterials(std::string name);

	static bool CheckDependencies(std::string name);

	IMaterial *FindMaterialOverride(char const *pMaterialName, const char *pTextureGroupName, bool complain = true, const char *pComplainPrefix = NULL);
private:
	int findMaterialHook;
	KeyValues *materialConfig;
	std::map<std::string, Replacement> materialReplacements;

	ConVar *enabled;
	ConCommand *load_replacement_group;
	ConCommand *unload_replacement_group;
	void LoadReplacementGroup(const CCommand &command);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void UnloadReplacementGroup(const CCommand &command);
};