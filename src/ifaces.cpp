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
		Warning("[StatusSpec] %s is NULL!\n", #className); \
		return false; \
	}

IBaseClientDLL* Interfaces::pClientDLL = NULL;
IClientEntityList* Interfaces::pClientEntityList = NULL;
IVEngineClient* Interfaces::pEngineClient = NULL;
CSteamAPIContext* Interfaces::pSteamAPIContext = NULL;
CDllDemandLoader *Interfaces::pClientModule = NULL;

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

IGameResources* Interfaces::GetGameResources() {
#if defined _WIN32
	static DWORD pointer = NULL;
	if (!pointer)
		pointer = FindPattern((DWORD) GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE) GAMERESOURCES_SIG, GAMERESOURCES_MASK);
	typedef IGameResources* (*GGR_t) (void);
	GGR_t GGR = (GGR_t) pointer;
	return GGR();
#else
	return NULL;
#endif
}

IClientMode* Interfaces::GetClientMode() {
#if defined _WIN32
	static DWORD pointer = NULL;
	if (!pointer)
		pointer = FindPattern((DWORD) GetHandleOfModule(_T("client")), CLIENT_MODULE_SIZE, (PBYTE) CLIENTMODE_SIG, CLIENTMODE_MASK) + CLIENTMODE_OFFSET;
	return **(IClientMode***)(pointer);
#else
	return NULL;
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
	
	pEngineClient = (IVEngineClient*) interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	
	pClientModule = new CDllDemandLoader(CLIENT_MODULE_FILE);

	CreateInterfaceFn gameClientFactory = pClientModule->GetFactory();
	
	pClientDLL = (IBaseClientDLL*) gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, NULL);
	pClientEntityList = (IClientEntityList*) gameClientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, NULL);

	pSteamAPIContext = new CSteamAPIContext();
	if (!SteamAPI_InitSafe() || !pSteamAPIContext->Init()) {
		Warning("[StatusSpec] Could not initialize Steam API!\n");
		return false;
	}
	
	CheckPointerAndWarn(pClientDLL, IBaseClientDLL);
	CheckPointerAndWarn(pClientEntityList, IClientEntityList);
	CheckPointerAndWarn(pEngineClient, IVEngineClient);
	CheckPointerAndWarn(g_pVGuiSurface, vgui::ISurface);
	CheckPointerAndWarn(g_pVGui, vgui::IVGui);
	CheckPointerAndWarn(g_pVGuiPanel, vgui::IPanel);
	CheckPointerAndWarn(g_pVGuiSchemeManager, vgui::ISchemeManager);
	CheckPointerAndWarn(g_pFullFileSystem, IFileSystem);

	g_pEntityList = dynamic_cast<CBaseEntityList *>(Interfaces::pClientEntityList);
	
	return true;
}

void Interfaces::Unload() {
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
	
	pSteamAPIContext->Clear();

	pClientModule->Unload();
	pClientModule = NULL;
	
	pClientDLL = NULL;
	pClientEntityList = NULL;
	pEngineClient = NULL;
}