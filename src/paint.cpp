/*
 *  paint.cpp
 *  StatusSpec project
 *  
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "paint.h"

std::map<std::string, int> Paint::textureIDs;

void Paint::InitializeTexture(std::string texture) {
	if (textureIDs.find(texture) != textureIDs.end()) {
		return;
	}

	int textureID = g_pVGuiSurface->DrawGetTextureId(texture.c_str());
	
	if (textureID == -1) {
		textureID = g_pVGuiSurface->CreateNewTextureID();
		g_pVGuiSurface->DrawSetTextureFile(textureID, texture.c_str(), 0, false);
	}

	textureIDs[texture] = textureID;
}

void Paint::DrawTexture(std::string texture, int x, int y, int width, int height, Color filter) {
	g_pVGuiSurface->DrawSetTexture(GetTextureID(texture));
	g_pVGuiSurface->DrawSetColor(filter.r(), filter.g(), filter.b(), filter.a());
	g_pVGuiSurface->DrawTexturedRect(x, y, x + width, y + height);
}

int Paint::GetTextureID(std::string texture) {
	if (textureIDs.find(texture) == textureIDs.end()) {
		InitializeTexture(texture);
	}

	return textureIDs[texture];
}