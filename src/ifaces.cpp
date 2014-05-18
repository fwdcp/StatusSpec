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

IBaseClientDLL* Interfaces::pClientDLL = NULL;
IClientEntityList* Interfaces::pClientEntityList = NULL;
IVEngineClient* Interfaces::pEngineClient = NULL;
CDllDemandLoader *Interfaces::pClientModule = NULL;

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
	static DWORD funcadd = NULL;
	if (!funcadd)
		funcadd = FindPattern((DWORD) GetHandleOfModule(_T("client")), 0x2680C6, (PBYTE) "\xA1\x00\x00\x00\x00\x85\xC0\x74\x06\x05", "x????xxxxx");
	typedef IGameResources* (*GGR_t) (void);
	GGR_t GGR = (GGR_t) funcadd;
	return GGR();
}

bool Interfaces::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
	ConnectTier1Libraries(&interfaceFactory, 1);
	ConnectTier2Libraries(&interfaceFactory, 1);
	ConnectTier3Libraries(&interfaceFactory, 1);
	
	pEngineClient = (IVEngineClient*) interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	
	#ifdef _POSIX
	pClientModule = new CDllDemandLoader("tf/bin/client.so");
	#else
	pClientModule = new CDllDemandLoader("tf/bin/client.dll");
	#endif
	CreateInterfaceFn gameClientFactory = pClientModule->GetFactory();
	
	pClientDLL = (IBaseClientDLL*) gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, NULL);
	pClientEntityList = (IClientEntityList*) gameClientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, NULL);
	
	CheckPointerAndWarn(pClientDLL, IBaseClientDLL);
	CheckPointerAndWarn(pClientEntityList, IClientEntityList);
	CheckPointerAndWarn(pEngineClient, IVEngineClient);
	CheckPointerAndWarn(g_pVGuiSurface, vgui::ISurface);
	CheckPointerAndWarn(g_pVGui, vgui::IVGui);
	CheckPointerAndWarn(g_pVGuiPanel, vgui::IPanel);
	CheckPointerAndWarn(g_pVGuiSchemeManager, vgui::ISchemeManager);
	CheckPointerAndWarn(g_pFullFileSystem, IFileSystem);
	
	DevMsg("Engine client %p, client DLL %p, VGUI %p\n", pEngineClient, pClientDLL, g_pVGui);
	
	return true;
}

void Interfaces::Unload() {
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
	
	pClientModule->Unload();
	pClientModule = NULL;
	
	pClientDLL = NULL;
	pClientEntityList = NULL;
	pEngineClient = NULL;
}