/*
 *  statusspec.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2013 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "statusspec.h"

inline bool DataCompare( const BYTE* pData, const BYTE* bSig, const char* szMask )
{
    for( ; *szMask; ++szMask, ++pData, ++bSig)
    {
        if( *szMask == 'x' && *pData != *bSig)
            return false;
    }
	
    return ( *szMask ) == NULL;
}

// Finds a pattern at the specified address
inline DWORD FindPattern ( DWORD dwAddress, DWORD dwSize, BYTE* pbSig, const char* szMask )
{
    for( DWORD i = NULL; i < dwSize; i++ )
    {
        if( DataCompare( (BYTE*) ( dwAddress + i ), pbSig, szMask ) )
            return (DWORD)( dwAddress + i );
    }
    return 0;
}

IGameResources* GetGameResources() {
	//IGameResources* res;
    static DWORD funcadd = NULL;
    if( !funcadd )
        funcadd = FindPattern( (DWORD) GetHandleOfModule( _T("client") ), 0x2680C6, (PBYTE) "\xA1\x00\x00\x00\x00\x85\xC0\x74\x06\x05", "x????xxxxx" );
        
    typedef IGameResources* (*GGR_t) (void);
    GGR_t GGR = (GGR_t) funcadd;
    return GGR();
}

static void icons_enabled_change(IConVar *var, const char *pOldValue, float flOldValue);
ConVar icons_enabled("statusspec_icons_enabled", "0", 0, "enable status icons", icons_enabled_change);
ConVar icons_size("statusspec_icons_size", "33", 0, "square size of status icons");
ConVar icons_max("statusspec_icons_max", "5", 0, "max number of icons to be rendered");
ConVar icons_blu_x_base("statusspec_icons_blu_x_base", "360", 0, "x-coordinate of the first BLU player");
ConVar icons_blu_x_delta("statusspec_icons_blu_x_delta", "0", 0, "amount to move in x-direction for next BLU player");
ConVar icons_blu_y_base("statusspec_icons_blu_y_base", "497", 0, "y-coordinate of the first BLU player");
ConVar icons_blu_y_delta("statusspec_icons_blu_y_delta", "-33", 0, "amount to move in y-direction for next BLU player");
ConVar icons_blu_bg_red("statusspec_icons_blu_bg_red", "104", 0, "red value of the icon background for BLU players");
ConVar icons_blu_bg_green("statusspec_icons_blu_bg_green", "124", 0, "green value of the icon background for BLU players");
ConVar icons_blu_bg_blue("statusspec_icons_blu_bg_blue", "155", 0, "blue value of the icon background for BLU players");
ConVar icons_blu_bg_alpha("statusspec_icons_blu_bg_alpha", "127", 0, "alpha value of the icon background for BLU players");
ConVar icons_red_x_base("statusspec_icons_red_x_base", "360", 0, "x-coordinate of the first RED player");
ConVar icons_red_x_delta("statusspec_icons_red_x_delta", "0", 0, "amount to move in x-direction for next RED player");
ConVar icons_red_y_base("statusspec_icons_red_y_base", "542", 0, "y-coordinate of the first RED player");
ConVar icons_red_y_delta("statusspec_icons_red_y_delta", "33", 0, "amount to move in y-direction for next RED player");
ConVar icons_red_bg_red("statusspec_icons_red_bg_red", "180", 0, "red value of the icon background for RED players");
ConVar icons_red_bg_green("statusspec_icons_red_bg_green", "92", 0, "green value of the icon background for RED players");
ConVar icons_red_bg_blue("statusspec_icons_red_bg_blue", "77", 0, "blue value of the icon background for RED players");
ConVar icons_red_bg_alpha("statusspec_icons_red_bg_alpha", "127", 0, "alpha value of the icon background for RED players");

void (__fastcall *origPaintTraverse)(void* thisptr, int edx, VPANEL, bool, bool);

void UpdateEntities() {
	int iEntCount = pEntityList->GetHighestEntityIndex();
	IClientEntity *cEntity;
	
	playerInfo.clear();

	for (int i = 0; i < iEntCount; i++) {
		cEntity = pEntityList->GetClientEntity(i);

		// Ensure valid player entity
		if (cEntity == NULL || !(GetGameResources()->IsConnected(i))) {
			continue;
		}
		
		// get our stuff directly from entity data
		int team = *MakePtr(int*, cEntity, WSOffsets::pCTFPlayer__m_iTeamNum);
		uint32_t playerCond = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCond);
		uint32_t condBits = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer___condition_bits);
		uint32_t playerCondEx = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCondEx);
		uint32_t playerCondEx2 = *MakePtr(uint32_t*, cEntity, WSOffsets::pCTFPlayer__m_nPlayerCondEx2);
		
		if (team != TEAM_RED && team != TEAM_BLU) {
			continue;
		}
		
		int j = playerInfo.size();
		playerInfo.push_back(player_t());
		
		uint64_t playerCondExAdj = playerCondEx;
		playerCondExAdj <<= 32;
		
		playerInfo[j].team = team;
		playerInfo[j].lower = playerCond|condBits|playerCondExAdj;
		playerInfo[j].upper = playerCondEx2;
	}
}

void __fastcall hookedPaintTraverse( vgui::IPanel *thisPtr, int edx,  VPANEL vguiPanel, bool forceRepaint, bool allowForce = true ) {
	if (icons_enabled.GetBool()) {
		UpdateEntities();
	}
	
	origPaintTraverse(thisPtr, edx, vguiPanel, forceRepaint, allowForce);

	if (pEngineClient->IsDrawingLoadingImage() || !pEngineClient->IsInGame() || !pEngineClient->IsConnected() || pEngineClient->Con_IsVisible( ))
		return;
	
	const char* panelName = pPanel->GetName(vguiPanel);
	if (panelName[0] == 'M' && panelName[3] == 'S' && panelName[9] == 'T' && panelName[12] == 'P') {
		if (icons_enabled.GetBool()) {
			
			int iSize = icons_size.GetInt();
			int iMaxIcons = icons_max.GetInt();
			int iBluXBase = icons_blu_x_base.GetInt();
			int iBluXDelta = icons_blu_x_delta.GetInt();
			int iBluYBase = icons_blu_y_base.GetInt();
			int iBluYDelta = icons_blu_y_delta.GetInt();
			int iBluBGRed = icons_blu_bg_red.GetInt();
			int iBluBGGreen = icons_blu_bg_green.GetInt();
			int iBluBGBlue = icons_blu_bg_blue.GetInt();
			int iBluBGAlpha = icons_blu_bg_alpha.GetInt();
			int iRedXBase = icons_red_x_base.GetInt();
			int iRedXDelta = icons_red_x_delta.GetInt();
			int iRedYBase = icons_red_y_base.GetInt();
			int iRedYDelta = icons_red_y_delta.GetInt();
			int iRedBGRed = icons_red_bg_red.GetInt();
			int iRedBGGreen = icons_red_bg_green.GetInt();
			int iRedBGBlue = icons_red_bg_blue.GetInt();
			int iRedBGAlpha = icons_red_bg_alpha.GetInt();
			
			int iIconSize = iSize - 2;
			int iBarSize = iSize * iMaxIcons;
			
			int iPlayerCount = playerInfo.size();
			int iRedPlayers = -1;
			int iBluPlayers = -1;
			
			for (int i = 0; i < iPlayerCount; i++) {
				int iX = 0;
				int iY = 0;
				
				if (playerInfo[i].team == TEAM_RED) {
					iRedPlayers++;
					iX = iRedXBase + (iRedPlayers * iRedXDelta);
					iY = iRedYBase + (iRedPlayers * iRedYDelta);
					pSurface->DrawSetColor(iRedBGRed, iRedBGGreen, iRedBGBlue, iRedBGAlpha);
					pSurface->DrawFilledRect(iX, iY, iX + iBarSize, iY + iSize);
				}
				else if (playerInfo[i].team == TEAM_BLU) {
					iBluPlayers++;
					iX = iBluXBase + (iBluPlayers * iBluXDelta);
					iY = iBluYBase + (iBluPlayers * iBluYDelta);
					pSurface->DrawSetColor(iBluBGRed, iBluBGGreen, iBluBGBlue, iBluBGAlpha);
					pSurface->DrawFilledRect(iX, iY, iX + iBarSize, iY + iSize);
				}
				
				int iIcons = 0;
				
				if (playerInfo[i].lower & TFCOND_LOWER_UBERCHARGED) {
					pSurface->DrawSetTexture(m_iTextureUbercharged);
					pSurface->DrawSetColor(255, 255, 255, 255);
					
					if (playerInfo[i].team == TEAM_RED) {
						iX = iRedXBase + (iRedPlayers * iRedXDelta) + (iIcons * iSize) + 1;
						iY = iRedYBase + (iRedPlayers * iRedYDelta) + 1;
						pSurface->DrawTexturedRect(iX, iY, iX + iIconSize, iY + iIconSize);
					}
					else if (playerInfo[i].team == TEAM_BLU) {
						iX = iBluXBase + (iBluPlayers * iBluXDelta) + (iIcons * iSize) + 1;
						iY = iBluYBase + (iBluPlayers * iBluYDelta) + 1;
						pSurface->DrawTexturedRect(iX, iY, iX + iIconSize, iY + iIconSize);
					}
					
					iIcons++;
				}
				
				if (iIcons >= iMaxIcons) {
					continue;
				}
				
				if (playerInfo[i].lower & TFCOND_LOWER_KRITZKRIEGED) {
					pSurface->DrawSetTexture(m_iTextureCritBoosted);
					pSurface->DrawSetColor(255, 255, 255, 255);
					
					if (playerInfo[i].team == TEAM_RED) {
						iX = iRedXBase + (iRedPlayers * iRedXDelta) + (iIcons * iSize) + 1;
						iY = iRedYBase + (iRedPlayers * iRedYDelta) + 1;
						pSurface->DrawTexturedRect(iX, iY, iX + iIconSize, iY + iIconSize);
					}
					else if (playerInfo[i].team == TEAM_BLU) {
						iX = iBluXBase + (iBluPlayers * iBluXDelta) + (iIcons * iSize) + 1;
						iY = iBluYBase + (iBluPlayers * iBluYDelta) + 1;
						pSurface->DrawTexturedRect(iX, iY, iX + iIconSize, iY + iIconSize);
					}
					
					iIcons++;
				}
				
				if (iIcons >= iMaxIcons) {
					continue;
				}
				
				if (playerInfo[i].lower & (TFCOND_LOWER_MARKEDFORDEATH | TFCOND_LOWER_MARKEDFORDEATHSILENT)) {
					pSurface->DrawSetTexture(m_iTextureMarkedForDeath);
					pSurface->DrawSetColor(255, 255, 255, 255);
					
					if (playerInfo[i].team == TEAM_RED) {
						iX = iRedXBase + (iRedPlayers * iRedXDelta) + (iIcons * iSize) + 1;
						iY = iRedYBase + (iRedPlayers * iRedYDelta) + 1;
						pSurface->DrawTexturedRect(iX, iY, iX + iIconSize, iY + iIconSize);
					}
					else if (playerInfo[i].team == TEAM_BLU) {
						iX = iBluXBase + (iBluPlayers * iBluXDelta) + (iIcons * iSize) + 1;
						iY = iBluYBase + (iBluPlayers * iBluYDelta) + 1;
						pSurface->DrawTexturedRect(iX, iY, iX + iIconSize, iY + iIconSize);
					}
					
					iIcons++;
				}
				
				if (iIcons >= iMaxIcons) {
					continue;
				}
			}
		}
	}
}

// The plugin is a static singleton that is exported as an interface
StatusSpecPlugin g_StatusSpecPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(StatusSpecPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_StatusSpecPlugin );

StatusSpecPlugin::StatusSpecPlugin()
{
}

StatusSpecPlugin::~StatusSpecPlugin()
{
}

bool StatusSpecPlugin::Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	ConnectTier1Libraries( &interfaceFactory, 1 );

	void* hmClient = GetHandleOfModule("client");
	CreateInterfaceFn pfnClient = (CreateInterfaceFn) GetFuncAddress(hmClient, "CreateInterface");
	pClient     = (IBaseClientDLL*)    pfnClient("VClient017", NULL);
	pEntityList = (IClientEntityList*) pfnClient("VClientEntityList003", NULL);

	void* hmEngine = GetHandleOfModule("engine");
	CreateInterfaceFn pfnEngine = (CreateInterfaceFn) GetFuncAddress(hmEngine, "CreateInterface");
	pEngineClient = (IVEngineClient*) pfnEngine("VEngineClient013", NULL);

	void* hmVGUI2          = GetHandleOfModule("vgui2");
	void* hmVGUIMatSurface = GetHandleOfModule("vguimatsurface");
	CreateInterfaceFn pfnVGUI2          = (CreateInterfaceFn) GetFuncAddress(hmVGUI2, "CreateInterface");
	CreateInterfaceFn pfnVGUIMatSurface = (CreateInterfaceFn) GetFuncAddress(hmVGUIMatSurface, "CreateInterface");

	pPanel   = (vgui::IPanel*)   pfnVGUI2("VGUI_Panel009", NULL);
	pScheme  = (vgui::ISchemeManager*) pfnVGUI2("VGUI_Scheme010", NULL);
	pSurface = (vgui::ISurface*) pfnVGUIMatSurface("VGUI_Surface030", NULL);

	m_font = 0;
	
	m_iTextureUbercharged = pSurface->CreateNewTextureID();
	pSurface->DrawSetTextureFile(m_iTextureUbercharged, "vgui/replay/thumbnails/ubercharge", 0, false);
	m_iTextureCritBoosted = pSurface->CreateNewTextureID();
	pSurface->DrawSetTextureFile(m_iTextureCritBoosted, "vgui/replay/thumbnails/critboost", 0, false);
	m_iTextureMarkedForDeath = pSurface->CreateNewTextureID();
	pSurface->DrawSetTextureFile(m_iTextureMarkedForDeath, "vgui/marked_for_death", 0, false);

	//Hook PaintTraverse
	origPaintTraverse = (void (__fastcall *)(void *, int, VPANEL, bool, bool))
	HookVFunc(*(DWORD**)pPanel, Index_PaintTraverse, (DWORD*) &hookedPaintTraverse);

	// Get offsets
	WSOffsets::PrepareOffsets();

	//Everything seems ok!
	Msg("%s loaded!\n", PLUGIN_DESC);

	//Register cvars
	ConVar_Register( 0 );
	return true;
}

void StatusSpecPlugin::Unload( void )
{
	ConVar_Unregister( );
	DisconnectTier1Libraries();
}

void StatusSpecPlugin::FireGameEvent( KeyValues * event ) {}
void StatusSpecPlugin::Pause( void ){}
void StatusSpecPlugin::UnPause( void ){}
const char *StatusSpecPlugin::GetPluginDescription( void ){return PLUGIN_DESC;}
void StatusSpecPlugin::LevelInit( char const *pMapName ){}
void StatusSpecPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ){}
void StatusSpecPlugin::GameFrame( bool simulating ){}
void StatusSpecPlugin::LevelShutdown( void ){}
void StatusSpecPlugin::ClientActive( edict_t *pEntity ){}
void StatusSpecPlugin::ClientDisconnect( edict_t *pEntity ){}
void StatusSpecPlugin::ClientPutInServer( edict_t *pEntity, char const *playername ){}
void StatusSpecPlugin::SetCommandClient( int index ){}
void StatusSpecPlugin::ClientSettingsChanged( edict_t *pEdict ){}
PLUGIN_RESULT StatusSpecPlugin::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ){return PLUGIN_CONTINUE;}
PLUGIN_RESULT StatusSpecPlugin::ClientCommand( edict_t *pEntity, const CCommand &args ){return PLUGIN_CONTINUE;}
PLUGIN_RESULT StatusSpecPlugin::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID ){return PLUGIN_CONTINUE;}
void StatusSpecPlugin::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue ){}
void StatusSpecPlugin::OnEdictAllocated( edict_t *edict ){}
void StatusSpecPlugin::OnEdictFreed( const edict_t *edict ){}

static void icons_enabled_change(IConVar *var, const char *pOldValue, float flOldValue) {
	UpdateEntities();
}
