// ****************************************************************************
// ****************************************************************************
//
//		Name:		main.cpp
//		Purpose:	Main Program (esp version)
//		Created:	8th March 2020
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// ****************************************************************************
// ****************************************************************************

#include "espinclude.h"

int nextFrameTime;

// ****************************************************************************
//
//								Set up code
//
// ****************************************************************************

void setup()
{
	Serial.begin(115200);delay(500);
	int fsOpen = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
	HWESPVideoInitialise();
	HWESPSoundInitialise();
	HWESPKeyboardInitialise();
	CPUReset();
	nextFrameTime = millis();
}

// ****************************************************************************
//
//									Execution
//
// ****************************************************************************

void loop()
{
    unsigned long frameRate = CPUExecuteInstruction();
    if (frameRate != 0) {
    	if (CPUIsFastMode() == 0) {
			while (millis() < nextFrameTime) {}
			nextFrameTime = millis() + 1000 / frameRate;
		}
	}
}	

