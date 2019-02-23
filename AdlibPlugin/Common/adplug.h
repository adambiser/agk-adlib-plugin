/*
AdlibPlugin - AppGameKit Plugin to play OPL2/3 files using AdPlug.
Copyright (c) 2019 Adam Biser <adambiser@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

adplug.h - Groups all AdPlugin includes and library usage together.
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
#if defined(_DEBUG)
#if defined(_WIN64)
#pragma comment(lib, "../AdPlug/Win64/adplug64-debug.lib")
#else
#pragma comment(lib, "../AdPlug/Win32/adplug-debug.lib")
#endif
#else
#if defined(_WIN64)
#pragma comment(lib, "../AdPlug/Win64/adplug64.lib")
#else
#pragma comment(lib, "../AdPlug/Win32/adplug.lib")
#endif
#endif
#endif

#endif // _ADPLUG_H_
