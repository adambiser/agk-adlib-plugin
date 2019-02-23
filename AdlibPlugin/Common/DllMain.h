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

DllMain.h - Main plugin functionality exports.
*/

#ifndef _DLLMAIN_H_
#define _DLLMAIN_H_
#pragma once

#include "..\AGKLibraryCommands.h"
#include "utils.h"

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

/* @page General Commands */
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
@desc Must be called each frame to ensure that the sound buffers are being loaded.
*/
extern "C" DLL_EXPORT void Update();
/*
@desc Destroys the OPL2 soft synth and removes all songs.
This is done when the plugin unloads and does not need to be explicitly called.
*/
extern "C" DLL_EXPORT void Shutdown();
/*
@desc Deletes all external data entries.
*/
extern "C" DLL_EXPORT void DeleteAllExternalData();
/*
@desc Deletes all music managed by the plugin.
*/
extern "C" DLL_EXPORT void DeleteAllMusic();
/*
@desc Deletes an external data entry.
@param entryname	The name of the entry to remove.
*/
extern "C" DLL_EXPORT void DeleteExternalData(const char *entryname);
/*
@desc Deletes the song information and invalidates the ID.
@param songID The song ID to delete.
*/
extern "C" DLL_EXPORT void DeleteMusic(int songID);
/*
@desc Returns a song's author.
@param songID The ID of the song.
@return A string.
*/
extern "C" DLL_EXPORT char *GetMusicAuthor(int songID);
/*
@desc Returns the song's description.
@param songID The ID of the song.
@return A string.
*/
extern "C" DLL_EXPORT char *GetMusicDescription(int songID);
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
extern "C" DLL_EXPORT int GetMusicSoundInstance();
/*
@desc Returns the current subsong for a song.
@param songID The ID of the song.
@return A 0-based subsong index.
*/
extern "C" DLL_EXPORT int GetMusicSubsong(int songID);
/*
@desc Returns the number of subsongs in a song.
@param songID The ID of the song.
@return The number of subsongs in a song.
*/
extern "C" DLL_EXPORT int GetMusicSubsongCount(int songID);
/*
@desc Returns the volume of the emulator.
@return An integer from 0 to 100.
*/
extern "C" DLL_EXPORT int GetMusicSystemVolume();
/*
@desc Returns a song's title.
@param songID The ID of the song.
@return A string.
*/
extern "C" DLL_EXPORT char *GetMusicTitle(int songID);
/*
@desc Returns a song's file type.
@param songID The ID of the song.
@return A string.
*/
extern "C" DLL_EXPORT char *GetMusicType(int songID);
/*
@desc Returns the volume of the given song ID.
@param songID The song ID.
@return An integer from 0 to 100.
*/
extern "C" DLL_EXPORT int GetMusicVolume(int songID);
/*
@desc Load external data required for some music file formats from a file.
Will raise an error if an entry with this name already exists.

ie: The standard.bnk for ROL files.
@param filename	The file to load.  This is used as the entry name internally.
*/
extern "C" DLL_EXPORT void LoadExternalDataFromFile(const char *filename);
/*
@desc Loads external data required for some music file formats from a file.
Will raise an error if an entry with this name already exists.

ie: The standard.bnk for ROL files.

This is the same as LoadExternalDataFromFile, but allows the internal entry name to be set.
@param filename		The file to load.
@param entryname	The name of the entry.  Each music file format has its own naming convention.
*/
extern "C" DLL_EXPORT void LoadExternalDataFromFileEx(const char *filename, const char *entryname);
/*
@desc Loads external data required for some music file formats from a memblock.
Will raise an error if an entry with this name already exists.

An internal copy of the memblock is made.
The memblock in the calling code can be deleted after calling this method.

ie: The standard.bnk for ROL files.
@param memblockID	The memblock to load.
@param entryname	The name of the entry.  Each music file format has its own naming convention.
*/
extern "C" DLL_EXPORT void LoadExternalDataFromMemblock(int memblockID, const char *entryname);
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
@desc Plays a subsong as a sound effect.
Some file formats, such as ADL files, contain many subsongs, some that are music and some that are sound effects.
Sound effects can be played simultaneously with the songs.

If the given song is not currently playing, the plugin will change to the given song.
The subsong will be played one time.
@param songID	The song ID containing the subsong.
@param subsong	The subsong to play.
*/
extern "C" DLL_EXPORT void PlaySound(int songID, int subsong);
/*
@desc Resumes music playback if it was paused.
*/
extern "C" DLL_EXPORT void ResumeMusic();
/*
@desc Seeks to a given time value within the song.
If the song is currently playing, it will continue playing from the new position.
If the song is not currently playing, it will take effect after the next call to PlayMusic.

If the song contains subsongs, this only affects the current subsong.
Changing subsongs resets the seek position to the beginning.
@param songID	The song ID.
@param seconds	The time in seconds to seek to.
@param mode		0 for absolute seeking, 1 for relative seeking.
*/
extern "C" DLL_EXPORT void SeekMusic(int songID, float seconds, int mode);
/*
@desc Changes the number of times the current song will loop.
This resets the loop count to 0.
@param loop		The number of times to loop, or 1 to loop forever.
*/
extern "C" DLL_EXPORT void SetMusicLoopCount(int loop);
/*
@desc Sets the subsong for a song.
This also resets the seek position for the song to 0.

If changing the subsong for the currently playing song, the subsong will immediately begin playing from the beginning.
@param songID The ID of the song.
@param subsong The subsong index.
*/
extern "C" DLL_EXPORT void SetMusicSubsong(int songID, int subsong);
/*
@desc Sets the volume of the emulator.
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
