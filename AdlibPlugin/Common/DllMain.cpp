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

DllMain.cpp - Main plugin functionality exports.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <vector>
#include "DllMain.h"
#include "../AGKLibraryCommands.h"
#include "adplug.h"

#include "player.h"
#include "memfprovider.h"
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
// When this is set, the song is done looping and coming to a stop.
int buffersUntilStop = 0;
int framesToRender = 0;
/*
Playback settings.
*/
int currentLoopSetting = 0;
int loopCount = 0;
int musicSystemVolume = 100;
bool musicPaused = false;
/*
Song list.
*/
MemblockFileProvider fileProvider;
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
	if (currentSong && musicSystemVolume)
	{
		return (int)((musicSystemVolume / 100.0f) * currentSong->GetVolume());
	}
	return 0;
}

int Init(int emulator)
{
	if (opl)
	{
		agk::PluginError("Adlib emulator already initialized.");
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
		agk::PluginError("Invalid emulator type value.");
		return 0;
	}
	if (!opl)
	{
		agk::PluginError("Failed to create Adlib emulator.");
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
	Log("%d - LoadNextBuffer: %d", agk::GetMilliseconds(), nextBuffer);
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
		bool eof = false;
		do {
			if (!framesToRender)
			{
				// Read song instructions.
				//agk::Log("currentSong->update");
				eof = !currentSong->Update();
				float refresh = currentSong->GetRefresh();
				if (refresh)
				{
					//agk::Log("has framesToRender");
					framesToRender = (int)(SOUND_SAMPLE_RATE / currentSong->GetRefresh());
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
			// Handle eof after processing frames.  Rewinding resets the emulator.
			if (eof)
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
					//agk::Log("currentSong->rewind");
					currentSong->Rewind();
					eof = false;
				}
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
	DeleteAllExternalData();
	DeleteAllMusic();
	if (opl)
	{
		delete opl;
		opl = NULL;
	}
}

void DeleteAllExternalData()
{
	fileProvider.clear();
}

void DeleteAllMusic()
{
	for (AgkPlayer *song : songs)
	{
		delete song;
	}
	songs.clear();
}

void DeleteExternalData(const char *entryname)
{
	fileProvider.removeFile(entryname);
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

static char *CreateString(std::string text)
{
	unsigned int size = text.size() + 1;
	char *str = agk::CreateString(size);
	strcpy_s(str, size, text.c_str());
	return str;
}

char *GetMusicAuthor(int songID)
{
	ValidateSongID(songID, NULL);
	return CreateString(songs[songID]->GetAuthor());
}

char *GetMusicDescription(int songID)
{
	ValidateSongID(songID, NULL);
	return CreateString(songs[songID]->GetDescription());
}

float GetMusicDuration(int songID)
{
	ValidateSongID(songID, 0.0f);
	return songs[songID]->GetSongLength();
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
	return songs[songID]->GetPosition() - framesToRender / (float)SOUND_SAMPLE_RATE;
}

int GetMusicRate(int songID)
{
	ValidateSongID(songID, 0);
	return songs[songID]->GetSpeed();
}

int GetMusicSoundInstance()
{
	return soundInstance;
}

int GetMusicSubsong(int songID)
{
	ValidateSongID(songID, 0);
	return songs[songID]->GetSubsong();
}

int GetMusicSubsongCount(int songID)
{
	ValidateSongID(songID, 0);
	return songs[songID]->GetSubsongCount();
}

int GetMusicSystemVolume()
{
	return musicSystemVolume;
}

char *GetMusicTitle(int songID)
{
	ValidateSongID(songID, NULL);
	return CreateString(songs[songID]->GetTitle());
}

char *GetMusicType(int songID)
{
	ValidateSongID(songID, NULL);
	return CreateString(songs[songID]->GetType());
}

int GetMusicVolume(int songID)
{
	ValidateSongID(songID, 0);
	return songs[songID]->GetVolume();
}

void LoadExternalDataFromFile(const char *filename)
{
	LoadExternalDataFromFileEx(filename, filename);
}

void LoadExternalDataFromFileEx(const char *filename, const char *entryname)
{
	unsigned int memblockID = agk::CreateMemblockFromFile(filename);
	LoadExternalDataFromMemblock(memblockID, entryname);
	agk::DeleteMemblock(memblockID);
}

void LoadExternalDataFromMemblock(int memblockID, const char *entryname)
{
	unsigned int size = agk::GetMemblockSize(memblockID);
	unsigned int memID = agk::CreateMemblock(size);
	agk::CopyMemblock(memblockID, memID, 0, 0, size);
	if (!fileProvider.addFile(entryname, memID))
	{
		std::string msg = "An external data entry already exists for '";
		msg.append(entryname);
		msg.append("'");
		agk::PluginError(msg.c_str());
	}
}

int LoadMusic(const char *filename, unsigned int memblockID)
{
	if (!opl)
	{
		agk::PluginError("Emulator must be initialized before music can be loaded.");
		return 0;
	}
	Log("Loading music from %s", filename);
	CPlayer	*p = NULL;
	std::string error;
	//agk::Message(filename);
	if (fileProvider.addFile(filename, memblockID))
	{
		try
		{
			p = CAdPlug::factory(filename, opl, CAdPlug::players, fileProvider);
		}
		catch (int e)
		{
			error.append("Error #").append(std::to_string(e));
		}
		catch (std::string e)
		{
			error.append(e);
		}
		catch (...)
		{
			error.append("Unknown error.");
		}
		fileProvider.removeFile(filename);
	}
	else
	{
		error.append("A data entry already exists for this file name.");
	}
	if (!p && error.size() == 0)
	{
		error.append("Failed to determine music file type.");
	}
	if (error.size() > 0)
	{
		delete p;
		std::string msg = "Error loading music: ";
		msg.append(filename);
		msg.append("\n");
		msg.append(error);
		agk::PluginError(msg.c_str());
		return 0;
	}
	//Log("file: '%s' loaded = %d", filename, p);
	//p->rewind(3);
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
}

void PlayMusic(int songID, int loop)
{
	StopMusic();
	ResetOPL();
	Log("PlayMusic: %d. loop = %d", songID, loop);
	ValidateSongID(songID, );
	currentSong = songs[songID];
	// Rewind takes the song to the current seek position, which might be 0 anyway.
	currentSong->Rewind();
	//Log("framesPerTic: %d", framesPerTic);
	currentLoopSetting = loop;
	agk::Log("Loading buffers");
	for (int buffer = 0; buffer < SOUND_BUFFER_COUNT; buffer++)
	{
		LoadNextBuffer();
	}
	agk::Log("Play sounds.");
	soundInstance = agk::PlaySound(musicSoundID, GetPlayVolume(), 1);
	clockSoundInstance = agk::PlaySound(clockSoundID, 0, 1);
	lastClockLoopCount = agk::GetSoundInstanceLoopCount(clockSoundInstance);
}

void PlaySound(int songID, int subsong)
{
	Log("PlaySound: %d / %d", songID, subsong);
	ValidateSongID(songID, );
	// If not currently playing the given song, switch to it.
	if (currentSong == songs[songID])
	{
		currentSong->PlaySound(subsong);
	}
	else
	{
		// No looping for sound effects.
		SetMusicSubsong(songID, subsong);
		PlayMusic(songID, 0);
		return;
	}
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

void SeekMusic(int songID, float seconds, int mode)
{
	ValidateSongID(songID, );
	songs[songID]->Seek(seconds, mode);
	if (currentSong == songs[songID])
	{
		currentSong->Rewind();
	}
}

void SetMusicLoopCount(int loop)
{
	currentLoopSetting = loop;
	loopCount = 0;
}

void SetMusicSubsong(int songID, int subsong)
{
	ValidateSongID(songID, );
	//unsigned int oldSubsong = songs[songID]->getsubsong();
	songs[songID]->SetSubsong(subsong);
	// If currently playing, immediately start playing the new subsong.
	if (currentSong == songs[songID])
	{
		// Remember: ValidateSongID makes the songID 0-based.  Go back to 1-based here.
		//PlayMusic(songID + 1, currentLoopSetting);
		// Just rewind for the new subsong.
		// ADL files can play multiple subsongs simultaneously.  Some subsongs are songs, some are sound effects.
		songs[songID]->Rewind();
	}
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
	songs[songID]->SetVolume(volume);
	// Change volume if playing.
	if (soundInstance && currentSong == songs[songID])
	{
		agk::SetSoundInstanceVolume(soundInstance, GetPlayVolume());
	}
}

void StopMusic()
{
	agk::Log("Stopping music.");
	if (currentSong)
	{
		// Rewind the wong and clear this pointer, but do not delete the song!
		currentSong->Rewind();
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
