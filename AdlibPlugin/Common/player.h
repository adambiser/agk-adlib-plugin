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

player.h - Wrapper for CPlayers to provide some extra functionality.
*/

#ifndef _PLAYER_H_
#define _PLAYER_H_
#pragma once

#include "adplug.h"
#include "utils.h"
#include "..\AGKLibraryCommands.h"

class AgkPlayer
{
public:
	AgkPlayer(CPlayer *p) : 
		player(p), 
		volume(100),
		subsong(-1),
		position(0),
		seekPosition(0)
	{}

	~AgkPlayer()
	{
		if (player)
		{
			delete player;
			player = NULL;
		}
	}

	std::string GetType() { return player->gettype(); }
	std::string GetTitle() { return player->gettitle(); }
	std::string GetAuthor() { return player->getauthor(); }
	std::string GetDescription() { return player->getdesc(); }

	float GetRefresh() { return player->getrefresh(); }
	unsigned int GetSpeed() { return player->getspeed(); }
	// Rewinds to the last seek position set.  The seek position is then cleared.
	void Rewind();
	// Plays a subsong as a sound effect.  Keeps the music looping.
	void PlaySound(unsigned int subsong);
	// In seconds.
	float GetSongLength()
	{
		// For ADL files, this will reset the OPL so that songlength(subsong) is accurate.
		player->rewind();
		return player->songlength(subsong) / 1000.0f;
	}
	bool Update();
	int GetVolume() { return volume; }
	void SetVolume(int newvolume);
	float GetPosition() { return position; }
	float GetSeekPosition() { return seekPosition; }
	
	void Seek(float seconds, int mode);

	// Return our subsong, not player->getsubsong().  player->getsubsong() can change when playing sounds with music (ADL files).
	unsigned int GetSubsong() { return subsong; } // player->getsubsong();
	unsigned int GetSubsongCount() { return player->getsubsongs(); }
	void SetSubsong(unsigned int newsubsong);

protected:
	CPlayer *player;
	int volume;
	int subsong;
	float position;
	float seekPosition;
};

#endif // _PLAYER_H_
