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

#ifndef _UTILS_H_
#define _UTILS_H_
#pragma once

inline int limit(int value, int min, int max) {
	return (value > max) ? max : (value < min) ? min : value;
}

#endif // _UTILS_H_
