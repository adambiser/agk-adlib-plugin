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

memstream.h - Memblock-based I/O stream.
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
