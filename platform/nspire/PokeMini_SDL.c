/*
  PokeMini - Pok�mon-Mini Emulator
  Copyright (C) 2009-2015  JustBurn

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "n2DLib.h"

#include "PokeMini.h"
#include "Hardware.h"
#include "ExportBMP.h"
#include "ExportWAV.h"
#include "Joystick.h"
#include "Keyboard.h"

#include "Video_x1.h"
#include "Video_x2.h"
#include "Video_x3.h"
#include "Video_x4.h"
#include "Video_x5.h"
#include "Video_x6.h"
#include "PokeMini_BG2.h"
#include "PokeMini_BG3.h"
#include "PokeMini_BG4.h"
#include "PokeMini_BG5.h"
#include "PokeMini_BG6.h"

const char *AppName = "PokeMini " PokeMini_Version " SDL";

int emurunning = 1, emulimiter = 0;
int PMWidth, PMHeight;
int PixPitch, PMOff, UIOff;

void setup_screen();

// Sound buffer size
#define SOUNDBUFFER	0
#define PMSNDBUFFER	0

// Custom command line (NEW IN 0.5.0)
const TCommandLineCustom CustomArgs[] = {
	{ "", NULL, COMMANDLINE_EOL }
};
const TCommandLineCustom CustomConf[] = {
	{ "", NULL, COMMANDLINE_EOL }
};

// Platform menu (REQUIRED >= 0.4.4)
int UIItems_PlatformC(int index, int reason);
TUIMenu_Item UIItems_Platform[] = {
	PLATFORMDEF_GOBACK,
	/*{ 0,  9, "Define Buttons...", UIItems_PlatformC },*/
	PLATFORMDEF_SAVEOPTIONS,
	PLATFORMDEF_END(UIItems_PlatformC)
};

int UIItems_PlatformC(int index, int reason)
{
	int zoomchanged = 0;
	if (reason == UIMENU_OK) {
		reason = UIMENU_RIGHT;
	}
	if (reason == UIMENU_CANCEL) {
		UIMenu_PrevMenu();
	}
	/*if (reason == UIMENU_RIGHT) {
		switch (index) {
			case 9: // Define Keyboard...
				KeyboardEnterMenu();
				break;
		}
	}*/
	return 1;
}

// Setup screen
void setup_screen()
{
	TPokeMini_VideoSpec *videospec;
	int depth, PMOffX, PMOffY, UIOffX, UIOffY;

	videospec = (TPokeMini_VideoSpec *)&PokeMini_Video3x3;
	PMWidth = 304; 
	PMHeight = 208; 
	PMOffX = 16; 
	PMOffY = 48; 
	UIOffX = 16; 
	UIOffY = 48;
	UIMenu_SetDisplay(288, 192, PokeMini_BGR16, (uint8_t *)PokeMini_BG3, (uint16_t *)PokeMini_BG3_PalBGR16, (uint32_t *)PokeMini_BG3_PalBGR32);

	// Set video spec and check if is supported
	depth = PokeMini_SetVideo(videospec, 16, CommandLine.lcdfilter, CommandLine.lcdmode);

	UIMenu_Process();
	
	clearBufferB();
	updateScreen();

	// Calculate pitch and offset
	PixPitch = 320;
	PMOff = (PMOffY * 320) + (PMOffX * 2);
	UIOff = (UIOffY * 320) + (UIOffX * 2);
}

#define PAD_BUTTON_LEFT 0x0001
#define PAD_BUTTON_RIGHT 0x0002
#define PAD_BUTTON_DOWN 0x0004
#define PAD_BUTTON_UP 0x0008
#define PAD_TRIGGER_Z 0x0010
#define PAD_TRIGGER_R 0x0020
#define PAD_TRIGGER_L 0x0040
#define PAD_BUTTON_A 0x0100
#define PAD_BUTTON_B 0x0200
#define PAD_BUTTON_X 0x0400
#define PAD_BUTTON_Y 0x0800
#define PAD_BUTTON_MENU 0x1000
#define PAD_BUTTON_START 0x1000

// Handle keyboard and quit events
void handleevents()
{
	unsigned int press = 0;
	
	t_key pad;
	if (isKeyPressed(KEY_NSPIRE_UP))
		press = press << PAD_BUTTON_UP;
	if (isKeyPressed(KEY_NSPIRE_LEFT))
		press = press << PAD_BUTTON_LEFT;
	if (isKeyPressed(KEY_NSPIRE_RIGHT))
		press = press << PAD_BUTTON_RIGHT;
	if (isKeyPressed(KEY_NSPIRE_DOWN))
		press = press << PAD_BUTTON_DOWN;
	if (isKeyPressed(KEY_NSPIRE_CTRL))
		press = press << PAD_BUTTON_A;
	if (isKeyPressed(KEY_NSPIRE_SHIFT))
		press = press << PAD_BUTTON_B;
	if (isKeyPressed(KEY_NSPIRE_TAB))
		press = press << PAD_BUTTON_START;
	if (isKeyPressed(KEY_NSPIRE_ESC))
		emurunning = 0;
		
	JoystickBitsEvent(press);
}

// Used to fill the sound buffer
void emulatorsound(void *unused, uint8_t *stream, int len)
{
}

// Enable / Disable sound
void enablesound(int sound)
{
}

// Callback when joystick is re-opened
void reopen_joystick(int enable, int index)
{
}

// Menu loop
void menuloop()
{
	//SDL_Event event;

	// Update window's title and stop sound
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// Update EEPROM
	PokeMini_SaveFromCommandLines(0);
	
	clearBufferB();
	updateScreen();

	// Menu's loop
	while (emurunning && (UI_Status == UI_STATUS_MENU)) 
	{
		// Slowdown to approx. 60fps
		// Process UI
		UIMenu_Process();
			
		// Render the menu or the game screen
		UIMenu_Display_16((uint16_t *)((uint8_t *)BUFF_BASE_ADDRESS + UIOff), PixPitch);
		
		updateScreen();

		// Handle events
		handleevents();
		
	}
	
	clearBufferB();
	updateScreen();

	// Apply configs
	PokeMini_ApplyChanges();
	if (UI_Status == UI_STATUS_EXIT) emurunning = 0;
	//SDL_EnableKeyRepeat(0, 0);
}

// Main function
int main(int argc, char **argv)
{
	//SDL_Event event;
	char title[256];
	char fpstxt[16];

	// Process arguments
	printf("%s\n\n", AppName);
	PokeMini_InitDirs(argv[0], NULL);
	CommandLineInit();
	CommandLineConfFile("pokemini.cfg.tns", "pokemini_sdl.cfg.tns", CustomConf);

	// Initialize SDL
	//SDL_Init(SDL_INIT_VIDEO);
	
	initBuffering();
	
	clearBufferB();
	updateScreen();
	
	setup_screen();
	enablesound(0);
	//SDL_EnableKeyRepeat(0, 0);
	
	// Initialize the emulator
	printf("Starting emulator...\n");
	if (!PokeMini_Create(0, PMSNDBUFFER)) {
		fprintf(stderr, "Error while initializing emulator\n");
		return 1;
	}

	// Setup palette and LCD mode
	PokeMini_VideoPalette_Init(PokeMini_BGR16, 1);
	PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
	PokeMini_ApplyChanges();

	// Load stuff
	PokeMini_UseDefaultCallbacks();
	if (!PokeMini_LoadFromCommandLines("Using FreeBIOS", "EEPROM data will be discarded!")) {
		UI_Status = UI_STATUS_MENU;
	}

	// Enable sound & init UI
	printf("Running emulator...\n");
	UIMenu_Init();
	//KeyboardRemap(&KeybMapSDL);

	// Emulator's loop
	unsigned long time, NewTickFPS = 0, NewTickSync = 0;
	int fps = 72, fpscnt = 0;
	while (emurunning) 
	{
		// Emulate and syncupdateScreen();ronize
		PokeMini_EmulateFrame();
		
		// Render the menu or the game screen
		PokeMini_VideoBlit((void *)((uint8_t *)BUFF_BASE_ADDRESS + PMOff), PixPitch);
		
		updateScreen();

		// Handle events
		handleevents();
		
		// Menu
		if (UI_Status == UI_STATUS_MENU) menuloop();
	}

	// Disable sound & free UI
	enablesound(0);
	
	deinitBuffering();
	
	UIMenu_Destroy();

	// Save Stuff
	PokeMini_SaveFromCommandLines(1);

	// Terminate...
	printf("Shutdown emulator...\n");
	PokeMini_VideoPalette_Free();
	PokeMini_Destroy();
	//SDL_Quit();

	return 0;
}
