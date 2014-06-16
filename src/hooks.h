/*
 *  hooks.h
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

#define CLIENT_DLL
#define GLOWS_ENABLE

#include "cdll_int.h"
#include "KeyValues.h"
#include "igameresources.h"
#include "vgui/vgui.h"
#include "vgui/IPanel.h"
#include "cbase.h"
#include "c_basecombatcharacter.h"
#include "glow_outline_effect.h"

#include <sourcehook/sourcehook_impl.h>
#include <sourcehook/sourcehook.h>

using namespace vgui;

class C_TFPlayer;

#if defined _WIN32
#define OFFSET_GETGLOWEFFECTCOLOR 223
#define OFFSET_UPDATEGLOWEFFECT 224
#define OFFSET_DESTROYGLOWEFFECT 225
#endif


extern SourceHook::ISourceHook *g_SHPtr;
extern int g_PLID;