/*
 *  steamtools.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../modules.h"

#include "steam/steamclientpublic.h"

class ConVar;
class IConVar;

class SteamTools : public Module {
public:
	SteamTools();

	static bool CheckDependencies();
private:
	ConVar *rich_presence_status;
	void ChangeRichPresenceStatus(IConVar *var, const char *pOldValue, float flOldValue);
};