/*
 *  enums.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

// taken from SourceMod

enum TFClassType
{
	TFClass_Unknown = 0,
	TFClass_Scout,
	TFClass_Sniper,
	TFClass_Soldier,
	TFClass_DemoMan,
	TFClass_Medic,
	TFClass_Heavy,
	TFClass_Pyro,
	TFClass_Spy,
	TFClass_Engineer
};

enum TFTeam
{
	TFTeam_Unassigned = 0,
	TFTeam_Spectator = 1,
	TFTeam_Red = 2,
	TFTeam_Blue = 3	
};

enum TFCond
{
	TFCond_Slowed,
	TFCond_Zoomed,
	TFCond_Disguising,
	TFCond_Disguised,
	TFCond_Cloaked,
	TFCond_Ubercharged,
	TFCond_TeleportedGlow,
	TFCond_Taunting,
	TFCond_UberchargeFading,
	TFCond_CloakFlicker,
	TFCond_Teleporting,
	TFCond_Kritzkrieged,
	TFCond_TmpDamageBonus,
	TFCond_DeadRingered,
	TFCond_Bonked,
	TFCond_Dazed,
	TFCond_Buffed,
	TFCond_Charging,
	TFCond_DemoBuff,
	TFCond_CritCola,
	TFCond_InHealRadius,
	TFCond_Healing,
	TFCond_OnFire,
	TFCond_Overhealed,
	TFCond_Jarated,
	TFCond_Bleeding,
	TFCond_DefenseBuffed,
	TFCond_Milked,
	TFCond_MegaHeal,
	TFCond_RegenBuffed,
	TFCond_MarkedForDeath,
	TFCond_NoHealingDamageBuff,
	TFCond_SpeedBuffAlly,
	TFCond_HalloweenCritCandy,
	TFCond_CritCanteen,
	TFCond_CritDemoCharge,
	TFCond_CritHype,
	TFCond_CritOnFirstBlood,
	TFCond_CritOnWin,
	TFCond_CritOnFlagCapture,
	TFCond_CritOnKill,
	TFCond_RestrictToMelee,
	TFCond_DefenseBuffNoCritBlock,
	TFCond_Reprogrammed,
	TFCond_CritMmmph,
	TFCond_DefenseBuffMmmph,
	TFCond_FocusBuff,
	TFCond_DisguiseRemoved,
	TFCond_MarkedForDeathSilent,
	TFCond_DisguisedAsDispenser,
	TFCond_Sapped,
	TFCond_UberchargedHidden,
	TFCond_UberchargedCanteen,
	TFCond_HalloweenBombHead,
	TFCond_HalloweenThriller,
	TFCond_RadiusHealOnDamage,
	TFCond_CritOnDamage,
	TFCond_UberchargedOnTakeDamage,
	TFCond_UberBulletResist,
	TFCond_UberBlastResist,
	TFCond_UberFireResist,
	TFCond_SmallBulletResist,
	TFCond_SmallBlastResist,
	TFCond_SmallFireResist,
	TFCond_Stealthed,
	TFCond_MedigunDebuff,
	TFCond_StealthedUserBuffFade,
	TFCond_BulletImmune,
	TFCond_BlastImmune,
	TFCond_FireImmune,
	TFCond_PreventDeath,
	TFCond_MVMBotRadiowave,
	TFCond_HalloweenSpeedBoost,
	TFCond_HalloweenQuickHeal,
	TFCond_HalloweenGiant,
	TFCond_HalloweenTiny,
	TFCond_HalloweenInHell,
	TFCond_HalloweenGhostMode
};

enum TFResistType {
	TFResistType_Bullet,
	TFResistType_Explosive,
	TFResistType_Fire
};