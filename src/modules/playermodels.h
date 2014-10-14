/*
*  playermodels.h
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
#include <sstream>
#include <string>

#include "convar.h"
#include "ehandle.h"

#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"
#include "../player.h"

class PlayerModels {
public:
	PlayerModels();

	void SetModelIndexOverride(C_BaseEntity *instance, int index);
	void SetModelPointerOverride(C_BaseEntity *instance, const model_t *pModel);
private:
	KeyValues *modelConfig;
	bool setModelIndexDetoured;
	bool setModelPointerDetoured;

	const model_t *GetModelOverride(C_BaseEntity *entity, const model_t *model);

	ConVar *enabled;
	void ToggleEnabled(IConVar *var, const char *pOldValue, float flOldValue);
};

extern PlayerModels *g_PlayerModels;