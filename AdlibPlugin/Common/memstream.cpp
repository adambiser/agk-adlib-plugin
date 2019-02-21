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
