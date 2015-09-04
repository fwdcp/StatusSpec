/*
 *  filesystemtools.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "../modules.h"

class CCommand;
class ConCommand;

class FilesystemTools : public Module {
public:
	FilesystemTools();

	static bool CheckDependencies();
private:
	ConCommand *searchpath_add;
	ConCommand *searchpath_remove;
	void AddSearchPath(const CCommand &command);
	void RemoveSearchPath(const CCommand &command);
};
