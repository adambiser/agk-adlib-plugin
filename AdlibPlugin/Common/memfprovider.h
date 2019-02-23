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

memfprovider.h - Memblock-based file provider.
*/

#ifndef _MEMFPROVIDER_H_
#define _MEMFPROVIDER_H_
#pragma once

#include "../AGKLibraryCommands.h"
#include <map>
#include "adplug.h"
#include "memstream.h"

class MemblockFileProvider : public CFileProvider
{
public:
	MemblockFileProvider() {}
	~MemblockFileProvider()
	{
		clear();
	}
	void clear();
	bool addFile(std::string filename, unsigned int memID);
	void removeFile(std::string filename);
	binistream *open(std::string filename) const;
	void close(binistream *f) const {}
private:
	std::map<std::string, MemblockStream*> files;
};

#endif // _MEMFPROVIDER_H_
