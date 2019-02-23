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
		subsong(-1), //p->getsubsong()),
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

	std::string gettype() { return player->gettype(); }
	std::string gettitle() { return player->gettitle(); }
	std::string getauthor() { return player->getauthor(); }
	std::string getdesc() { return player->getdesc(); }

	float getrefresh() { return player->getrefresh(); }
	unsigned int getspeed() { return player->getspeed(); }
	// Rewinds to the last seek position set.  The seek position is then cleared.
	void rewind();
	// Plays a subsong as a sound effect.  Keeps the music looping.
	//void playSoundEffect(unsigned int subsong);
	// In seconds.
	float songlength()
	{
		//std::string msg;
		//if (subsong == 4)
		//{
		//	msg.append("songlength ").append(std::to_string(subsong));
		//	agk::Log(msg.c_str());
		//}
		//// get song length
		//float slength = 0.0f;
		player->rewind();
		//player->rewind(subsong);
		//while (player->update() && slength < 600000)	// song length limit: 10 minutes
		//{
		//	slength += 1000.0f / player->getrefresh();
		//	if (subsong == 4)
		//	{
		//		msg.clear();
		//		msg.append("slength = ").append(std::to_string(slength));
		//		agk::Log(msg.c_str());
		//	}
		//}
		//player->rewind(subsong);
		//return slength / 1000.0f;
		return player->songlength(subsong) / 1000.0f;
	}
	bool update();
	int getvolume() { return volume; }
	void setvolume(int newvolume);
	float getposition() { return position; }
	float getseekposition() { return seekPosition; }
	
	void seek(float seconds, int mode);

	unsigned int getsubsong() { return player->getsubsong(); }
	unsigned int getsubsongs() { return player->getsubsongs(); }
	void setsubsong(unsigned int newsubsong);

protected:
	CPlayer *player;
	int volume;
	int subsong;
	float position;
	float seekPosition;
};
#endif // _PLAYER_H_
