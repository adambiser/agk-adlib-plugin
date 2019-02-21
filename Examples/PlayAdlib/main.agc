// Project: PlayImf 
// Created: 2019-01-22
#option_explicit

// show all errors
SetErrorMode(2)

#constant FONT_SIZE	20
#constant NEWLINE	Chr(10)

#constant NORMAL_COLOR		255, 255, 255
#constant HIGHLIGHT_COLOR	255, 255, 0

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
emulatorNames as string[4] = ["Nuked", "DOSBox", "Ken Silverman", "Tatsuyuki Satoh", "Dual OPL"]
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
controlButtonNames as string[4] = ["Stop", "Pause/_Resume", "System_Volume", "Song_Volume", "Reload_Songs"]
#constant CONTROL_BUTTON_START	1
#constant STOP_BUTTON			1
#constant PAUSE_BUTTON			2
#constant SYSTEM_VOLUME_BUTTON	3
#constant SONG_VOLUME_BUTTON	4
#constant RELOAD_SONGS_BUTTON	5
#constant EMULATOR_BUTTON_START	10

index as integer
buttonX as integer
buttonY as integer
// Control buttons
buttonX = BUTTON_PADDING + CONTROL_BUTTON_SIZE / 2
buttonY = GetWindowHeight() - CONTROL_BUTTON_SIZE / 2 - BUTTON_PADDING
for index = 0 to controlButtonNames.length
	//~ AddVirtualButton(CONTROL_BUTTON_START + index, GetWindowWidth() - CONTROL_BUTTON_SIZE / 2, CONTROL_BUTTON_SIZE * index + CONTROL_BUTTON_SIZE / 2, CONTROL_BUTTON_SIZE)
	AddVirtualButton(CONTROL_BUTTON_START + index, buttonX, buttonY, CONTROL_BUTTON_SIZE)
	SetVirtualButtonText(CONTROL_BUTTON_START + index, ReplaceString(controlButtonNames[index], "_", NEWLINE, -1))
	inc buttonX, CONTROL_BUTTON_SIZE + BUTTON_PADDING
next
// Emulator buttons
buttonX = GetWindowWidth() - emulatorNames.length * (BUTTON_PADDING + CONTROL_BUTTON_SIZE) - CONTROL_BUTTON_SIZE / 2 - BUTTON_PADDING
//~ dec buttonY, CONTROL_BUTTON_SIZE + BUTTON_PADDING
for index = 0 to emulatorNames.length
	AddVirtualButton(EMULATOR_BUTTON_START + index, buttonX, buttonY, CONTROL_BUTTON_SIZE)
	SetVirtualButtonText(EMULATOR_BUTTON_START + index, ReplaceString(emulatorNames[index], " ", NEWLINE, -1))
	inc buttonX, CONTROL_BUTTON_SIZE + BUTTON_PADDING
next

// Song info
global songInfoTextID as integer
songInfoTextID = CreateText("")
SetTextPosition(songInfoTextID, 0, 160)
SetTextSize(songInfoTextID, FONT_SIZE)

// Load songs
global songIDs as integer[]
global songNameTextIDs as integer[]
global currentSongIndex as integer = -1
global currentSongID as integer = 0

global externalDataFileNames as string[]
externalDataFileNames.insertsorted("go-_-go.bnk")
externalDataFileNames.insertsorted("icepatch.003")
externalDataFileNames.insertsorted("insts.dat")
externalDataFileNames.insertsorted("lines1.snd")
externalDataFileNames.insertsorted("SONG1.ins")
externalDataFileNames.insertsorted("standard.bnk")
externalDataFileNames.insertsorted("tafa.tim")

#constant SONG_NAME_X		400
#constant SONG_NAME_Y		0
global songsPerColumn as integer
songsPerColumn = (GetWindowHeight() - CONTROL_BUTTON_SIZE - BUTTON_PADDING) / FONT_SIZE

//
// Scans the current folder for files.
// Loads the external data files that it finds.
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
EndFunction filenames

Function LoadSongs()
	// Delete any existing songs.
	index as integer
	for index = 0 to songIDs.length
		if adlib.GetMusicExists(songIDs[index])
			adlib.DeleteMusic(songIDs[index])
		endif
		DeleteText(songNameTextIDs[index])
	next
	// Remove external data entries.
	adlib.DeleteAllExternalData()
	songIDs.length = -1
	songNameTextIDs.length = songIDs.length
	if SetFolder("songs")
		SetErrorMode(0)
		filenames as string[]
		filenames = GetSongFiles()
		// Load songs
		songIDs.length = filenames.length
		songNameTextIDs.length = filenames.length
		columnWidth as integer = 0
		filenameX as integer
		filenameX = SONG_NAME_X - BUTTON_PADDING
		filenameY as integer
		for index = 0 to songIDs.length
			if mod(index, songsPerColumn) = 0
				// Don't go too wide...
				if columnWidth > 150
					columnWidth = 150
				endif
				inc filenameX, columnWidth + BUTTON_PADDING
				columnWidth = 0
				filenameY = SONG_NAME_Y
			endif
			songIDs[index] = adlib.LoadMusicFromFile(filenames[index])
			if GetErrorOccurred()
				Message(GetLastError())
			else
				songNameTextIDs[index] = CreateText(filenames[index])
				SetTextPosition(songNameTextIDs[index], filenameX, filenameY)
				SetTextSize(songNameTextIDs[index], FONT_SIZE)
				testWidth as integer
				testWidth = GetTextTotalWidth(songNameTextIDs[index])
				if testWidth > columnWidth
					columnWidth = testWidth
				endif
			endif
			inc filenameY, FONT_SIZE
		next
		SetErrorMode(2)
		SetFolder("..")
	endif
	ChangeSong(-1)
EndFunction

Function ChangeEmulator(emulator as integer)
	// Shutdown is safe to call before the emulator has been initialized.
	// However, you'll likely only need to call Init since Shutdown is called when the plugin is unloaded.
	adlib.Shutdown()
	adlib.Init(emulator)
	SetVirtualButtonColor(EMULATOR_BUTTON_START + currentEmulator - 1, NORMAL_COLOR)
	currentEmulator = emulator
	SetVirtualButtonColor(EMULATOR_BUTTON_START + currentEmulator - 1, HIGHLIGHT_COLOR)
	currentSongIndex = -1
	SetTextString(songInfoTextID, "")
	LoadSongs()
EndFunction

Function ChangeSong(songIndex as integer)
	if currentSongIndex >= 0
		SetTextColor(songNameTextIDs[currentSongIndex], NORMAL_COLOR, 255)
	endif
	currentSongIndex = songIndex
	if currentSongIndex >= 0
		currentSongID = songIDs[currentSongIndex]
		// Load info before playing the song.
		LoadSongInformation()
		adlib.PlayMusic(currentSongID, 1)
		SetTextColor(songNameTextIDs[currentSongIndex], HIGHLIGHT_COLOR, 255)
	else
		currentSongID = 0
		SetTextString(songInfoTextID, "")
	endif
EndFunction

Function LoadSongInformation()
	if not currentSongID
		ExitFunction
	endif
	// Load song information
	info as string
	info = "CurrentSong ID: " + str(currentSongID) + NEWLINE
	info = info + "GetMusicExists: " + str(adlib.GetMusicExists(currentSongID)) + NEWLINE
	if adlib.GetMusicExists(currentSongID)
		info = info + "GetMusicVolume: " + str(adlib.GetMusicVolume(currentSongID)) + NEWLINE
		info = info + "GetMusicRate: " + str(adlib.GetMusicRate(currentSongID)) + NEWLINE
		// NOTE GetMusicDuration should not be called on a song while it is playing or it will start over.
		info = info + "GetMusicDuration: " + GetDurationString(adlib.GetMusicDuration(currentSongID)) + NEWLINE
		//~ info = info + "GetMusicPosition: " + str(adlib.GetMusicPosition(currentSongID), 2) + NEWLINE
	endif
	SetTextString(songInfoTextID, info)
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
	if currentSongID
		//~ Print("CurrentSong ID: " + str(currentSongID))
		//~ Print("GetMusicExists: " + str(adlib.GetMusicExists(currentSongID)))
		if adlib.GetMusicExists(currentSongID)
			//~ Print("GetMusicVolume: " + str(adlib.GetMusicVolume(currentSongID)))
			//~ Print("GetMusicRate: " + str(adlib.GetMusicRate(currentSongID)))
			//~ Print("GetMusicDuration: " + songDurations[currentSong])
			Print("GetMusicPosition: " + str(adlib.GetMusicPosition(currentSongID), 2))
		endif
	endif
	Sync()
	if GetVirtualButtonPressed(STOP_BUTTON)
		adlib.StopMusic()
	elseif GetVirtualButtonPressed(PAUSE_BUTTON)
		//~ Message("Not implemented")
		if adlib.GetMusicPlaying()
			if adlib.GetMusicPaused()
				adlib.ResumeMusic()
			else
				adlib.PauseMusic()
			endif
		endif
	elseif GetVirtualButtonPressed(SYSTEM_VOLUME_BUTTON)
		if adlib.GetMusicSystemVolume() = 100
			adlib.SetMusicSystemVolume(50)
		else
			adlib.SetMusicSystemVolume(100)
		endif
	elseif GetVirtualButtonPressed(SONG_VOLUME_BUTTON)
		if currentSongID
			if adlib.GetMusicVolume(currentSongID) = 100
				adlib.SetMusicVolume(currentSongID, 50)
			else
				adlib.SetMusicVolume(currentSongID, 100)
			endif
			LoadSongInformation()
		endif
	elseif GetVirtualButtonPressed(RELOAD_SONGS_BUTTON)
		LoadSongs()
	else
		for index = EMULATOR_BUTTON_START to EMULATOR_BUTTON_START + emulatorNames.length
			if GetVirtualButtonPressed(index)
				ChangeEmulator(OPL_NUKED + index - EMULATOR_BUTTON_START)
				exit
			endif
		next
	endif
	if GetPointerPressed()
		mouseX as float
		mouseY as float
		mouseX = GetPointerX()
		mouseY = GetPointerY()
		for index = 0 to songNameTextIDs.length
			if not GetTextExists(songNameTextIDs[index])
				continue
			endif
			if GetTextHitTest(songNameTextIDs[index], mouseX, mouseY)
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
	seconds = duration
	minutes as integer
	minutes = seconds / 60
	dec seconds, minutes * 60
	text as string
	text = str(seconds)
	if seconds < 10
		text = "0" + text
	endif
	text = str(minutes) + ":" + text
EndFunction text
