// ****************************************************************************
// ****************************************************************************
//
//		Name:		espmiscellany.cpp
//		Purpose:	Other Routines
//		Created:	22nd October 2021
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// ****************************************************************************
// ****************************************************************************

#include "espinclude.h"

fabgl::PS2Controller PS2Controller;
fabgl::Keyboard  Keyboard;

// ****************************************************************************
//		
//							Initialise Keyboard
//
// ****************************************************************************

void HWESPKeyboardInitialise(void) {
	Keyboard.begin(PS2_PORT0_CLK, PS2_PORT0_DAT,false,false);
}

// ****************************************************************************
//		
//							Get keyboard activity
//
// ****************************************************************************

int HWESPGetScanCode(void) {
	return Keyboard.getNextScancode(0);
}

// ****************************************************************************
//
//					Transmit character over serial port
//
// ****************************************************************************

void HWESPTransmitCharacter(BYTE8 ch) {
	Serial.print(((char)ch));
}
