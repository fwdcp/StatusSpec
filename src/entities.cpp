/*
 *  entities.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "entities.h"

#define RETRIEVE_OFFSET(offset, retrieve) \
	if (!retrieve) { \
		Warning("[StatusSpec] Offset %s is invalid!\n", #offset); \
		return false; \
	}

int Entities::pCTFPlayer__m_iClass = 0;
int Entities::pCTFPlayer__m_iTeamNum = 0;
int Entities::pCTFPlayer__m_nPlayerCond = 0;
int Entities::pCTFPlayer___condition_bits = 0;
int Entities::pCTFPlayer__m_nPlayerCondEx = 0;
int Entities::pCTFPlayer__m_nPlayerCondEx2 = 0;
int Entities::pCTFPlayer__m_hActiveWeapon = 0;
int Entities::pCTFPlayer__m_hMyWeapons[MAX_WEAPONS] = { 0 };
int Entities::pCEconEntity__m_hOwnerEntity = 0;
int Entities::pCEconEntity__m_iItemDefinitionIndex = 0;
int Entities::pCWeaponMedigun__m_bChargeRelease = 0;
int Entities::pCWeaponMedigun__m_nChargeResistType = 0;
int Entities::pCWeaponMedigun__m_flChargeLevel = 0;
int Entities::pCTFPlayerResource__m_iHealth[MAX_PLAYERS + 1] = { 0 };
int Entities::pCTFPlayerResource__m_iMaxHealth[MAX_PLAYERS + 1] = { 0 };
int Entities::pCTFPlayerResource__m_iMaxBuffedHealth[MAX_PLAYERS + 1] = { 0 };
int Entities::pCTFPlayerResource__m_iKillstreak[MAX_PLAYERS + 1] = { 0 };
int Entities::pCWeaponMedigun__m_bHealing = 0;
int Entities::pCWeaponMedigun__m_hHealingTarget = 0;
int Entities::pCTFPlayer__m_iKillStreak = 0;
int Entities::pCTFGrenadePipebombProjectile__m_iType = 0;
int Entities::pCBaseEntity__m_iTeamNum = 0;

bool Entities::PrepareOffsets() {
	RETRIEVE_OFFSET(pCTFPlayer__m_iClass, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_iClass, 1, "m_iClass"));
	RETRIEVE_OFFSET(pCTFPlayer__m_iTeamNum, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_iTeamNum, 1, "m_iTeamNum"));
	RETRIEVE_OFFSET(pCTFPlayer__m_nPlayerCond, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_nPlayerCond, 1, "m_nPlayerCond"));
	RETRIEVE_OFFSET(pCTFPlayer___condition_bits, GetClassPropOffset("CTFPlayer", pCTFPlayer___condition_bits, 1, "_condition_bits"));
	RETRIEVE_OFFSET(pCTFPlayer__m_nPlayerCondEx, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_nPlayerCondEx, 1, "m_nPlayerCondEx"));
	RETRIEVE_OFFSET(pCTFPlayer__m_nPlayerCondEx2, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_nPlayerCondEx2, 1, "m_nPlayerCondEx2"));
	RETRIEVE_OFFSET(pCTFPlayer__m_hActiveWeapon, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_hActiveWeapon, 1, "m_hActiveWeapon"));
	for (int i = 0; i < MAX_WEAPONS; i++) {
		char *elementName = new char[4];
		sprintf(elementName, "%03i", i);
		RETRIEVE_OFFSET(pCTFPlayer__m_hMyWeapons[i], GetClassPropOffset("CTFPlayer", pCTFPlayer__m_hMyWeapons[i], 2, "m_hMyWeapons", elementName));
	}
	RETRIEVE_OFFSET(pCEconEntity__m_hOwnerEntity, GetClassPropOffset("CEconEntity", pCEconEntity__m_hOwnerEntity, 1, "m_hOwnerEntity"));
	RETRIEVE_OFFSET(pCEconEntity__m_iItemDefinitionIndex, GetClassPropOffset("CEconEntity", pCEconEntity__m_iItemDefinitionIndex, 1, "m_iItemDefinitionIndex"));
	RETRIEVE_OFFSET(pCWeaponMedigun__m_bChargeRelease, GetClassPropOffset("CWeaponMedigun", pCWeaponMedigun__m_bChargeRelease, 1, "m_bChargeRelease"));
	RETRIEVE_OFFSET(pCWeaponMedigun__m_nChargeResistType, GetClassPropOffset("CWeaponMedigun", pCWeaponMedigun__m_nChargeResistType, 1, "m_nChargeResistType"));
	RETRIEVE_OFFSET(pCWeaponMedigun__m_flChargeLevel, GetClassPropOffset("CWeaponMedigun", pCWeaponMedigun__m_flChargeLevel, 1, "m_flChargeLevel"));
	for (int i = 0; i <= MAX_PLAYERS; i++) {
		char *elementName = new char[4];
		sprintf(elementName, "%03i", i);
		RETRIEVE_OFFSET(pCTFPlayerResource__m_iHealth[i], GetClassPropOffset("CTFPlayerResource", pCTFPlayerResource__m_iHealth[i], 2, "m_iHealth", elementName));
	}
	for (int i = 0; i <= MAX_PLAYERS; i++) {
		char *elementName = new char[4];
		sprintf(elementName, "%03i", i);
		RETRIEVE_OFFSET(pCTFPlayerResource__m_iMaxHealth[i], GetClassPropOffset("CTFPlayerResource", pCTFPlayerResource__m_iMaxHealth[i], 2, "m_iMaxHealth", elementName));
	}
	for (int i = 0; i <= MAX_PLAYERS; i++) {
		char *elementName = new char[4];
		sprintf(elementName, "%03i", i);
		RETRIEVE_OFFSET(pCTFPlayerResource__m_iMaxBuffedHealth[i], GetClassPropOffset("CTFPlayerResource", pCTFPlayerResource__m_iMaxBuffedHealth[i], 2, "m_iMaxBuffedHealth", elementName));
	}
	for (int i = 0; i <= MAX_PLAYERS; i++) {
		char *elementName = new char[4];
		sprintf(elementName, "%03i", i);
		RETRIEVE_OFFSET(pCTFPlayerResource__m_iKillstreak[i], GetClassPropOffset("CTFPlayerResource", pCTFPlayerResource__m_iKillstreak[i], 2, "m_iKillstreak", elementName));
	}
	RETRIEVE_OFFSET(pCWeaponMedigun__m_bHealing, GetClassPropOffset("CWeaponMedigun", pCWeaponMedigun__m_bHealing, 1, "m_bHealing"));
	RETRIEVE_OFFSET(pCWeaponMedigun__m_hHealingTarget, GetClassPropOffset("CWeaponMedigun", pCWeaponMedigun__m_hHealingTarget, 1, "m_hHealingTarget"));
	RETRIEVE_OFFSET(pCTFPlayer__m_iKillStreak, GetClassPropOffset("CTFPlayer", pCTFPlayer__m_iKillStreak, 1, "m_iKillStreak"));
	RETRIEVE_OFFSET(pCTFGrenadePipebombProjectile__m_iType, GetClassPropOffset("CTFGrenadePipebombProjectile", pCTFGrenadePipebombProjectile__m_iType, 1, "m_iType"));
	RETRIEVE_OFFSET(pCBaseEntity__m_iTeamNum, GetClassPropOffset("CBaseEntity", pCBaseEntity__m_iTeamNum, 1, "m_iTeamNum"));

	return true;
}

bool Entities::GetClassPropOffset(const char *className, int &offset, int depth, ...) {
	ClientClass *cc = Interfaces::pClientDLL->GetAllClasses();

	while (cc) {
		if (Q_strcmp(cc->GetName(), className) == 0) {
			RecvTable *table = cc->m_pRecvTable;

			offset = 0;
			RecvProp *prop = nullptr;

			if (table) {
				va_list args;
				va_start(args, depth);

				for (int i = 0; i < depth; i++) {
					int subOffset = 0;

					if (prop && prop->GetType() == DPT_DataTable) {
						table = prop->GetDataTable();
					}

					if (!table) {
						return false;
					}

					if (GetSubProp(table, va_arg(args, const char *), prop, subOffset)) {
						offset += subOffset;
					}
					else {
						return false;
					}

					table = nullptr;
				}

				va_end(args);

				return true;
			}
		}
		cc = cc->m_pNext;
	}

	return false;
}

bool Entities::GetSubProp(RecvTable *table, const char *propName, RecvProp *&prop, int &offset) {
	for (int i = 0; i < table->GetNumProps(); i++) {
		offset = 0;

		RecvProp *currentProp = table->GetProp(i);

		if (strcmp(currentProp->GetName(), propName) == 0) {
			prop = currentProp;
			offset = currentProp->GetOffset();
			return true;
		}

		if (currentProp->GetType() == DPT_DataTable) {
			if (GetSubProp(currentProp->GetDataTable(), propName, prop, offset)) {
				offset += currentProp->GetOffset();
				return true;
			}
		}
	}

	return false;
}

inline bool CheckBaseclass(RecvTable *sTable, const char *baseclassDataTableName) {
	if (strcmp(sTable->GetName(), baseclassDataTableName) == 0) {
		return true;
	}
	
	for (int i = 0; i < sTable->GetNumProps(); i++) {
		RecvProp *sProp = sTable->GetProp(i);
		
		if (strcmp(sProp->GetName(), "baseclass") != 0) {
			continue;
		}

		RecvTable *sChildTable = sProp->GetDataTable();
		if (sChildTable) {
			return CheckBaseclass(sChildTable, baseclassDataTableName);
		}
	}
	
	return false;
}

bool Entities::CheckClassBaseclass(ClientClass *clientClass, const char *baseclassDataTableName) {
	RecvTable *sTable = clientClass->m_pRecvTable;
	
	if (sTable) {
		return CheckBaseclass(sTable, baseclassDataTableName);
	}
	
	return false;
}