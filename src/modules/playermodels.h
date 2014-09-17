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
#include <string>

#include "convar.h"
#include "ehandle.h"
#include "iclientrenderable.h"

#include "../entities.h"
#include "../funcs.h"
#include "../ifaces.h"

#include "../statusspec.h"

#if defined _WIN32
#define strtoull _strtoui64
#endif

class PlayerModels {
public:
	PlayerModels();

	bool IsEnabled();

	const model_t *GetModelOverride(IClientRenderable *entity);

	void ProcessEntity(IClientEntity *entity);
private:
	std::map<EHANDLE, int> hooks;
	std::map<std::string, const model_t *> models;
	KeyValues *modelConfig;

	ConVar *enabled;
};

extern PlayerModels *g_PlayerModels;