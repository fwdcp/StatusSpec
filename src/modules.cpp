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

#include "modules.h"

#include "common.h"

void ModuleManager::UnloadAllModules() {
	for (auto iterator : modules) {
		PRINT_TAG();
		ConColorMsg(Color(0, 255, 0, 255), "Module %s unloaded!\n", iterator.first.c_str());

		delete iterator.second;
	}

	modules.clear();
}