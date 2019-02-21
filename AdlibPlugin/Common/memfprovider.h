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
