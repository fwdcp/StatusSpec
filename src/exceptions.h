/**
 *  exceptions.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014 thesupremecommander
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "stdafx.h"

#include <sstream>
#include <string>

#ifdef _WIN32
#define noexcept _NOEXCEPT
#endif

class bad_pointer : public std::exception {
public:
	bad_pointer(const char *type) noexcept;
	virtual const char *what() const noexcept;
private:
	const char *pointerType;
};

inline bad_pointer::bad_pointer(const char *type) noexcept{
	pointerType = type;
}

inline const char *bad_pointer::what() const noexcept{
	std::string s;
	std::stringstream ss;

	ss << "Invalid pointer to " << pointerType << "!\n";
	ss >> s;

	return s.c_str();
}