/*
 *  filesystemtools.cpp
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "filesystemtools.h"

#include "convar.h"
#include "filesystem.h"

FilesystemTools::FilesystemTools() {
	searchpath_add = new ConCommand("statusspec_filesystemtools_searchpath_add", [](const CCommand &command) { g_ModuleManager->GetModule<FilesystemTools>()->AddSearchPath(command); }, "add a filesystem search path", FCVAR_NONE);
	searchpath_remove = new ConCommand("statusspec_filesystemtools_searchpath_remove", [](const CCommand &command) { g_ModuleManager->GetModule<FilesystemTools>()->RemoveSearchPath(command); }, "remove a filesystem search path", FCVAR_NONE);
}

bool FilesystemTools::CheckDependencies() {
	bool ready = true;

	if (!g_pFullFileSystem) {
		PRINT_TAG();
		Warning("Required interface IFileSystem for module %s not available!\n", g_ModuleManager->GetModuleName<FilesystemTools>().c_str());

		ready = false;
	}

	return ready;
}

void FilesystemTools::AddSearchPath(const CCommand &command) {
	if (command.ArgC() >= 3) {
		SearchPathAdd_t location = PATH_ADD_TO_TAIL;

		if (command.ArgC() >= 4) {
			if (V_stricmp(command.Arg(3), "head") == 0) {
				location = PATH_ADD_TO_HEAD;
			}
			else if (V_stricmp(command.Arg(3), "tail") == 0) {
				location = PATH_ADD_TO_TAIL;
			}
		}

		g_pFullFileSystem->AddSearchPath(command.Arg(1), command.Arg(2), location);
	}
	else {
		Warning("Usage: statusspec_filesystemtools_searchpath_add <path> <id> [location]\n");
	}
}

void FilesystemTools::RemoveSearchPath(const CCommand &command) {
	if (command.ArgC() >= 3) {
		g_pFullFileSystem->RemoveSearchPath(command.Arg(1), command.Arg(2));
	}
	else {
		Warning("Usage: statusspec_filesystemtools_searchpath_remove <path> <id>\n");
	}
}