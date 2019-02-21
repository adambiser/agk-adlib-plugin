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
	// Don't add if the file already exists in the system.
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
