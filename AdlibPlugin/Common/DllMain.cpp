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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <vector>
#include "DllMain.h"
#include "../AGKLibraryCommands.h"
#include "adplug.h"

#include "player.h"
#include "memstream.h"

/*
NOTE: Cannot use bool as an exported function return type because of AGK2 limitations.  Use int instead.
*/
#define SOUND_BUFFER_LENGTH		4096	// buffer size in samples
#define SOUND_BUFFER_COUNT		2		// two buffers
#define SOUND_HEADER_LENGTH		12
#define SOUND_CHANNELS			2		// Stereo
#define SOUND_SAMPLE_RATE		44100	// CD quality
#define SOUND_SAMPLE_BITS		16		// 16-bit.  Be sure to set GetMemblockSample and SetMemblockSample below!
/* 
Precalculate values.
*/
static const int soundBytesPerSample = (SOUND_SAMPLE_BITS / 8);
static const int soundBytesPerFrame = soundBytesPerSample * SOUND_CHANNELS;
static const int soundBytesPerBuffer = SOUND_BUFFER_LENGTH * soundBytesPerFrame;
/*
AGK Objects.
*/
// The memblock used to create the buffered sound object.
unsigned int musicMemblockID = 0;
// The sound object.
unsigned int musicSoundID = 0;
// When the music is playing, this is the sound instance.
unsigned int soundInstance = 0;
// The memblock used to create the timing sound object.
//int clockMemblockID = 0;
// The timing sound object.
unsigned int clockSoundID = 0;
// When the music is playing, this is the timing sound instance.
unsigned int clockSoundInstance = 0;
/*
Adlib Emulator
*/
static Copl *opl;
/*
Buffering information
*/
// Pointer to buffer starts within musicMemblockID.
unsigned char *bufferPos[SOUND_BUFFER_COUNT];
// The next buffer to load.
int nextBuffer = 0;
// The time at which the next buffer should load.
int lastClockLoopCount = 0;
/*
Playback settings.
*/
int musicSystemVolume = 100;
int currentLoopSetting = 0;
int loopCount = 0;
// When this is set, the song is done looping and coming to a stop.
int buffersUntilStop = 0;
int framesToRender = 0;
bool musicPaused = false;
/*
Song list.
*/
std::vector<AgkPlayer *> songs = std::vector<AgkPlayer *>();
AgkPlayer *currentSong = NULL;

// Note that this also subtracts 1 from songID since the ID is 1-based, but the lookup is 0-based!
#define ValidateSongID(songID, returnValue) \
	if (songID <= 0 || (size_t) songID > songs.size() || !songs[songID - 1])	\
	{																			\
		agk::PluginError("Invalid music ID.");									\
		return returnValue;														\
	}																			\
	songID--

// Calls the AGK Log function, but with formatting.
// TODO Move this into a global functions file?
void Log(char *format, ...)
{
	char buffer[256];
	va_list _ArgList;
	__crt_va_start(_ArgList, format);
	int result = vsprintf_s(buffer, 256, format, _ArgList);
	__crt_va_end(_ArgList);
	if (result > 0)
	{
		agk::Log(buffer);
	}
	else
	{
		agk::Log("Log message was too long or null.");
	}
}

// TODO Move this into a global functions file?
inline int limit(int value, int min, int max) {
	return (value > max) ? max : (value < min) ? min : value;
}

void ResetOPL()
{
	if (opl)
	{
		Log("Resetting Adlib emulator.");
		opl->init();
	}
}

int GetPlayVolume()
{
	//return 100;
	if (currentSong && musicSystemVolume)
	{
		//int volume = musicSystemVolume;
		int volume = (int)((musicSystemVolume / 100.0f) * currentSong->getvolume());
		//Log("GetPlayVolume: %d", volume);
		return volume;
	}
	return 0;
}

int Init(int emulator)
{
	if (opl)
	{
		agk::PluginError("OPL emulator already initialized.");
		return 0;
	}
	agk::Log("Initializing Adlib emulator.");
	switch (emulator)
	{
	case OPL_NUKED:
		opl = new CNemuopl(SOUND_SAMPLE_RATE);
		break;
	case OPL_DOSBOX:
		opl = new CWemuopl(SOUND_SAMPLE_RATE, true, true);
		break;
	case OPL_SILVERMAN:
		opl = new CKemuopl(SOUND_SAMPLE_RATE, true, true);
		break;
	case OPL_SATOH:
		opl = new CTemuopl(SOUND_SAMPLE_RATE, true, true);
		break;
	case OPL_DUAL:
		opl = new CEmuopl(SOUND_SAMPLE_RATE, true, true);
		break;
	default:
		agk::PluginError("Invalid emulator value.");
		return 0;
	}
	if (!opl)
	{
		agk::PluginError("Failed to create OPL emulator.");
		return 0;
	}
	// Set up the sound buffer memblock.
	agk::Log("Creating Adlib sound buffers.");
	musicMemblockID = agk::CreateMemblock(SOUND_HEADER_LENGTH + soundBytesPerBuffer * SOUND_BUFFER_COUNT);
	agk::SetMemblockShort(musicMemblockID, 0, SOUND_CHANNELS);
	agk::SetMemblockShort(musicMemblockID, 2, SOUND_SAMPLE_BITS);
	agk::SetMemblockInt(musicMemblockID, 4, SOUND_SAMPLE_RATE);
	agk::SetMemblockInt(musicMemblockID, 8, SOUND_BUFFER_LENGTH * SOUND_BUFFER_COUNT);
	for (int index = 0; index < SOUND_BUFFER_COUNT; index++)
	{
		bufferPos[index] = agk::GetMemblockPtr(musicMemblockID) + SOUND_HEADER_LENGTH + soundBytesPerBuffer * index;
	}
	// Initialize the sound buffers to silence.
	musicSoundID = agk::CreateSoundFromMemblock(musicMemblockID);
	// Create a silent sound that's the length of a single sound buffer.  Its loop count is used to determine when to load the next buffer.
	// Use a 8-bit mono sound for minimal memory usage.
	int clockMemblockID = agk::CreateMemblock(SOUND_HEADER_LENGTH + SOUND_BUFFER_LENGTH);
	agk::SetMemblockShort(clockMemblockID, 0, 1);
	agk::SetMemblockShort(clockMemblockID, 2, 8);
	agk::SetMemblockInt(clockMemblockID, 4, SOUND_SAMPLE_RATE);
	agk::SetMemblockInt(clockMemblockID, 8, SOUND_BUFFER_LENGTH);
	clockSoundID = agk::CreateSoundFromMemblock(clockMemblockID);
	agk::DeleteMemblock(clockMemblockID);
	return true;
}

void WriteReg(int reg, int val)
{
	if (opl)
	{
		opl->write(reg, val);
	}
}

void LoadNextBuffer()
{
	//Log("LoadNextBuffer: %d", nextBuffer);
	// Start by zeroing the buffer to silence.
	ZeroMemory(bufferPos[nextBuffer], soundBytesPerBuffer);
	if (buffersUntilStop)
	{
		//agk::Log("Reached the end of the song and not looping.");
		buffersUntilStop--;
		if (!buffersUntilStop)
		{
			StopMusic();
			return;
		}
	}
	else
	{
		// Load the buffer.
		short *waveptr = reinterpret_cast<short *>(bufferPos[nextBuffer]);
		//short *endptr = reinterpret_cast<short *>(bufferPos[nextBuffer] + soundBytesPerBuffer);
		int index = 0;
		int frames;
		do {
			if (!framesToRender)
			{
				// Before checking for frames to render, see if the song has ended and check looping.
				//if (currentSong->isEndOfSong())
				//{
				//	loopCount++;
				//	//Log("Loop check: %d, %d", loopCount, currentLoopSetting);
				//	if (currentLoopSetting == 0 || (currentLoopSetting > 1 && loopCount == currentLoopSetting))
				//	{
				//		loopCount = 0;
				//		// Load an additional silent buffer and wait for it to begin playing before stopping.
				//		// This means that 2 buffer loads need to occur before stopping.
				//		buffersUntilStop = 2;
				//		agk::Log("Ending song.  No looping set.");
				//		// Don't render anything else.
				//		break;
				//	}
				//	else
				//	{
				//		currentSong->rewind();
				//	}
				//}
				// Read song instructions.
				//currentSong->update();
				if (currentSong->update())
				{
					float refresh = currentSong->getrefresh();
					if (refresh)
					{
						framesToRender = (int)(SOUND_SAMPLE_RATE / currentSong->getrefresh());
					}
				}
				else
				{
					loopCount++;
					//Log("Loop check: %d, %d", loopCount, currentLoopSetting);
					if (currentLoopSetting == 0 || (currentLoopSetting > 1 && loopCount == currentLoopSetting))
					{
						loopCount = 0;
						// Load an additional silent buffer and wait for it to begin playing before stopping.
						// This means that 2 buffer loads need to occur before stopping.
						buffersUntilStop = 2;
						agk::Log("Ending song.  No looping set.");
						// Don't render anything else.
						break;
					}
					else
					{
						currentSong->rewind();
					}
				}
			}
			if (framesToRender)
			{
				frames = framesToRender;
				if (index + frames >= SOUND_BUFFER_LENGTH)
				{
					frames = SOUND_BUFFER_LENGTH - index;
				}
				framesToRender -= frames;
				//Log("index: %d, frames: %d, framesToRender: %d", index, frames, framesToRender);
				opl->update(waveptr, frames);
				waveptr += frames * SOUND_CHANNELS;
				index += frames;
			}
		} while (index < SOUND_BUFFER_LENGTH);
	}
	// Recreate the music sound object.
	agk::CreateSoundFromMemblock(musicSoundID, musicMemblockID);
	nextBuffer++;
	if (nextBuffer == SOUND_BUFFER_COUNT)
	{
		nextBuffer = 0;
	}
}

void Update()
{
	if (!opl || !soundInstance || !currentSong || !clockSoundInstance || !musicMemblockID)
	{
		return;
	}
	//Log("CheckBuffers");
	// Resizing the window can cause looping sounds to stop playing.
	// Make sure both the sound buffer instance and the timing sound instance are still playing.
	if (!agk::GetSoundInstancePlaying(soundInstance) || !agk::GetSoundInstancePlaying(clockSoundInstance))
	{
		Log("Restarting Adlib playback.");
		// Make sure everything is stopped.
		PauseMusic();
		ResumeMusic();
	}
	int currentClockLoopCount = agk::GetSoundInstanceLoopCount(clockSoundInstance);
	if (lastClockLoopCount != currentClockLoopCount)
	{
		lastClockLoopCount = currentClockLoopCount;
		LoadNextBuffer();
	}
}

void Shutdown()
{
	StopMusic();
	agk::Log("Shutting down Adlib emulator.");
	if (clockSoundID)
	{
		agk::DeleteSound(clockSoundID);
		clockSoundID = 0;
	}
	if (musicSoundID)
	{
		agk::DeleteSound(musicSoundID);
		musicSoundID = 0;
	}
	if (musicMemblockID)
	{
		for (int index = 0; index < SOUND_BUFFER_COUNT; index++)
		{
			bufferPos[index] = NULL;
		}
		agk::DeleteMemblock(musicMemblockID);
		musicMemblockID = 0;
	}
	for (AgkPlayer *song : songs)
	{
		delete song;
	}
	songs.clear();
	if (opl)
	{
		delete opl;
		opl = NULL;
	}
}

void DeleteMusic(int songID)
{
	ValidateSongID(songID, );
	// If deleting the current song, stop the music.
	if (currentSong == songs[songID])
	{
		StopMusic();
	}
	delete songs[songID];
	songs[songID] = NULL;
}

float GetMusicDuration(int songID)
{
	ValidateSongID(songID, 0.0f);
	return songs[songID]->songlength() / 1000.0f;
}

int GetMusicExists(int songID)
{
	return (songID > 0 && (size_t)songID <= songs.size() && songs[songID - 1]);
}

int GetMusicLoopCount()
{
	return loopCount;
}

int GetMusicPaused()
{
	return musicPaused;
}

int GetMusicPlaying()
{
	if (soundInstance)
	{
		return agk::GetSoundInstancePlaying(soundInstance);
	}
	return musicPaused;
}

float GetMusicPosition(int songID)
{
	ValidateSongID(songID, 0);
	// Subtract the frames that still need to be rendered.
	return songs[songID]->getposition() - framesToRender / (float)SOUND_SAMPLE_RATE;
}

int GetMusicRate(int songID)
{
	ValidateSongID(songID, 0);
	return songs[songID]->getspeed();
}

int GetMusicSoundinstance()
{
	return soundInstance;
}

int GetMusicSystemVolume()
{
	return musicSystemVolume;
}

int GetMusicVolume(int songID)
{
	ValidateSongID(songID, 0);
	return songs[songID]->getvolume();
}

// TODO Need a way to load files that have external dependencies, like KSM files needing inst.dat.
int LoadMusic(const char *filename, unsigned int memblockID)
{
	if (!opl)
	{
		agk::PluginError("Emulator must be initialized before music can be loaded.");
		return 0;
	}
	Log("Loading music from %s", filename);
	MemblockProvider fp;
	CPlayer	*p = NULL;
	try
	{
		//agk::Message(filename);
		fp.addFile(filename, memblockID);
		p = CAdPlug::factory(filename, opl, CAdPlug::players, fp);
		if (!p)
		{
			throw std::string("Unknown error.");
		}
		fp.removeFile(filename);
	}
	catch (std::string e)
	{
		fp.removeFile(filename);
		delete p;
		std::string msg = "Error loading music: ";
		msg.append(filename);
		msg.append("\n");
		msg.append(e);
		agk::PluginError(msg.c_str());
		return 0;
	}
	Log("file: '%s' loaded = %d", filename, p);
	//if (!p)
	//{
	//	std::string msg = "Error loading music: ";
	//	msg.append(filename);
	//	agk::PluginError(msg.c_str());
	//	return 0;
	//}
	p->rewind();
	songs.push_back(new AgkPlayer(p));
	Log("Loaded music %d from file %s.", (int)songs.size(), filename);
	return (int)songs.size();
}

int LoadMusicFromFile(const char *filename)
{
	unsigned int memblockID = agk::CreateMemblockFromFile(filename);
	int songID = LoadMusic(filename, memblockID);
	agk::DeleteMemblock(memblockID);
	return songID;
}

int LoadMusicFromMemblock(int memblockID, const char *filetype)
{
	char filename[16] = "memblock.";
	strcat_s(filename, sizeof filename, filetype);
	return LoadMusic(filename, memblockID);
}

void PauseMusic()
{
	if (musicPaused)
	{
		return;
	}
	musicPaused = true;
	agk::StopSoundInstance(soundInstance);
	agk::StopSoundInstance(clockSoundInstance);
	soundInstance = 0;
	clockSoundInstance = 0;
}

void PlayMusic(int songID, int loop)
{
	StopMusic();
	Log("PlayMusic: %d. loop = %d", songID, loop);
	ValidateSongID(songID, );
	currentSong = songs[songID];
	//currentSong->rewind();
	//Log("framesPerTic: %d", framesPerTic);
	currentLoopSetting = loop;
	for (int buffer = 0; buffer < SOUND_BUFFER_COUNT; buffer++)
	{
		LoadNextBuffer();
	}
	soundInstance = agk::PlaySound(musicSoundID, GetPlayVolume(), 1);
	clockSoundInstance = agk::PlaySound(clockSoundID, 0, 1);
	lastClockLoopCount = agk::GetSoundInstanceLoopCount(clockSoundInstance);
}

void ResumeMusic()
{
	if (!musicPaused)
	{
		return;
	}
	musicPaused = false;
	// Reload the buffers!
	nextBuffer = 0;
	for (int buffer = 0; buffer < SOUND_BUFFER_COUNT; buffer++)
	{
		LoadNextBuffer();
	}
	soundInstance = agk::PlaySound(musicSoundID, GetPlayVolume(), 1);
	clockSoundInstance = agk::PlaySound(clockSoundID, 0, 1);
	lastClockLoopCount = agk::GetSoundInstanceLoopCount(clockSoundInstance);
}

//void SeekMusic(int songID, float seconds, int mode)
//{
//	ValidateSongID(songID, );
//	songs[songID]->seek(seconds, mode);
//}

void SetMusicLoopCount(int loop)
{
	currentLoopSetting = loop;
	loopCount = 0;
}

void SetMusicSystemVolume(int volume)
{
	musicSystemVolume = limit(volume, 0, 100);
	if (soundInstance && currentSong)
	{
		agk::SetSoundInstanceVolume(soundInstance, GetPlayVolume());
	}
}

void SetMusicVolume(int songID, int volume)
{
	ValidateSongID(songID, );
	volume = limit(volume, 0, 100);
	songs[songID]->setvolume(volume);
	// Change volume if playing.
	if (soundInstance && currentSong == songs[songID])
	{
		agk::SetSoundInstanceVolume(soundInstance, GetPlayVolume());
	}
}

void StopMusic()
{
	agk::Log("Stopping music.");
	ResetOPL();
	if (currentSong)
	{
		// Rewind the wong and clear this pointer, but do not delete the song!
		currentSong->rewind();
		currentSong = NULL;
	}
	if (soundInstance)
	{
		agk::StopSoundInstance(soundInstance);
		soundInstance = 0;
	}
	if (clockSoundInstance)
	{
		agk::StopSoundInstance(clockSoundInstance);
		clockSoundInstance = 0;
	}
	lastClockLoopCount = 0;
	loopCount = 0;
	currentLoopSetting = 0;
	nextBuffer = 0;
	framesToRender = 0;
	buffersUntilStop = 0;
	musicPaused = false;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Shutdown();
		break;
	}
	return TRUE;
}
