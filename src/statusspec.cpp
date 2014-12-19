/*
 *  statusspec.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusspec.h"

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

	g_ModuleManager->LoadModule<AntiFreeze>("AntiFreeze");
	g_ModuleManager->LoadModule<CameraTools>("Camera Tools");
	g_ModuleManager->LoadModule<CustomMaterials>("Custom Materials");
	g_ModuleManager->LoadModule<CustomModels>("Custom Models");
	g_ModuleManager->LoadModule<CustomTextures>("Custom Textures");
	g_ModuleManager->LoadModule<FOVOverride>("FOV Override");
	g_ModuleManager->LoadModule<Killstreaks>("Killstreaks");
	g_ModuleManager->LoadModule<LocalPlayer>("Local Player");
	g_ModuleManager->LoadModule<MedigunInfo>("Medigun Info");
	
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