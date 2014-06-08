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

#include "cdll_int.h"
#include "KeyValues.h"
#include "vgui/vgui.h"
#include "vgui/IPanel.h"
#include "igameresources.h"

#include <sourcehook/sourcehook_impl.h>
#include <sourcehook/sourcehook.h>

using namespace vgui;

extern SourceHook::ISourceHook *g_SHPtr;
extern int g_PLID;