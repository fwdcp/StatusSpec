/*
 *  entities.cpp
 *  WebSpec project
 *  Modified for AdvSpec, used in StatusSpec
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "entities.h"

#define RetrieveAndCheckOffsetAndWarn(offset, retrieve) \
	Entities::offset = retrieve; \
	if (Entities::offset == INVALID_OFFSET) { \
		Warning("[StatusSpec] Offset %s is invalid!\n", #offset); \
		return false; \
	}

int Entities::pCTFPlayer__m_bGlowEnabled = 0;
int Entities::pCTFPlayer__m_iClass = 0;
int Entities::pCTFPlayer__m_iTeamNum = 0;
int Entities::pCTFPlayer__m_nPlayerCond = 0;
int Entities::pCTFPlayer___condition_bits = 0;
int Entities::pCTFPlayer__m_nPlayerCondEx = 0;
int Entities::pCTFPlayer__m_nPlayerCondEx2 = 0;
int Entities::pCTFPlayer__m_hActiveWeapon = 0;
int Entities::pCTFPlayer__m_hMyWeapons[MAX_WEAPONS] = {0};
int Entities::pCEconEntity__m_hOwnerEntity = 0;
int Entities::pCEconEntity__m_iItemDefinitionIndex = 0;
int Entities::pCWeaponMedigun__m_bChargeRelease = 0;
int Entities::pCWeaponMedigun__m_nChargeResistType = 0;
int Entities::pCWeaponMedigun__m_flChargeLevel = 0;

//=================================================================================
// Find the offsets for all stored NetVars
//=================================================================================
bool Entities::PrepareOffsets() {
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_bGlowEnabled, Entities::FindOffsetOfClassProp("CTFPlayer", "m_bGlowEnabled"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_iClass, Entities::FindOffsetOfClassProp("CTFPlayer", "m_iClass"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_iTeamNum, Entities::FindOffsetOfClassProp("CTFPlayer", "m_iTeamNum"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_nPlayerCond, Entities::FindOffsetOfClassProp("CTFPlayer", "m_nPlayerCond"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer___condition_bits, Entities::FindOffsetOfClassProp("CTFPlayer", "_condition_bits"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_nPlayerCondEx, Entities::FindOffsetOfClassProp("CTFPlayer", "m_nPlayerCondEx"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_nPlayerCondEx2, Entities::FindOffsetOfClassProp("CTFPlayer", "m_nPlayerCondEx2"));
	RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_hActiveWeapon, Entities::FindOffsetOfClassProp("CTFPlayer", "m_hActiveWeapon"));
	for (int i = 0; i < MAX_WEAPONS; i++) {
		RetrieveAndCheckOffsetAndWarn(pCTFPlayer__m_hMyWeapons[i], Entities::FindOffsetOfArrayEnt("CTFPlayer", "m_hMyWeapons", i));
	}
	RetrieveAndCheckOffsetAndWarn(pCEconEntity__m_hOwnerEntity, Entities::FindOffsetOfClassProp("CEconEntity", "m_hOwnerEntity"));
	RetrieveAndCheckOffsetAndWarn(pCEconEntity__m_iItemDefinitionIndex, Entities::FindOffsetOfClassProp("CEconEntity", "m_iItemDefinitionIndex"));
	RetrieveAndCheckOffsetAndWarn(pCWeaponMedigun__m_bChargeRelease, Entities::FindOffsetOfClassProp("CWeaponMedigun", "m_bChargeRelease"));
	RetrieveAndCheckOffsetAndWarn(pCWeaponMedigun__m_nChargeResistType, Entities::FindOffsetOfClassProp("CWeaponMedigun", "m_nChargeResistType"));
	RetrieveAndCheckOffsetAndWarn(pCWeaponMedigun__m_flChargeLevel, Entities::FindOffsetOfClassProp("CWeaponMedigun", "m_flChargeLevel"));
	
	return true;
}

//=================================================================================
// Loop through all server classes until className is found, then crawl through the
// class to find the property
//=================================================================================
int Entities::FindOffsetOfClassProp(const char *className, const char *propName) {
	//ServerClass *sc = serverGameDLL->GetAllServerClasses();
	ClientClass *cc = Interfaces::pClientDLL->GetAllClasses();
	while (cc) {
		if (Q_strcmp(cc->GetName(), className) == 0) {
			//SendTable *sTable = sc->m_pTable;
			RecvTable *sTable = cc->m_pRecvTable;
			if (sTable) {
				int offset = 0;
				bool found = Entities::CrawlForPropOffset(sTable, propName, offset);
				if (!found)
					offset = -1;
				return offset;
			}
		}
		cc = cc->m_pNext;
	}
	return -1;
}

int Entities::FindOffsetOfArrayEnt(const char *classname, const char *arrayName, int element) {
	ClientClass *cc = Interfaces::pClientDLL->GetAllClasses();
	while (cc) {
		if (Q_strcmp(cc->GetName(), classname) == 0) {
			RecvTable *rTable = cc->m_pRecvTable;
			if (rTable) {
				int offset = 0;
				bool found = Entities::CrawlForArrayEnt(rTable, arrayName, element, offset);
				if (!found)
					offset = -1;
				return offset;
			}
		}
		cc = cc->m_pNext;
	}

	return -1;
}

//=================================================================================
// Search through a class table, and any subtables, for a given property name
//=================================================================================
bool Entities::CrawlForPropOffset(RecvTable *sTable, const char *propName, int &offset) {
	for (int i=0; i < sTable->GetNumProps(); i++) {
		//SendProp *sProp = sTable->GetProp(i);
		RecvProp *sProp = sTable->GetProp(i);
		if (strcmp(sProp->GetName(),"000") == 0) //End of an array
			continue;

		//SendTable *sChildTable = sProp->GetDataTable();
		RecvTable *sChildTable = sProp->GetDataTable();

		//Check if it is an array, don't care for these atm so skip them
		bool isArray = false;
		if (sChildTable && sChildTable->GetNumProps() > 0) {
			if (!strcmp(sChildTable->GetProp(0)->GetName(), "000")
				|| !strcmp(sChildTable->GetProp(0)->GetName(), "lengthproxy"))
				isArray = true;
		}

		if (!isArray) {
			//If we have our property, add to the offset and start returning
			if (strcmp(sProp->GetName(), propName) == 0) {
				offset += sProp->GetOffset();
				return true;
			}
			
			//If we find a subtable, search it for the property, 
			//but keep current offset in case it isn't found here
			if (sProp->GetType() == DPT_DataTable) {
				int origOffset = offset;
				offset += sProp->GetOffset();
				bool found = Entities::CrawlForPropOffset(sChildTable, propName, offset);
				if (found) {
					return true;
				} else {
					offset = origOffset;
				}
			}
		} else {
			continue;
		}

		if (strcmp(sProp->GetName(), "000") == 0) //More array stuff from dumping function, may not be needed here
			break;
	}
	return false;
}

bool Entities::CrawlForArrayEnt(RecvTable *sTable, const char *propName, int element, int &offset) {
	for (int i=0; i < sTable->GetNumProps(); i++) {
		RecvProp *sProp = sTable->GetProp(i);
		if (strcmp(sProp->GetName(),"000") == 0) //End of an array
			continue;

		//SendTable *sChildTable = sProp->GetDataTable();
		RecvTable *sChildTable = sProp->GetDataTable();

		//Check if it is an array, don't care for these atm so skip them
		bool isArray = false;
		if (sChildTable && sChildTable->GetNumProps() > 0) {
			if (!strcmp(sChildTable->GetProp(0)->GetName(), "000")
				|| !strcmp(sChildTable->GetProp(0)->GetName(), "lengthproxy"))
				isArray = true;
		}

		if (!isArray) {
			//If we have our property, add to the offset and start returning
			if (strcmp(sProp->GetName(), propName) == 0) {
				offset += sProp->GetOffset();
				return true;
			}
			
			//If we find a subtable, search it for the property, 
			//but keep current offset in case it isn't found here
			if (sProp->GetType() == DPT_DataTable) {
				int origOffset = offset;
				offset += sProp->GetOffset();
				bool found = Entities::CrawlForArrayEnt(sChildTable, propName, element, offset);
				if (found) {
					return true;
				} else {
					offset = origOffset;
				}
			}
		} else {
			if (strcmp(sProp->GetName(), propName) != 0)
				continue;

			//We have our array
			offset += sProp->GetOffset();

			int elements = sProp->GetDataTable()->GetNumProps();
			if (element < 0 || element >= elements)
				return false;

			offset += sProp->GetDataTable()->GetProp(element)->GetOffset();
			return true;
		}

		if (strcmp(sProp->GetName(), "000") == 0) //More array stuff from dumping function, may not be needed here
			break;
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