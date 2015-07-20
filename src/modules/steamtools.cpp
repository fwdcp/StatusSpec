/*
 *  steamtools.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "steamtools.h"

#include "convar.h"
#include "steam/steam_api.h"

#include "../common.h"
#include "../funcs.h"
#include "../ifaces.h"

SteamTools::SteamTools() {
	rich_presence_status = new ConVar("statusspec_steamtools_rich_presence_status", "", FCVAR_NONE, "the rich presence status displayed to Steam", [](IConVar *var, const char *pOldValue, float flOldValue) { g_ModuleManager->GetModule<SteamTools>()->ChangeRichPresenceStatus(var, pOldValue, flOldValue); });
}

bool SteamTools::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::steamLibrariesAvailable) {
		PRINT_TAG();
		Warning("Required Steam libraries for module %s not available!\n", g_ModuleManager->GetModuleName<SteamTools>().c_str());

		ready = false;
	}

	return ready;
}

void SteamTools::ChangeRichPresenceStatus(IConVar *var, const char *pOldValue, float flOldValue) {
	Interfaces::pSteamAPIContext->SteamFriends()->ClearRichPresence();
	Interfaces::pSteamAPIContext->SteamFriends()->SetRichPresence("status", rich_presence_status->GetString());
}