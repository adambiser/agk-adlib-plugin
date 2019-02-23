// Project: PlayImf 
// Created: 2019-01-22
#option_explicit

// show all errors
SetErrorMode(2)

#constant FONT_SIZE	20
#constant NEWLINE	Chr(10)

#constant NORMAL_COLOR		255, 255, 255
#constant HIGHLIGHT_COLOR	255, 255, 0
#constant INACTIVE_COLOR	128, 128, 128

// set window properties
SetWindowTitle("PlayAdlib")
SetWindowSize(1024, 768, 0)
SetWindowAllowResize(1)
SetVirtualResolution(1024, 768)
SetOrientationAllowed(1, 1, 1, 1)
SetSyncRate(30, 0)
SetScissor(0, 0, 0, 0)
UseNewDefaultFonts(1)
SetPrintSize(FONT_SIZE)

#import_plugin AdlibPlugin as adlib

// Emulator types.
global emulatorNames as string[4] = ["Nuked", "DOSBox", "Ken Silverman", "Tatsuyuki Satoh", "Dual OPL"]
#constant OPL_NUKED		1
#constant OPL_DOSBOX	2
#constant OPL_SILVERMAN	3
#constant OPL_SATOH		4
#constant OPL_DUAL		5

global currentEmulator as integer = OPL_NUKED

#constant BUTTON_WIDTH			100
#constant BUTTON_HEIGHT			100
#constant BUTTON_PADDING		10
#constant CONTROL_BUTTON_SIZE	80

// Create control buttons.
global controlButtonNames as string[10] = ["Stop", "Pause/_Resume", "Seek_Middle", "Seek to_End - 10s", "Prev_Subsong", "Next_Subsong", "ADL #20_As Sound", "ADL #22_As Sound", "System_Volume", "Song_Volume", "Reload_Songs"]
#constant CONTROL_BUTTON_START	1
#constant STOP_BUTTON			1
#constant PAUSE_BUTTON			2
#constant SEEK_MIDDLE_BUTTON	3
#constant SEEK_END_BUTTON		4
#constant PREV_SUBSONG_BUTTON	5
#constant NEXT_SUBSONG_BUTTON	6
#constant SOUND_20_BUTTON		7
#constant SOUND_22_BUTTON		8
#constant SYSTEM_VOLUME_BUTTON	9
#constant SONG_VOLUME_BUTTON	10
#constant RELOAD_SONGS_BUTTON	11
#constant EMULATOR_BUTTON_START	20

index as integer
buttonX as integer
buttonY as integer
// Control buttons
buttonX = BUTTON_PADDING + CONTROL_BUTTON_SIZE / 2
buttonY = GetWindowHeight() - (BUTTON_PADDING + CONTROL_BUTTON_SIZE / 2) - (CONTROL_BUTTON_SIZE + BUTTON_PADDING) * 3
for index = 0 to controlButtonNames.length
	select index + CONTROL_BUTTON_START
		case PREV_SUBSONG_BUTTON, SYSTEM_VOLUME_BUTTON
			// Next row
			buttonX = BUTTON_PADDING + CONTROL_BUTTON_SIZE / 2
			inc buttonY, CONTROL_BUTTON_SIZE + BUTTON_PADDING
		endcase
	endselect
	AddVirtualButton(index + CONTROL_BUTTON_START, buttonX, buttonY, CONTROL_BUTTON_SIZE)
	SetVirtualButtonText(index + CONTROL_BUTTON_START, ReplaceString(controlButtonNames[index], "_", NEWLINE, -1))
	inc buttonX, CONTROL_BUTTON_SIZE + BUTTON_PADDING
next
// Emulator buttons
buttonX = BUTTON_PADDING + CONTROL_BUTTON_SIZE / 2
inc buttonY, CONTROL_BUTTON_SIZE + BUTTON_PADDING
for index = 0 to emulatorNames.length
	AddVirtualButton(index + EMULATOR_BUTTON_START, buttonX, buttonY, CONTROL_BUTTON_SIZE)
	SetVirtualButtonText(index + EMULATOR_BUTTON_START, ReplaceString(emulatorNames[index], " ", NEWLINE, -1))
	inc buttonX, CONTROL_BUTTON_SIZE + BUTTON_PADDING
next

//
// Change emulators.
//
Function ChangeEmulator(emulator as integer)
	// Note: Shutdown is safe to call before the emulator has been initialized.
	// However, you'll likely only need to call Init since Shutdown is called when the plugin is unloaded.
	// Aside from this demo, there's really not much of a reason to switch emulators while running.
	adlib.Shutdown()
	adlib.Init(emulator)
	SetVirtualButtonColor(currentEmulator + EMULATOR_BUTTON_START - 1, NORMAL_COLOR)
	currentEmulator = emulator
	SetVirtualButtonColor(currentEmulator + EMULATOR_BUTTON_START - 1, HIGHLIGHT_COLOR)
	LoadSongs()
EndFunction

// Load songs
Type SongInfo
	id as integer
	nameTextID as integer
	duration as float
	durationString as string
EndType
global songs as SongInfo[]

global currentSong as SongInfo

global externalDataFileNames as string[]
externalDataFileNames.insertsorted("go-_-go.bnk")
externalDataFileNames.insertsorted("icepatch.003")
externalDataFileNames.insertsorted("insts.dat")
externalDataFileNames.insertsorted("lines1.snd")
externalDataFileNames.insertsorted("SONG1.ins")
externalDataFileNames.insertsorted("standard.bnk")
externalDataFileNames.insertsorted("tafa.tim")

#constant MAX_SONG_COLUMN_WIDTH	150
global songNameX as integer
songNameX = GetWindowWidth() - (MAX_SONG_COLUMN_WIDTH + BUTTON_PADDING) * 2
global songNameY as integer = 0
global songsPerColumn as integer
songsPerColumn = GetWindowHeight() / FONT_SIZE

//
// Scans the current folder for files.
// Loads the external data files that it finds using the hard-coded list.
// Returns a sorted array of filenames.
//
Function GetSongFiles()
	filenames as string[]
	filename as String
	filename = GetFirstFile()
	while filename <> ""
		if externalDataFileNames.find(filename) >= 0
			// Add external data files now.
			adlib.LoadExternalDataFromFile(filename)
			if GetErrorOccurred()
				Message(GetLastError())
			endif
		else
			filenames.insertsorted(filename)
		endif
		filename = GetNextFile()
	endwhile
	//~ filenames.insert("DUNE19.ADL")
	//~ filenames.insert("EOBSOUND.ADL")
	//~ filenames.insert("LOREINTR.ADL")
EndFunction filenames

//
// Loads all of the music in the "songs" folder.
//
Function LoadSongs()
	// Clear the current song information.
	ChangeSong(-1)
	// Make sure all of the old songs, etc, are deleted first.
	adlib.DeleteAllExternalData()
	adlib.DeleteAllMusic()
	index as integer
	for index = 0 to songs.length
		DeleteText(songs[index].nameTextID)
	next
	songs.length = -1
	if SetFolder("songs")
		SetErrorMode(0)
		filenames as string[]
		filenames = GetSongFiles()
		// Load songs
		songs.length = filenames.length
		columnWidth as integer = 0
		filenameX as integer
		filenameX = songNameX - BUTTON_PADDING
		filenameY as integer
		for index = 0 to songs.length
			if mod(index, songsPerColumn) = 0
				// Don't go too wide...
				if columnWidth > MAX_SONG_COLUMN_WIDTH
					columnWidth = MAX_SONG_COLUMN_WIDTH
				endif
				inc filenameX, columnWidth + BUTTON_PADDING
				columnWidth = 0
				filenameY = songNameY
			endif
			songs[index].id = adlib.LoadMusicFromFile(filenames[index])
			if GetErrorOccurred()
				Message(GetLastError())
			else
				// NOTE GetMusicDuration should not be called on a song while it is playing or it will start over.
				songs[index].duration = adlib.GetMusicDuration(songs[index].id)
				songs[index].durationString = GetDurationString(songs[index].duration)
				// Show the filename in a list.
				songs[index].nameTextID = CreateText(filenames[index])
				SetTextPosition(songs[index].nameTextID, filenameX, filenameY)
				SetTextSize(songs[index].nameTextID, FONT_SIZE)
				testWidth as integer
				testWidth = GetTextTotalWidth(songs[index].nameTextID)
				if testWidth > columnWidth
					columnWidth = testWidth
				endif
			endif
			inc filenameY, FONT_SIZE
		next
		SetErrorMode(2)
		SetFolder("..")
	endif
EndFunction

Function ChangeSong(newSongIndex as integer)
	if currentSong.nameTextID
		SetTextColor(currentSong.nameTextID, NORMAL_COLOR, 255)
	endif
	if newSongIndex >= 0
		currentSong = songs[newSongIndex]
	else
		emptySongInfo as SongInfo
		currentSong = emptySongInfo
	endif
	// Load info before playing the song.
	hasSubSongs as integer
	if currentSong.id
		// Don't loop short songs.
		adlib.PlayMusic(currentSong.id, (currentSong.duration >= 2))
		SetTextColor(currentSong.nameTextID, HIGHLIGHT_COLOR, 255)
		// Check for subsongs.
		hasSubSongs = (adlib.GetMusicSubSongCount(currentSong.id) > 1)
	endif
	MySetVirtualButtonActive(STOP_BUTTON, currentSong.id)
	MySetVirtualButtonActive(PAUSE_BUTTON, currentSong.id)
	MySetVirtualButtonActive(SEEK_MIDDLE_BUTTON, currentSong.id)
	MySetVirtualButtonActive(SEEK_END_BUTTON, currentSong.id)
	MySetVirtualButtonActive(PREV_SUBSONG_BUTTON, currentSong.id and hasSubSongs)
	MySetVirtualButtonActive(NEXT_SUBSONG_BUTTON, currentSong.id and hasSubSongs)
	MySetVirtualButtonActive(SOUND_20_BUTTON, currentSong.id and hasSubSongs)
	MySetVirtualButtonActive(SOUND_22_BUTTON, currentSong.id and hasSubSongs)
EndFunction

Function MySetVirtualButtonActive(button as integer, active as integeR)
	SetVirtualButtonActive(button, active)
	if active
		SetVirtualButtonColor(button, NORMAL_COLOR)
	else
		SetVirtualButtonColor(button, INACTIVE_COLOR)
	endif
EndFunction

Function ChangeSubsong(newSubsong as integeR)
	// To use GetMusicDuration for a new subsong, make sure the song is not already playing because
	// SetMusicSubsong will start playing the new subsong automatically.
	// To use GetMusicDuration for a subsong, first call StopMusic.
	// However, to allow ADL's ability to play multiple subsongs at once (songs and sound effects),
	// This is not done.
	playing as integer
	playing = adlib.GetMusicPlaying()
	adlib.StopMusic()
	adlib.SetMusicSubsong(currentSong.id, newSubsong)
	index as integer
	index = songs.find(currentSong.id)
	songs[index].duration = adlib.GetMusicDuration(songs[index].id)
	songs[index].durationString = GetDurationString(songs[index].duration)
	// Refresh the currentsong's information, too.
	currentSong = songs[index]
	if playing
		// Don't loop short songs.
		adlib.PlayMusic(currentSong.id, (currentSong.duration >= 2))
	endif
EndFunction

ChangeEmulator(currentEmulator)

do
	adlib.Update()
	Print("FPS: " + str(ScreenFPS(), 1))
	Print("GetMusicPlaying: " + str(adlib.GetMusicPlaying()))
	Print("GetMusicPaused: " + str(adlib.GetMusicPaused()))
	Print("GetMusicLoopCount: " + str(adlib.GetMusicLoopCount()))
	Print("GetMusicSystemVolume: " + str(adlib.GetMusicSystemVolume()))
	Print("")
	Print("CurrentSong ID: " + str(currentSong.id))
	Print("GetMusicExists: " + str(adlib.GetMusicExists(currentSong.id)))
	if adlib.GetMusicExists(currentSong.id)
		Print("File Type: " + adlib.GetMusicType(currentSong.id))
		Print("Volume: " + str(adlib.GetMusicVolume(currentSong.id)))
		Print("Rate: " + str(adlib.GetMusicRate(currentSong.id)))
		Print("Subsong: " + str(adlib.GetMusicSubsong(currentSong.id)) + " of " + str(adlib.GetMusicSubsongCount(currentSong.id)))
		Print("")
		Print("Position: " + GetDurationString(adlib.GetMusicPosition(currentSong.id)))
		Print("Duration: " + currentSong.durationString)
		Print("")
		Print("Title: " + adlib.GetMusicTitle(currentSong.id))
		Print("Author: " + adlib.GetMusicAuthor(currentSong.id))
		Print("Description: " + adlib.GetMusicDescription(currentSong.id))
	endif
	Sync()
	if GetVirtualButtonPressed(STOP_BUTTON)
		adlib.StopMusic()
	elseif GetVirtualButtonPressed(PAUSE_BUTTON)
		if adlib.GetMusicPlaying()
			if adlib.GetMusicPaused()
				adlib.ResumeMusic()
			else
				adlib.PauseMusic()
			endif
		endif
	elseif GetVirtualButtonPressed(SEEK_MIDDLE_BUTTON)
		if currentSong.id
			adlib.PauseMusic()
			adlib.SeekMusic(currentSong.id, adlib.GetMusicDuration(currentSong.id) / 2, 0)
			adlib.ResumeMusic()
		endif
	elseif GetVirtualButtonPressed(SEEK_END_BUTTON)
		if currentSong.id
			adlib.SeekMusic(currentSong.id, adlib.GetMusicDuration(currentSong.id) - 10, 0)
		endif
	elseif GetVirtualButtonPressed(PREV_SUBSONG_BUTTON)
		if currentSong.id
			prevSubsong as integer
			prevSubsong = adlib.GetMusicSubsong(currentSong.id) - 1
			if prevSubsong < 0
				prevSubsong = adlib.GetMusicSubsongCount(currentSong.id) - 1
			endif
			ChangeSubsong(prevSubsong)
		endif
	elseif GetVirtualButtonPressed(NEXT_SUBSONG_BUTTON)
		if currentSong.id
			nextSubsong as integer
			nextSubsong = adlib.GetMusicSubsong(currentSong.id) + 1
			if nextSubsong >= adlib.GetMusicSubsongCount(currentSong.id)
				nextSubsong = 0
			endif
			ChangeSubsong(nextSubsong)
		endif
	elseif GetVirtualButtonPressed(SOUND_20_BUTTON)
		if currentSong.id
			adlib.PlaySound(currentSong.id, 20)
		endif
	elseif GetVirtualButtonPressed(SOUND_22_BUTTON)
		if currentSong.id
			adlib.PlaySound(currentSong.id, 22)
		endif
	elseif GetVirtualButtonPressed(SYSTEM_VOLUME_BUTTON)
		if adlib.GetMusicSystemVolume() = 100
			adlib.SetMusicSystemVolume(50)
		else
			adlib.SetMusicSystemVolume(100)
		endif
	elseif GetVirtualButtonPressed(SONG_VOLUME_BUTTON)
		if currentSong.id
			if adlib.GetMusicVolume(currentSong.id) = 100
				adlib.SetMusicVolume(currentSong.id, 50)
			else
				adlib.SetMusicVolume(currentSong.id, 100)
			endif
		endif
	elseif GetVirtualButtonPressed(RELOAD_SONGS_BUTTON)
		LoadSongs()
	else
		for index = 0 to emulatorNames.length
			if GetVirtualButtonPressed(EMULATOR_BUTTON_START + index)
				ChangeEmulator(OPL_NUKED + index)
				exit
			endif
		next
	endif
	if GetPointerPressed()
		mouseX as float
		mouseY as float
		mouseX = GetPointerX()
		mouseY = GetPointerY()
		for index = 0 to songs.length
			if not GetTextExists(songs[index].nameTextID)
				continue
			endif
			if GetTextHitTest(songs[index].nameTextID, mouseX, mouseY)
				ChangeSong(index)
				exit
			endif
		next
	endif
	if GetRawKeyPressed(27)
		end
	endif
loop

Function GetDurationString(duration as float)
	seconds as integer
	seconds = floor(duration)
	ms as float
	ms = duration - seconds
	minutes as integer
	minutes = seconds / 60
	dec seconds, minutes * 60
	text as string
	text = str(seconds)
	if seconds < 10
		text = "0" + text
	endif
	text = str(minutes) + ":" + text + mid(str(ms, 3), 2, -1)
EndFunction text
