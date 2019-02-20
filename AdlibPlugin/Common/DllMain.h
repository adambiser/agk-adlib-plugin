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

#ifndef _DLLMAIN_H_
#define _DLLMAIN_H_
#pragma once

#include "..\AGKLibraryCommands.h"

/*
Emulator Types
*/
#define OPL_NUKED		1
#define OPL_DOSBOX		2
#define OPL_SILVERMAN	3
#define OPL_SATOH		4
#define OPL_DUAL		5

//#define SEEK_ABSOLUTE	0
//#define SEEK_RELATIVE	1

/*
@desc
Initializes the OPL2 soft synth.  This method should be called before attempting to do anything else with this plugin.

All emulators play as 16-bit stereo 44100 Hz sounds.
@param emulator The emulator to use.
1 = Nuked OPL3 emulator
2 = DOSBox emulator.
3 = Ken Silverman's emulator.
4 = Tatsuyuki Satoh's emulator.
5 = Dual OPL.
@return 1 on success; otherwise 0.
*/
extern "C" DLL_EXPORT int Init(int emulator);
/*
#desc Must be called each frame to ensure that the sound buffers are being loaded.
*/
extern "C" DLL_EXPORT void Update();
/*
@desc Destroys the OPL2 soft synth and removes all songs.
This is done when the plugin unloads and does not need to be explicitly called.
*/
extern "C" DLL_EXPORT void Shutdown();
/*
@desc Deletes the song information and invalidates the ID.
@param songID The song ID to delete.
*/
extern "C" DLL_EXPORT void DeleteMusic(int songID);
/*
@desc Returns the duration of the song in seconds.
This should not be called on a song while it is playing or the song will start again from the beginning.
@param songID The song ID.
@return Duration in seconds.
*/
extern "C" DLL_EXPORT float GetMusicDuration(int songID);
/*
@desc Checks the existence for the given song ID.
@return 1 if a song exists at the specified ID; otherwise 0.
*/
extern "C" DLL_EXPORT int GetMusicExists(int songID);
/*
@desc Returns the number of times the current song has looped.
@return The loop count.
*/
extern "C" DLL_EXPORT int GetMusicLoopCount();
/*
@desc Returns whether music playback is paused.
@return 1 if paused; otherwise 0.
*/
extern "C" DLL_EXPORT int GetMusicPaused();
/*
@desc Returns whether the OPL synth is playing.
@return 1 if playing; otherwise 0.
*/
extern "C" DLL_EXPORT int GetMusicPlaying();
/*
@desc Returns the current position in the music file in seconds, between 0 for the beginning of the song and GetMusicDuration for the end of the song.
This position is only an approximation.
@param songID The song ID.
@return Position in seconds.
*/
extern "C" DLL_EXPORT float GetMusicPosition(int songID);
/*
@desc Returns the song's playback rate.
@param songID The song ID.
@return The song's rate.
*/
extern "C" DLL_EXPORT int GetMusicRate(int songID);
/*
@desc Returns the sound instance used for music playback.
There should be no need to change the sound instance directly.  Use the available plugin methods instead.
@return The sound instance ID.
*/
extern "C" DLL_EXPORT int GetMusicSoundinstance();
/*
@desc Returns the volume of the OPL2 synth.
@return An integer from 0 to 100.
*/
extern "C" DLL_EXPORT int GetMusicSystemVolume();
/*
@desc Returns the volume of the given song ID.
@param songID The song ID.
@return An integer from 0 to 100.
*/
extern "C" DLL_EXPORT int GetMusicVolume(int songID);
/*
@desc Loads song information from the given file name.
@param filename The name of the file to load.
@return The music ID of the loaded song or 0 if an error occurs.
*/
extern "C" DLL_EXPORT int LoadMusicFromFile(const char *filename);
/*
@desc Loads song information from the given memblock.
@param memblockID The memblock containing song information.
@param filetype	The file extension without the leading period indicating the type of data in the memblock.
@return The music ID of the loaded song or 0 if an error occurs.
*/
extern "C" DLL_EXPORT int LoadMusicFromMemblock(int memblockID, const char *filetype);
/*
@desc Pauses music playback.
GetMusicPlaying will continue to return 1.
*/
extern "C" DLL_EXPORT void PauseMusic();
/*
@desc Play the song ID.
@param songID	The song ID to play.
@param loop		The number of times to loop, or 1 to loop forever.
*/
extern "C" DLL_EXPORT void PlayMusic(int songID, int loop);
/*
@desc Resumes music playback if it was paused.
*/
extern "C" DLL_EXPORT void ResumeMusic();
/*
@desc Seeks to a given time value within the song.
If the song is currently playing, it will continue playing from the new position.
If the song is not currently playing, it will take effect after the next call to PlayMusic.
@param songID	The song ID.
@param seconds	The time in seconds to seek to.
@param mode		0 for absolute seeking, 1 for relative seeking.
*/
//extern "C" DLL_EXPORT void SeekMusic(int songID, float seconds, int mode);
/*
@desc Changes the number of times the current song will loop.
This resets the loop count to 0.
@param loop		The number of times to loop, or 1 to loop forever.
*/
extern "C" DLL_EXPORT void SetMusicLoopCount(int loop);
/*
#desc Sets the colume for the OPL2 synth.
@param volume A number between 0 and 100 inclusive.
*/
extern "C" DLL_EXPORT void SetMusicSystemVolume(int volume);
/*
@desc Sets the volume for a song.  By default, songs start with a volume of 100.
@param songID The ID of the song to change.
@param volume A number between 0 and 100 inclusive.
*/
extern "C" DLL_EXPORT void SetMusicVolume(int songID, int volume);
/*
@desc Stops music playback.
*/
extern "C" DLL_EXPORT void StopMusic();

#endif // _DLLMAIN_H_
