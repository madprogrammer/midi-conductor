/* Copyright (c) Anoufrienko Sergey aka madpr
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>

#define CHECK(condition, msg) \
	if(!(condition)) throw std::logic_error(msg);
