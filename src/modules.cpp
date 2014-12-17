/*
 *  modules.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include "modules.h"

void ModuleManager::UnloadAllModules() {
	for (auto iterator = modules.begin(); iterator != modules.end(); ++iterator) {
		delete iterator->second;
	}

	modules.clear();
}