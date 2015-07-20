/*
*  custommodels.h
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

class C_BaseEntity;
class CCommand;
class ConCommand;
class ConVar;
class IConVar;
class KeyValues;
struct model_t;

class CustomModels : public Module {
public:
	CustomModels();

	static bool CheckDependencies();
private:
	void SetModelOverride(C_BaseEntity *entity, const model_t *&model);

	struct Replacement {
		std::string group;
		std::string replacement;
	};

	KeyValues *modelConfig;
	std::map<std::string, Replacement> modelReplacements;
	int setModelHook;

	ConVar *enabled;
	ConCommand *load_replacement_group;
	ConCommand *reload_settings;
	ConCommand *unload_replacement_group;
	void LoadReplacementGroup(const CCommand &command);
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
	void UnloadReplacementGroup(const CCommand &command);
};