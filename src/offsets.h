/*
 *  offsets.h
 *  WebSpec project
 *  Modified for AdvSpec, used in StatusSpec
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "ifaces.h"

#include "cdll_client_int.h"
#include "client_class.h"

#define MAX_WEAPONS 48

class WSOffsets {
public:
	static int pCTFPlayer__m_iTeamNum;
	static int pCTFPlayer__m_nPlayerCond;
	static int pCTFPlayer___condition_bits;
	static int pCTFPlayer__m_nPlayerCondEx;
	static int pCTFPlayer__m_nPlayerCondEx2;
	static int pCEconEntity__m_hOwnerEntity;
	static int pCEconEntity__m_iItemDefinitionIndex;

	static void PrepareOffsets();
	static int FindOffsetOfClassProp(const char *className, const char *propName);
	static int FindOffsetOfArrayEnt(const char *classname, const char *arrayName, int element);

private:
	static bool CrawlForPropOffset(RecvTable *sTable, const char *propName, int &offset);
	static bool CrawlForArrayEnt(RecvTable *sTable, const char *propName, int element, int &offset);
};

#define MakePtr(cast, ptr, addValue) (cast)((unsigned long) (ptr) + (unsigned long) (addValue))
