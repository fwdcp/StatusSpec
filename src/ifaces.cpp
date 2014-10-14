/**
 *  ifaces.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "ifaces.h"

#define CheckPointerAndWarn(pPointer, className) \
	if (pPointer == nullptr) { \
		Warning("[StatusSpec] %s is not initialized!\n", #className); \
		return false; \
	}

IBaseClientDLL *Interfaces::pClientDLL = nullptr;
IClientEntityList *Interfaces::pClientEntityList = nullptr;
CDllDemandLoader *Interfaces::pClientModule = nullptr;
IVEngineClient *Interfaces::pEngineClient = nullptr;
IFileSystem *Interfaces::pFileSystem = nullptr;
IGameEventManager2 *Interfaces::pGameEventManager = nullptr;
IVModelInfoClient *Interfaces::pModelInfoClient = nullptr;
IPlayerInfoManager *Interfaces::pPlayerInfoManager = nullptr;
IVRenderView *Interfaces::pRenderView = nullptr;
CSteamAPIContext *Interfaces::pSteamAPIContext = nullptr;

CBaseEntityList *g_pEntityList;

inline bool DataCompare(const BYTE* pData, const BYTE* bSig, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	
	return (*szMask) == NULL;
}

inline DWORD FindPattern(DWORD dwAddress, DWORD dwSize, BYTE* pbSig, const char* szMask)
{
	for (DWORD i = NULL; i < dwSize; i++)
	{
		if (DataCompare((BYTE*) (dwAddress + i), pbSig, szMask))
			return (DWORD) (dwAddress + i);
	}
	
	return 0;
}

IClientMode* Interfaces::GetClientMode() {
#if defined _WIN32
	static DWORD pointer = NULL;

	if (!pointer) {
		pointer = FindPattern((DWORD)GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE)CLIENTMODE_SIG, CLIENTMODE_MASK) + CLIENTMODE_OFFSET;

		if (!pointer) {
			throw bad_pointer("IClientMode");
		}
	}

	if (!**(IClientMode***)pointer) {
		throw bad_pointer("IClientMode");
	}

	return **(IClientMode***)(pointer);
#else
	throw bad_pointer("IClientMode");
#endif
}

IGameResources* Interfaces::GetGameResources() {
#if defined _WIN32
	static DWORD pointer = NULL;

	if (!pointer) {
		pointer = FindPattern((DWORD)GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE)GAMERESOURCES_SIG, GAMERESOURCES_MASK);

		if (!pointer) {
			throw bad_pointer("IGameResources");
		}
	}

	typedef IGameResources* (*GGR_t) (void);
	GGR_t GGR = (GGR_t) pointer;
	IGameResources *gr = GGR();

	if (!gr) {
		throw bad_pointer("IGameResources");
	}

	return gr;
#else
	throw bad_pointer("IGameResources");
#endif
}

C_HLTVCamera* Interfaces::GetHLTVCamera() {
#if defined _WIN32
	static DWORD pointer = NULL;

	if (!pointer) {
		pointer = FindPattern((DWORD)GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE)HLTVCAMERA_SIG, HLTVCAMERA_MASK) + HLTVCAMERA_OFFSET;

		if (!pointer) {
			throw bad_pointer("C_HLTVCamera");
		}
	}

	if (!*(C_HLTVCamera**)pointer) {
		throw bad_pointer("C_HLTVCamera");
	}

	return *(C_HLTVCamera**)(pointer);
#else
	throw bad_pointer("C_HLTVCamera");
#endif
}

bool Interfaces::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
	ConnectTier1Libraries(&interfaceFactory, 1);
	ConnectTier2Libraries(&interfaceFactory, 1);
	ConnectTier3Libraries(&interfaceFactory, 1);
	
	if (!vgui::VGui_InitInterfacesList("statusspec", &interfaceFactory, 1)) {
		Warning("[StatusSpec] Could not initialize VGUI interfaces!\n");
		return false;
	}
	
	pEngineClient = (IVEngineClient *)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr);
	pGameEventManager = (IGameEventManager2 *)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr);
	pModelInfoClient = (IVModelInfoClient *)interfaceFactory(VMODELINFO_CLIENT_INTERFACE_VERSION, nullptr);
	pPlayerInfoManager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER, nullptr);
	pRenderView = (IVRenderView *)interfaceFactory(VENGINE_RENDERVIEW_INTERFACE_VERSION, nullptr);
	
	pClientModule = new CDllDemandLoader(CLIENT_MODULE_FILE);

	CreateInterfaceFn gameClientFactory = pClientModule->GetFactory();
	
	pClientDLL = (IBaseClientDLL*)gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, nullptr);
	pClientEntityList = (IClientEntityList*)gameClientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, nullptr);

	pSteamAPIContext = new CSteamAPIContext();
	if (!SteamAPI_InitSafe() || !pSteamAPIContext->Init()) {
		Warning("[StatusSpec] Could not initialize Steam API!\n");
		return false;
	}
	
	CheckPointerAndWarn(pClientDLL, IBaseClientDLL);
	CheckPointerAndWarn(pClientEntityList, IClientEntityList);
	CheckPointerAndWarn(pEngineClient, IVEngineClient);
	CheckPointerAndWarn(pGameEventManager, IGameEventManager2);
	CheckPointerAndWarn(pModelInfoClient, IVModelInfoClient);
	CheckPointerAndWarn(pRenderView, IVRenderView);
	CheckPointerAndWarn(g_pFullFileSystem, IFileSystem);
	CheckPointerAndWarn(g_pMaterialSystem, IMaterialSystem);
	CheckPointerAndWarn(g_pMaterialSystemHardwareConfig, IMaterialSystemHardwareConfig);
	CheckPointerAndWarn(g_pStudioRender, IStudioRender);
	CheckPointerAndWarn(g_pVGuiSurface, vgui::ISurface);
	CheckPointerAndWarn(g_pVGui, vgui::IVGui);
	CheckPointerAndWarn(g_pVGuiPanel, vgui::IPanel);
	CheckPointerAndWarn(g_pVGuiSchemeManager, vgui::ISchemeManager);

	g_pEntityList = dynamic_cast<CBaseEntityList *>(Interfaces::pClientEntityList);

	char dll[MAX_PATH];
	bool steam;
	if (FileSystem_GetFileSystemDLLName(dll, sizeof(dll), steam) == FS_OK) {
		CFSLoadModuleInfo fsLoadModuleInfo;
		fsLoadModuleInfo.m_bSteam = steam;
		fsLoadModuleInfo.m_pFileSystemDLLName = dll;
		fsLoadModuleInfo.m_ConnectFactory = interfaceFactory;

		if (FileSystem_LoadFileSystemModule(fsLoadModuleInfo) == FS_OK) {
			CFSMountContentInfo fsMountContentInfo;
			fsMountContentInfo.m_bToolsMode = fsLoadModuleInfo.m_bToolsMode;
			fsMountContentInfo.m_pDirectoryName = fsLoadModuleInfo.m_GameInfoPath;
			fsMountContentInfo.m_pFileSystem = fsLoadModuleInfo.m_pFileSystem;

			if (FileSystem_MountContent(fsMountContentInfo) == FS_OK) {
				CFSSearchPathsInit fsSearchPathsInit;
				fsSearchPathsInit.m_pDirectoryName = fsLoadModuleInfo.m_GameInfoPath;
				fsSearchPathsInit.m_pFileSystem = fsLoadModuleInfo.m_pFileSystem;

				if (FileSystem_LoadSearchPaths(fsSearchPathsInit) == FS_OK) {
					Interfaces::pFileSystem = fsLoadModuleInfo.m_pFileSystem;

					CheckPointerAndWarn(Interfaces::pFileSystem, IFileSystem);

					return true;
				}
			}
		}
	}

	return false;
}

void Interfaces::Unload() {
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
	
	pSteamAPIContext->Clear();

	pClientModule->Unload();
	pClientModule = nullptr;
	
	pClientDLL = nullptr;
	pClientEntityList = nullptr;
	pEngineClient = nullptr;
	pFileSystem = nullptr;
	pGameEventManager = nullptr;
	pRenderView = nullptr;
}