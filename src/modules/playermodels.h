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

#if defined _WIN32
#define strtoull _strtoui64
#endif

class PlayerModels {
public:
	PlayerModels();

	bool IsEnabled();

	const model_t *SetModelOverride(C_BaseEntity *entity, const model_t *model);
private:
	KeyValues *modelConfig;

	ConVar *enabled;
};

extern PlayerModels *g_PlayerModels;