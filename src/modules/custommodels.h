/*
*  custommodels.h
*  StatusSpec project
*
*  Copyright (c) 2014 thesupremecommander
*  BSD 2-Clause License
*  http://opensource.org/licenses/BSD-2-Clause
*
*/

#pragma once

#include "../stdafx.h"

#include <functional>
#include <map>
#include <sstream>
#include <string>

#include "convar.h"
#include "ehandle.h"

#include "../common.h"
#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../modules.h"
#include "../player.h"

typedef struct ModelReplacement_s {
	std::string group;
	std::string replacement;
} ModelReplacement_t;

class CustomModels : public Module {
public:
	CustomModels(std::string name);

	static bool CheckDependencies(std::string name);
private:
	KeyValues *modelConfig;
	std::map<std::string, Replacement> modelReplacements;
	int setModelHook;

	void SetModelOverride(C_BaseEntity *entity, const model_t *&model);

	ConVar *enabled;
	ConCommand *load_replacement_group;
	ConCommand *unload_replacement_group;
	void LoadReplacementGroup(const CCommand &command);
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void UnloadReplacementGroup(const CCommand &command);
};