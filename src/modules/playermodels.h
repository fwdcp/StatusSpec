/*
 *  playermodels.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../modules.h"

class C_BaseEntity;
class ConCommand;
class ConVar;
class KeyValues;
class IConVar;
struct model_t;

class PlayerModels : public Module {
public:
	PlayerModels();

	static bool CheckDependencies();
private:
	void SetModelOverride(C_BaseEntity *entity, const model_t *&model);

	KeyValues *modelConfig;
	int setModelHook;

	ConVar *enabled;
	ConCommand *reload_settings;
	void ReloadSettings();
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};