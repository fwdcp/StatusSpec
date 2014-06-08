/*
 *  paint.h
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include <map>
#include <string>

#include "Color.h"
#include "vgui/ISurface.h"

#include "ifaces.h"

class Paint {
public:
	static void Paint::InitializeTexture(std::string texture);
	static void DrawTexture(std::string texture, int x, int y, int width, int height, Color filter = Color(255, 255, 255, 255));
private:
	static int GetTextureID(std::string texture);
	static std::map<std::string, int> textureIDs;
};