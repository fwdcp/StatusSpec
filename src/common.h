/**
 *  common.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include "Color.h"

typedef struct ColorConCommand_s {
	Color color;
	ConCommand *command;
} ColorConCommand_t;