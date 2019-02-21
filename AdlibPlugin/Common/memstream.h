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

#ifndef _MEMSTREAM_H_
#define _MEMSTREAM_H_
#pragma once

#include "../AGKLibraryCommands.h"
#include "adplug.h"

class MemblockStream : public binistream
{
public:
	MemblockStream(unsigned int memID) :
		memblockID(memID),
		offset(0)
	{
		size = agk::GetMemblockSize(memblockID);
		setFlag(binio::FloatIEEE);
	}
	~MemblockStream()
	{
		agk::DeleteMemblock(memblockID);
	}
	binio::Int peekInt(unsigned int size);
	binio::Float peekFloat(FType ft);
	void ignore(unsigned long amount = 1) { offset += amount; }
	void seek(long amount, Offset by = Set);
	long pos() { return offset; }
protected:
	binio::Byte getByte();
private:
	unsigned int memblockID;
	unsigned int size;
	unsigned int offset;
};

#endif // _MEMSTREAM_H_
