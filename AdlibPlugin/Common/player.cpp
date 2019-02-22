#include "player.h"

void AgkPlayer::rewind()
{
	player->rewind(subsong);
	// ADL starts at subsong 2, so sending subsong 0 will really select subsong 2.
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
