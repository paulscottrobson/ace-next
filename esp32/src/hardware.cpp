// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hardware.cpp
//		Purpose:	Hardware Emulation
//		Created:	22nd October 2021
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include "sys_processor.h"
#include "hardware.h"
#include "gfxkeys.h"
#include <stdlib.h>

static int lastToggleCycleTime = 0;
static int cycleToggleCount = 0;
static int cycleToggleTotal = 0;
static int screenInvalid = 0;
static BYTE8 colourRAM[0x400];

static BYTE8 HWForceKey(WORD16 addr,BYTE8 v,int unshifted,int shifted,int controlled);
static BYTE8 _HWUpdateKey(WORD16 addr,BYTE8 v,int keyInfo);

// *******************************************************************************************************************************
//												Reset Hardware
// *******************************************************************************************************************************

void HWReset(void) {	
	HWXSetFrequency(0);
	lastToggleCycleTime = 0;
	cycleToggleCount = cycleToggleTotal = 0;
	HWInvalidateScreen();
	TAPEReset();
}

// *******************************************************************************************************************************
//												  Reset CPU
// *******************************************************************************************************************************

#include <stdio.h>

void HWSync(void) {
	HWXSyncImplementation(0);
	if (lastToggleCycleTime != 0 && cycleToggleCount != 0) {
		//
		//		The actual frequency is Clock Frequency (3.54Mhz) / 64 / Sound parameter.
		//
		int frequency = CYCLE_RATE/2*cycleToggleCount/cycleToggleTotal;
		HWXSetFrequency(frequency);
	} else {
		HWXSetFrequency(0);
	}
	lastToggleCycleTime = 0;
	//
	// 		Screen invalid (bitmap changed) then repaint whole screen.
	//
	if (screenInvalid) {
		for (int i = 0;i < 32*24;i++) {
			HWWriteDisplay(i,CPUReadMemory(0x2400+i),colourRAM[i]);
		}
		screenInvalid = 0;
	}
}

// *******************************************************************************************************************************
//													Write to physical display
// *******************************************************************************************************************************

void HWWriteDisplay(WORD16 pos,BYTE8 data,BYTE8 colour) {
	colourRAM[pos & 0x3FF] = colour;
	HWXWriteDisplay(pos,data,colour);
}

// *******************************************************************************************************************************
//														Read Colour RAM
// *******************************************************************************************************************************

BYTE8 HWReadColourRAM(WORD16 pos) {
	return colourRAM[pos & 0x3FF];
}

// *******************************************************************************************************************************
//														Force screen redraw
// *******************************************************************************************************************************

void HWInvalidateScreen(void) {
	screenInvalid = -1;
}

// *******************************************************************************************************************************
//															Fixes for keyboard
// *******************************************************************************************************************************

#include <stdio.h>

static BYTE8 HWForceKey(WORD16 addr,BYTE8 v,int unshifted,int shifted,int controlled) {
	v = 0;
	if (HWXIsKeyPressed(GFXKEY_SHIFT)) {
		if (shifted != 0) v = _HWUpdateKey(addr,0,shifted);
	} else if (HWXIsKeyPressed(GFXKEY_CONTROL)) {
		if (controlled != 0) v = _HWUpdateKey(addr,0,controlled);
	} else {
		if (unshifted != 0) v = _HWUpdateKey(addr,0,unshifted);		
	}
	return v;
}


static BYTE8 _HWUpdateKey(WORD16 addr,BYTE8 v,int keyInfo) {
	if ((addr & (keyInfo & 0xFF00)) == 0) { 							// If required row being scanned.
		v = v | (keyInfo & 0xFF); 										// Set the bits required.
	}
	int modifier = keyInfo >> 16;
	if (modifier == 1) { 												// Symbol (e.g. Shift)
		if ((addr & 0x100) == 0) v |= 0x02;
	}
	if (modifier == 2) { 												// Shift (e.g. Ctrl)
		if ((addr & 0x100) == 0) v |= 0x01;
	}
	return v;
}

// *******************************************************************************************************************************
//										  Extended keyboard handlers
// *******************************************************************************************************************************

static BYTE8 _HWCheckBit(WORD16 portAddr,WORD16 key) {
	BYTE8 v = 0; 
	int scanRows = (portAddr ^ 0xFFFF) & 0xFF00; 			// Bits set if row being scanned
	if ((key & scanRows) == 0) {  							// If row scanned is active low.
		v = v | (key & 0xFF); 								// Set the bits (caller makes active low)
	}
	return v;
}

static BYTE8 _HWProcessKeyboardKey(WORD16 port,WORD16 us1,WORD16 us2,WORD16 s1,WORD16 s2) {
	BYTE8 v,isShift = (HWGetKeyboardRow(0) & 1) != 0; 		// Symbol shift 
	if (isShift) {
		v = _HWCheckBit(port,s1) | _HWCheckBit(port,s2);
	} else {
		v = _HWCheckBit(port,us1) | _HWCheckBit(port,us2);
	}
	return v ^ 0xFF;
};

// *******************************************************************************************************************************
//												Port Read/Write
// *******************************************************************************************************************************

#define KEYPRESSED(n) 	HWXIsKeyPressed(n)
#define FIXKEYBOARD(n1,n2,n3,n4) v = _HWProcessKeyboardKey(addr,n1,n2,n3,n4) 

BYTE8 HWReadPort(WORD16 addr) {
	BYTE8 v = 0xFF;
	BYTE8 port = addr & 0xFF;

	if (port == 0xFE) {
		v = 0;
		for (int i = 0;i < 8;i++) {
			if ((addr & (0x0100 << i)) == 0) {
				v |= HWGetKeyboardRow(i);
			}
		}
		#ifndef STANDARD_HARDWARE
		#include "_keyboard_fix.h"		
		#endif
		v ^= 0xFF;	
	}
	return v;
}

void HWWritePort(WORD16 addr,BYTE8 data) {
	BYTE8 port = addr & 0xFF;
		if (port == 0xFE) {
		if (lastToggleCycleTime == 0) {
			cycleToggleCount = 0;
			cycleToggleTotal = 0;
		} else {
			cycleToggleCount++;
			cycleToggleTotal += abs(lastToggleCycleTime - CPUGetCycles());
		}
		lastToggleCycleTime = CPUGetCycles();
	}
}

// ****************************************************************************
//
//							Key codes for the ports
//
// ****************************************************************************

static int keys[][8] = {
	{ GFXKEY_CONTROL,GFXKEY_SHIFT,'Z','X','C',0,0,0 },
	{ 'A','S','D','F','G',0,0,0 },
	{ 'Q','W','E','R','T',0,0,0 },
	{ '1','2','3','4','5',0,0,0 },
	{ '0','9','8','7','6',0,0,0 },
	{ 'P','O','I','U','Y',0,0,0 },
	{ GFXKEY_RETURN,'L','K','J','H',0,0,0 },
	{ ' ','M','N','B','V',0,0,0 }
};

// ****************************************************************************
//					Get the keys pressed for a particular row
// ****************************************************************************

int HWGetKeyboardRow(int row) {
	int word = 0;
	int p = 0;
	while (keys[row][p] != 0) {
		if (HWXIsKeyPressed(keys[row][p])) word |= (1 << p);
		p++;
	}
	return word;
}

