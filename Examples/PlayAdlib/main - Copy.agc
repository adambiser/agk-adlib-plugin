// Project: PlayImf 
// Created: 2019-01-22
#option_explicit

// show all errors
SetErrorMode(2)

// set window properties
SetWindowTitle("PlayAdlib")
SetWindowSize(1024, 768, 0)
SetWindowAllowResize(1) // allow the user to resize the window
// set display properties
SetVirtualResolution(1024, 768) // doesn't have to match the window
SetOrientationAllowed(1, 1, 1, 1) // allow both portrait and landscape on mobile devices
SetSyncRate(30, 0) // 30fps instead of 60 to save battery
SetScissor(0, 0, 0, 0) // use the maximum available screen space, no black borders
UseNewDefaultFonts(1) // since version 2.0.22 we can use nicer default fonts

#import_plugin AdlibPlugin as adlib
adlib.Init()

#constant NEWLINE	Chr(10)

// Show composer and license.
#constant LICENSE_1	"Songs composed by Roppy Chop Studios."
#constant LICENSE_2	"License: CC0, public domain"
#constant LICENSE_3	"Website: (click to open)"
#constant COMPOSER_WEBSITE	"https://opengameart.org/content/original-midi-album"
licenseTextID as integer
licenseTextID = CreateText(LICENSE_1 + NEWLINE + LICENSE_2 + NEWLINE + LICENSE_3 + NEWLINE + COMPOSER_WEBSITE)
SetTextSize(licenseTextID, 24)
SetTextPosition(licenseTextID, 10, 550)

// Load songs.
global songNames as string[7] = ["Major Scale", "Broken Arm", "Casual Afternoon", "Icy Garden", "Journey Forgotten", "No One", "Standardized Anxiety", "Without Time"]
global songIDs as integer[]
songIDs.length = songNames.length
global songDurations as string[]
songDurations.length = songNames.length

global index as integer
global currentSong as integer

//~ Message(GetWritePath() + GetFolder())

Function LoadSongs()
	for index = 0 to songNames.length
		if adlib.GetMusicExists(songIDs[index])
			adlib.DeleteMusic(songIDs[index])
		endif
		memblock as integer
		ext as string
		if index = 0
			//~ songIDs[index] = adlib.LoadMusicFromFile("imf/adlibsp.s3m")
			memblock = CreateMemblockFromFile("imf/adlibsp.s3m")
			ext = "s3m"
		else
			//~ songIDs[index] = adlib.LoadMusicFromFile("imf/" + songNames[index] + ".wlf")
			memblock = CreateMemblockFromFile("imf/" + songNames[index] + ".wlf")
			ext = "wlf"
		endif
		songIDs[index] = adlib.LoadMusicFromMemblock(memblock, ext)
		songDurations[index] = GetDurationString(adlib.GetMusicDuration(songIDs[index]))
	next
EndFunction

// Create virtual buttons.
#constant BUTTON_SIZE			100
#constant BUTTON_PADDING		10
#constant CONTROL_BUTTON_SIZE	80
#constant CONTROL_BUTTON_START	10

controlButtonNames as string[5] = ["Stop", "Pause/_Resume", "Restart_Synth", "Load_Songs", "System_Volume", "Song_Volume"] //, "Change_Rate"]
#constant STOP_BUTTON			10
#constant PAUSE_BUTTON			11
#constant RESTART_BUTTON		12
#constant LOAD_SONGS_BUTTON		13
#constant SYSTEM_VOLUME_BUTTON	14
#constant SONG_VOLUME_BUTTON	15
//~ #constant CHANGE_RATE_BUTTON	16

for index = 0 to songNames.length
	AddVirtualButton(index + 1, BUTTON_PADDING + index * (BUTTON_SIZE + BUTTON_PADDING) + BUTTON_SIZE / 2, 700, BUTTON_SIZE)
	SetVirtualButtonText(index + 1, ReplaceString(songNames[index], " ", NEWLINE, -1))
next
for index = 0 to controlButtonNames.length
	AddVirtualButton(index + CONTROL_BUTTON_START, GetWindowWidth() - CONTROL_BUTTON_SIZE / 2, CONTROL_BUTTON_SIZE * index + CONTROL_BUTTON_SIZE / 2, CONTROL_BUTTON_SIZE)
	SetVirtualButtonText(index + CONTROL_BUTTON_START, ReplaceString(controlButtonNames[index], "_", NEWLINE, -1))
next

LoadSongs()
//~ adlib.PlayMusic(songIDs[0], 1)

do
	adlib.Update()
	Print("FPS: " + str(ScreenFPS(), 1))
	Print("GetMusicPlaying: " + str(adlib.GetMusicPlaying()))
	Print("GetMusicPaused: " + str(adlib.GetMusicPaused()))
	Print("GetMusicLoopCount: " + str(adlib.GetMusicLoopCount()))
	Print("GetMusicSystemVolume: " + str(adlib.GetMusicSystemVolume()))
	Print("")
	if currentSong
		Print("CurrentSong: " + songNames[currentSong - 1])
		Print("GetMusicExists: " + str(adlib.GetMusicExists(songIDs[currentSong - 1])))
		if adlib.GetMusicExists(songIDs[currentSong - 1])
			Print("GetMusicVolume: " + str(adlib.GetMusicVolume(songIDs[currentSong - 1])))
			Print("GetMusicRate: " + str(adlib.GetMusicRate(songIDs[currentSong - 1])))
			Print("GetMusicDuration: " + songDurations[currentSong - 1])
			Print("GetMusicPosition: " + str(adlib.GetMusicPosition(songIDs[currentSong - 1]), 2))
		endif
	endif
	Sync()
	// Click on composer info to open the site.
	if GetPointerPressed()
		mouseX as float
		mouseY as float
		mouseX = GetPointerX()
		mouseY = GetPointerY()
		if GetTextHitTest(licenseTextID, mouseX, mouseY)
			OpenBrowser(COMPOSER_WEBSITE)
		endif
	endif
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
	elseif GetVirtualButtonPressed(RESTART_BUTTON)
		adlib.Shutdown()
		adlib.Init()
	elseif GetVirtualButtonPressed(LOAD_SONGS_BUTTON)
		LoadSongs()
	elseif GetVirtualButtonPressed(SYSTEM_VOLUME_BUTTON)
		if adlib.GetMusicSystemVolume() = 100
			adlib.SetMusicSystemVolume(50)
		else
			adlib.SetMusicSystemVolume(100)
		endif
	elseif GetVirtualButtonPressed(SONG_VOLUME_BUTTON)
		if currentSong
			if adlib.GetMusicVolume(songIDs[currentSong - 1]) = 100
				adlib.SetMusicVolume(songIDs[currentSong - 1], 50)
			else
				adlib.SetMusicVolume(songIDs[currentSong - 1], 100)
			endif
		endif
	//~ elseif GetVirtualButtonPressed(CHANGE_RATE_BUTTON)
		//~ if currentSong
			//~ if adlib.GetMusicRate(songIDs[currentSong - 1]) = 700
				//~ adlib.SetMusicRate(songIDs[currentSong - 1], 560)
			//~ else
				//~ adlib.SetMusicRate(songIDs[currentSong - 1], 700)
			//~ endif
			//~ songDurations[currentSong - 1] = GetDurationString(adlib.GetMusicDuration(songIDs[currentSong - 1]))
		//~ endif
	else
		for index = 1 to songIDs.length + 1
			If GetVirtualButtonPressed(index)
				currentSong = index
				adlib.PlayMusic(songIDs[index - 1], 1)
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
