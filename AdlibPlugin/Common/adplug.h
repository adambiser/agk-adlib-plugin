/*
Copyright (c) 2019 Adam Biser <adambiser@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _ADPLUG_H_
#define _ADPLUG_H_
#pragma once

#include "../AdPlug/src/adplug.h"
#include "../AdPlug/src/nemuopl.h" // Nuked OPL3 emulator
#include "../AdPlug/src/wemuopl.h" // DOSBox emulator.
#include "../AdPlug/src/kemuopl.h" // Ken Silverman's emulator.
#include "../AdPlug/src/temuopl.h" // Tatsuyuki Satoh's emulator.
#include "../AdPlug/src/emuopl.h"  // Dual OPL.

#if defined(_WINDOWS)
#if defined(_WIN64)
#pragma comment(lib, "adplug64.lib")
#else
#pragma comment(lib, "../AdPlug/Win32/adplug.lib")
#endif
#endif

#endif // _ADPLUG_H_
