/*
 *  statusspec.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2013 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef STATUSSPEC_H
#define STATUSSPEC_H

#define CLIENT_DLL

#include "stdafx.h"

#include "tier1/tier1.h"
#include "engine/iserverplugin.h"
#include "igameevents.h"

#include "offsets.h"
#include "vfuncs.h"
#include "icvar.h"

#define PLUGIN_DESC "StatusSpec v0.1"
#define SHORT_DESC "StatusSpec v0.1"

#define SCALED(normalValue) pScheme->GetProportionalScaledValue(normalValue)

#define TEAM_RED 2
#define TEAM_BLU 3

// taken from SourceMod and adapted

#define TFCOND_LOWER_SLOWED ((uint64_t) 1 << 0)
#define TFCOND_LOWER_ZOOMED ((uint64_t) 1 << 1)
#define TFCOND_LOWER_DISGUISING ((uint64_t) 1 << 2)
#define TFCOND_LOWER_DISGUISED ((uint64_t) 1 << 3)
#define TFCOND_LOWER_CLOAKED ((uint64_t) 1 << 4)
#define TFCOND_LOWER_UBERCHARGED ((uint64_t) 1 << 5)
#define TFCOND_LOWER_TELEPORTEDGLOW ((uint64_t) 1 << 6)
#define TFCOND_LOWER_TAUNTING ((uint64_t) 1 << 7)
#define TFCOND_LOWER_UBERCHARGEFADING ((uint64_t) 1 << 8)
#define TFCOND_LOWER_CLOAKFLICKER ((uint64_t) 1 << 9)
#define TFCOND_LOWER_TELEPORTING ((uint64_t) 1 << 10)
#define TFCOND_LOWER_KRITZKRIEGED ((uint64_t) 1 << 11)
#define TFCOND_LOWER_TMPDAMAGEBONUS ((uint64_t) 1 << 12)
#define TFCOND_LOWER_DEADRINGERED ((uint64_t) 1 << 13)
#define TFCOND_LOWER_BONKED ((uint64_t) 1 << 14)
#define TFCOND_LOWER_DAZED ((uint64_t) 1 << 15)
#define TFCOND_LOWER_BUFFED ((uint64_t) 1 << 16)
#define TFCOND_LOWER_CHARGING ((uint64_t) 1 << 17)
#define TFCOND_LOWER_DEMOBUFF ((uint64_t) 1 << 18)
#define TFCOND_LOWER_CRITCOLA ((uint64_t) 1 << 19)
#define TFCOND_LOWER_INHEALRADIUS ((uint64_t) 1 << 20)
#define TFCOND_LOWER_HEALING ((uint64_t) 1 << 21)
#define TFCOND_LOWER_ONFIRE ((uint64_t) 1 << 22)
#define TFCOND_LOWER_OVERHEALED ((uint64_t) 1 << 23)
#define TFCOND_LOWER_JARATED ((uint64_t) 1 << 24)
#define TFCOND_LOWER_BLEEDING ((uint64_t) 1 << 25)
#define TFCOND_LOWER_DEFENSEBUFFED ((uint64_t) 1 << 26)
#define TFCOND_LOWER_MILKED ((uint64_t) 1 << 27)
#define TFCOND_LOWER_MEGAHEAL ((uint64_t) 1 << 28)
#define TFCOND_LOWER_REGENBUFFED ((uint64_t) 1 << 29)
#define TFCOND_LOWER_MARKEDFORDEATH ((uint64_t) 1 << 30)
#define TFCOND_LOWER_NOHEALINGDAMAGEBUFF ((uint64_t) 1 << 31)
#define TFCOND_LOWER_SPEEDBUFFALLY ((uint64_t) 1 << 32)
#define TFCOND_LOWER_HALLOWEENCRITCANDY ((uint64_t) 1 << 33)
#define TFCOND_LOWER_CRITCANTEEN ((uint64_t) 1 << 34)
#define TFCOND_LOWER_CRITDEMOCHARGE ((uint64_t) 1 << 35)
#define TFCOND_LOWER_CRITHYPE ((uint64_t) 1 << 36)
#define TFCOND_LOWER_CRITONFIRSTBLOOD ((uint64_t) 1 << 37)
#define TFCOND_LOWER_CRITONWIN ((uint64_t) 1 << 38)
#define TFCOND_LOWER_CRITONFLAGCAPTURE ((uint64_t) 1 << 39)
#define TFCOND_LOWER_CRITONKILL ((uint64_t) 1 << 40)
#define TFCOND_LOWER_RESTRICTTOMELEE ((uint64_t) 1 << 41)
#define TFCOND_LOWER_DEFENSEBUFFNOCRITBLOCK ((uint64_t) 1 << 42)
#define TFCOND_LOWER_REPROGRAMMED ((uint64_t) 1 << 43)
#define TFCOND_LOWER_CRITMMMPH ((uint64_t) 1 << 44)
#define TFCOND_LOWER_DEFENSEBUFFMMMPH ((uint64_t) 1 << 45)
#define TFCOND_LOWER_FOCUSBUFF ((uint64_t) 1 << 46)
#define TFCOND_LOWER_DISGUISEREMOVED ((uint64_t) 1 << 47)
#define TFCOND_LOWER_MARKEDFORDEATHSILENT ((uint64_t) 1 << 48)
#define TFCOND_LOWER_DISGUISEDASDISPENSER ((uint64_t) 1 << 49)
#define TFCOND_LOWER_SAPPED ((uint64_t) 1 << 50)
#define TFCOND_LOWER_UBERCHARGEDHIDDEN ((uint64_t) 1 << 51)
#define TFCOND_LOWER_UBERCHARGEDCANTEEN ((uint64_t) 1 << 52)
#define TFCOND_LOWER_HALLOWEENBOMBHEAD ((uint64_t) 1 << 53)
#define TFCOND_LOWER_HALLOWEENTHRILLER ((uint64_t) 1 << 54)
#define TFCOND_LOWER_RADIUSHEALONDAMAGE ((uint64_t) 1 << 55)
#define TFCOND_LOWER_CRITONDAMAGE ((uint64_t) 1 << 56)
#define TFCOND_LOWER_UBERCHARGEDONTAKEDAMAGE ((uint64_t) 1 << 57)
#define TFCOND_LOWER_UBERBULLETRESIST ((uint64_t) 1 << 58)
#define TFCOND_LOWER_UBERBLASTRESIST ((uint64_t) 1 << 59)
#define TFCOND_LOWER_UBERFIRERESIST ((uint64_t) 1 << 60)
#define TFCOND_LOWER_SMALLBULLETRESIST ((uint64_t) 1 << 61)
#define TFCOND_LOWER_SMALLBLASTRESIST ((uint64_t) 1 << 62)
#define TFCOND_LOWER_SMALLFIRERESIST ((uint64_t) 1 << 63)

#define TFCOND_LOWER_STEALTHED ((uint64_t) 1 << 0)
#define TFCOND_UPPER_MEDIGUNDEBUFF ((uint64_t) 1 << 1)
#define TFCOND_UPPER_STEALTHEDUSERBUFFFADE ((uint64_t) 1 << 2)
#define TFCOND_UPPER_BULLETIMMUNE ((uint64_t) 1 << 3)
#define TFCOND_UPPER_BLASTIMMUNE ((uint64_t) 1 << 4)
#define TFCOND_UPPER_FIREIMMUNE ((uint64_t) 1 << 5)
#define TFCOND_UPPER_PREVENTDEATH ((uint64_t) 1 << 6)
#define TFCOND_UPPER_MVMBOTRADIOWAVE ((uint64_t) 1 << 7)
#define TFCOND_UPPER_HALLOWEENSPEEDBOOST ((uint64_t) 1 << 8)
#define TFCOND_UPPER_HALLOWEENQUICKHEAL ((uint64_t) 1 << 9)
#define TFCOND_UPPER_HALLOWEENGIANT ((uint64_t) 1 << 10)
#define TFCOND_UPPER_HALLOWEENTINY ((uint64_t) 1 << 11)
#define TFCOND_UPPER_HALLOWEENINHELL ((uint64_t) 1 << 12)
#define TFCOND_UPPER_HALLOWEENGHOSTMODE ((uint64_t) 1 << 13)

#include "cbase.h"
#include "cdll_int.h"
#include "icliententitylist.h"
#include "icliententity.h"
#include "convar.h"
#include "edict.h"
#include "igameresources.h"
#include "igameevents.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"
#include "ehandle.h"

#include "stdint.h"
#include "vector"

//class CBaseCombatCharacter;
class C_BaseCombatWeapon;
typedef unsigned int VPANEL;
typedef CHandle<C_BaseEntity> EHANDLE;

// client.dll
IBaseClientDLL* pClient;
IClientEntityList* pEntityList;

// engine.dll
IVEngineClient *pEngineClient;

// vgui2.dll
vgui::IPanel* pPanel;
vgui::ISurface* pSurface;
vgui::ISchemeManager* pScheme;
vgui::HFont m_font;

int m_iTextureUbercharged;
int m_iTextureCritBoosted;
int m_iTextureMarkedForDeath;

typedef struct player_s {
	int team;
    uint64_t upper;
    uint64_t lower;
} player_t;

std::vector<player_t> playerInfo;

#define Round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

class StatusSpecPlugin: public IServerPluginCallbacks, public IGameEventListener
{
public:
	StatusSpecPlugin();
	~StatusSpecPlugin();

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char     *GetPluginDescription( void );      
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );
	virtual void			OnEdictAllocated( edict_t *edict );
	virtual void			OnEdictFreed( const edict_t *edict );

	// IGameEventListener Interface
	virtual void FireGameEvent( KeyValues * event );
};

#endif
