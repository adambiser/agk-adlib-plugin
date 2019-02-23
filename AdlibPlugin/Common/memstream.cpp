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

memstream.cpp - Memblock-based I/O stream.
*/

#include "memstream.h"

binio::Int MemblockStream::peekInt(unsigned int size)
{
	unsigned int oldOffset = offset;
	Int value = readInt(size);
	offset = oldOffset;
	return value;
}

binio::Float MemblockStream::peekFloat(FType ft)
{
	unsigned int oldOffset = offset;
	Float value = readFloat(ft);
	offset = oldOffset;
	return value;
}

void MemblockStream::seek(long amount, Offset by)
{
	err &= ~Eof;
	switch (by)
	{
	case Offset::Add:
		offset += amount;
		break;
	case Offset::End:
		offset = size - amount;
		break;
	case Offset::Set:
		offset = amount;
		break;
	}
}

binio::Byte MemblockStream::getByte()
{
	if (offset >= size)
	{
		err |= Eof;
		return (Byte)EOF;
	}
	return (Byte)agk::GetMemblockByte(memblockID, offset++);
}
