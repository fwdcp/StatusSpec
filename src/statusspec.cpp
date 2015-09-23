/*
 *  statusspec.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusspec.h"

#include "convar.h"

#include "common.h"
#include "funcs.h"
#include "ifaces.h"
#include "modules.h"

#include "modules/antifreeze.h"
#include "modules/cameraautoswitch.h"
#include "modules/camerasmooths.h"
#include "modules/camerastate.h"
#include "modules/cameratools.h"
#include "modules/consoletools.h"
#include "modules/custommaterials.h"
#include "modules/custommodels.h"
#include "modules/customtextures.h"
#include "modules/filesystemtools.h"
#include "modules/fovoverride.h"
#include "modules/freezeinfo.h"
#include "modules/killstreaks.h"
#include "modules/localplayer.h"
#include "modules/mediguninfo.h"
#include "modules/multipanel.h"
#include "modules/playeraliases.h"
#include "modules/playermodels.h"
#include "modules/projectileoutlines.h"
#include "modules/steamtools.h"
#include "modules/teamhealthcomparison.h"

ModuleManager *g_ModuleManager = nullptr;

// The plugin is a static singleton that is exported as an interface
StatusSpecPlugin g_StatusSpecPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(StatusSpecPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_StatusSpecPlugin);

StatusSpecPlugin::StatusSpecPlugin() {}
StatusSpecPlugin::~StatusSpecPlugin() {}

bool StatusSpecPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
	PRINT_TAG();
	ConColorMsg(Color(0, 255, 255, 255), "version %s | a Forward Command Post project (http://fwdcp.net)\n", PLUGIN_VERSION);

	PRINT_TAG();
	ConColorMsg(Color(255, 255, 0, 255), "Loading plugin...\n");

	Interfaces::Load(interfaceFactory, gameServerFactory);
	Funcs::Load();

	g_ModuleManager = new ModuleManager();

	g_ModuleManager->RegisterAndLoadModule<AntiFreeze>("AntiFreeze");
	g_ModuleManager->RegisterAndLoadModule<CameraAutoSwitch>("Camera Auto Switch");
	g_ModuleManager->RegisterAndLoadModule<CameraSmooths>("Camera Smooths");
	g_ModuleManager->RegisterAndLoadModule<CameraState>("Camera State");
	g_ModuleManager->RegisterAndLoadModule<CameraTools>("Camera Tools");
	g_ModuleManager->RegisterAndLoadModule<ConsoleTools>("Console Tools");
	g_ModuleManager->RegisterAndLoadModule<CustomMaterials>("Custom Materials");
	g_ModuleManager->RegisterAndLoadModule<CustomModels>("Custom Models");
	g_ModuleManager->RegisterAndLoadModule<CustomTextures>("Custom Textures");
	g_ModuleManager->RegisterAndLoadModule<FilesystemTools>("Filesystem Tools");
	g_ModuleManager->RegisterAndLoadModule<FOVOverride>("FOV Override");
	g_ModuleManager->RegisterAndLoadModule<FreezeInfo>("Freeze Info");
	g_ModuleManager->RegisterAndLoadModule<Killstreaks>("Killstreaks");
	g_ModuleManager->RegisterAndLoadModule<LocalPlayer>("Local Player");
	g_ModuleManager->RegisterAndLoadModule<MedigunInfo>("Medigun Info");
	g_ModuleManager->RegisterAndLoadModule<MultiPanel>("MultiPanel");
	g_ModuleManager->RegisterAndLoadModule<PlayerAliases>("Player Aliases");
	g_ModuleManager->RegisterAndLoadModule<PlayerModels>("Player Models");
	g_ModuleManager->RegisterAndLoadModule<ProjectileOutlines>("Projectile Outlines");
	g_ModuleManager->RegisterAndLoadModule<SteamTools>("Steam Tools");
	g_ModuleManager->RegisterAndLoadModule<TeamHealthComparison>("Team Health Comparison");
	
	ConVar_Register();

	PRINT_TAG();
	ConColorMsg(Color(0, 255, 0, 255), "Finished loading!\n");

	return true;
}

void StatusSpecPlugin::Unload(void) {
	PRINT_TAG();
	ConColorMsg(Color(255, 255, 0, 255), "Unloading plugin...\n");

	g_ModuleManager->UnloadAllModules();

	Funcs::Unload();

	ConVar_Unregister();
	Interfaces::Unload();

	PRINT_TAG();
	ConColorMsg(Color(0, 255, 0, 255), "Finished unloading!\n");
}

void StatusSpecPlugin::Pause(void) {
	Funcs::Pause();
}

void StatusSpecPlugin::UnPause(void) {
	Funcs::Unpause();
}

const char *StatusSpecPlugin::GetPluginDescription(void) {
	std::stringstream ss;
	std::string desc;

	ss << "StatusSpec " << PLUGIN_VERSION;
	ss >> desc;

	return desc.c_str();
}

void StatusSpecPlugin::LevelInit(char const *pMapName) {}
void StatusSpecPlugin::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {}
void StatusSpecPlugin::GameFrame(bool simulating) {}
void StatusSpecPlugin::LevelShutdown(void) {}
void StatusSpecPlugin::ClientActive(edict_t *pEntity) {}
void StatusSpecPlugin::ClientDisconnect(edict_t *pEntity) {}
void StatusSpecPlugin::ClientPutInServer(edict_t *pEntity, char const *playername) {}
void StatusSpecPlugin::SetCommandClient(int index) {}
void StatusSpecPlugin::ClientSettingsChanged(edict_t *pEdict) {}
PLUGIN_RESULT StatusSpecPlugin::ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) { return PLUGIN_CONTINUE; }
PLUGIN_RESULT StatusSpecPlugin::ClientCommand(edict_t *pEntity, const CCommand &args) { return PLUGIN_CONTINUE; }
PLUGIN_RESULT StatusSpecPlugin::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) { return PLUGIN_CONTINUE; }
void StatusSpecPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {}
void StatusSpecPlugin::OnEdictAllocated(edict_t *edict) {}
void StatusSpecPlugin::OnEdictFreed(const edict_t *edict) {}