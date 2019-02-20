#define WIN32_LEAN_AND_MEAN

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
		// Don't use PluginError or it crashes.
		//agk::PluginError("Tried to read past end of memblock.");
		throw std::string("Tried to read past end of memblock.");
	}
	return (Byte)agk::GetMemblockByte(memblockID, offset++);
}
