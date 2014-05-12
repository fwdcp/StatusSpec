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
		Warning("[AS] %s is NULL!\n", #className); \
		return false; \
	}

CDllDemandLoader clientModule = NULL;

IBaseClientDLL* g_pClientDll;
IClientEntityList* g_pClientEntityList;
IVEngineClient* g_pEngineClient;
IGameResources* g_pGameResources;

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

bool LoadInterfaces(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
	ConnectTier1Libraries(&interfaceFactory, 1);
	ConnectTier2Libraries(&interfaceFactory, 1);
	ConnectTier3Libraries(&interfaceFactory, 1);
	
	IVEngineClient* g_pEngineClient = (IVEngineClient*) interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	
	#ifdef _POSIX
	CDllDemandLoader clientModule = CDllDemandLoader("tf/bin/client.so");
	#else
	CDllDemandLoader clientModule = CDllDemandLoader("tf/bin/client.dll");
	#endif;
	CreateInterfaceFn gameClientFactory = clientModule.GetFactory();
	
	static DWORD funcadd = NULL;
	if (!funcadd)
		funcadd = FindPattern((DWORD) GetHandleOfModule(_T("client")), 0x2680C6, (PBYTE) "\xA1\x00\x00\x00\x00\x85\xC0\x74\x06\x05", "x????xxxxx");
	typedef IGameResources* (*GGR_t) (void);
	GGR_t GGR = (GGR_t) funcadd;
	IGameResources* g_pGameResources = GGR();
	
	IBaseClientDLL* g_pClientDll = (IBaseClientDLL*) gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, NULL);
	IClientEntityList* g_pClientEntityList = (IClientEntityList*) gameClientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, NULL);

	CheckPointerAndWarn(g_pClientDll, IBaseClientDLL);
	CheckPointerAndWarn(g_pClientEntityList, IClientEntityList);
	CheckPointerAndWarn(g_pEngineClient, IVEngineClient);
	CheckPointerAndWarn(g_pGameResources, IGameResources);
	CheckPointerAndWarn(g_pVGuiSurface, vgui::ISurface);
	CheckPointerAndWarn(g_pVGui, vgui::IVGui);
	CheckPointerAndWarn(g_pVGuiPanel, vgui::IPanel);
	CheckPointerAndWarn(g_pVGuiSchemeManager, vgui::ISchemeManager);
	
	return true;
}

void UnloadInterfaces() {
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
	
	clientModule.Unload();
	clientModule = NULL;
	
	g_pClientDll = NULL;
	g_pClientEntityList = NULL;
	g_pEngineClient = NULL;
}