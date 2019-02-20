#include "player.h"

float AgkPlayer::getrefresh()
{
	return player->getrefresh();
}

unsigned int AgkPlayer::getspeed()
{
	return player->getspeed();
}

void AgkPlayer::rewind()
{
	player->rewind();
	position = 0;
}

unsigned long AgkPlayer::songlength()
{
	return player->songlength();
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
	if (newvolume < 0)
	{
		volume = 0;
	}
	else if (newvolume > 100)
	{
		volume = 100;
	}
	else
	{
		volume = newvolume;
	}
}

float AgkPlayer::getposition()
{
	return position;
}
