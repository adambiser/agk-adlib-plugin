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

memfprovider.cpp - Memblock-based file provider.
*/

#include "memfprovider.h"

void MemblockFileProvider::clear()
{
	for (auto it = files.begin(); it != files.end();)
	{
		delete it->second;
		it = files.erase(it);
	}
}

bool MemblockFileProvider::addFile(std::string filename, unsigned int memID)
{
	auto it = files.find(filename);
	// Don't add if the file already exists in the provider.
	if (it != files.end())
	{
		return false;
	}
	files.insert({ filename, new MemblockStream(memID) });
	return true;
}

void MemblockFileProvider::removeFile(std::string filename)
{
	auto it = files.find(filename);
	if (it != files.end())
	{
		delete it->second;
		files.erase(it);
	}
}

binistream *MemblockFileProvider::open(std::string filename) const
{
	auto it = files.find(filename);
	if (it != files.end())
	{
		// Always start at position 0.
		it->second->seek(0);
		return it->second;
	}
	// Return null when looking for a file that doesn't exist.
	return NULL;
}
