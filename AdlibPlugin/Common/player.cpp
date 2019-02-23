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

player.cpp - Wrapper for CPlayers to provide some extra functionality.
*/

#include "player.h"

void AgkPlayer::rewind()
{
	player->rewind(subsong);
	// ADL starts at subsong 2, so sending subsong -1 will really select subsong 2.
	subsong = player->getsubsong();
	position = 0;
	if (seekPosition > 0)
	{
		position = seekPosition;
		player->seek((unsigned long)(seekPosition * 1000));
		// Clear the seek position for the next call.
		seekPosition = 0;
	}
}

void AgkPlayer::playsound(unsigned int subsong)
{
	player->rewind(subsong);
}

bool AgkPlayer::update()
{
	bool result = player->update();
	if (result)
	{
		position += 1.0f / player->getrefresh();
	}
	return result;
}

void AgkPlayer::setvolume(int newvolume)
{
	volume = limit(newvolume, 0, 100);
}

void AgkPlayer::seek(float seconds, int mode)
{
	switch (mode)
	{
	case 0: // absolute
		seekPosition = seconds;
		break;
	case 1: // relative
		seekPosition = position + seconds;
		break;
	default:
		return;
	}
	// If out of bounds, start at the beginning.
	if (seekPosition < 0 || seekPosition >= songlength())
	{
		seekPosition = 0;
	}
}

void AgkPlayer::setsubsong(unsigned int newsubsong)
{
	subsong = limit(newsubsong, 0, getsubsongs() - 1);
	// Clear the seek position when switching subsongs.
	seekPosition = 0;
}
