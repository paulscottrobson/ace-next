// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hwtape.cpp
//		Purpose:	Tape Emulation
//		Created:	22nd October 2021
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include "sys_processor.h"
#include "hardware.h"
#include "gfxkeys.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdio.h>

static BYTE8 *nextBlock;

static BYTE8 tapeBuffer[20*1024];
static char  fileSave[32];
static int   tapCount;
static int   tapPtr = 0;
static int spoolIndex = -1;

// ****************************************************************************
//
//							   Tape Initialise
//
// ****************************************************************************

void TAPEReset(void) {
	nextBlock = NULL;
	tapCount = 0;
}

// ****************************************************************************
//
// 							Tape error
//
// ****************************************************************************

static BYTE8 TAPFail(void) {
	CPUSetPC(0x1AB6);  						// Error 10, tape error.
	return 1;
}
// ****************************************************************************
//
// 							Copy name from CPU Ram
//
// ****************************************************************************

static void _TAPGetName(char *buffer,WORD16 addr) {
	*buffer = '\0';
	if (CPUReadMemory(addr) <= 0x20) return;
	while (CPUReadMemory(addr) > 0x20) {
		*buffer++ = tolower(CPUReadMemory(addr));
		addr++;
	}
	strcpy(buffer,".tap");
}

// ****************************************************************************
//
//								Print directory
//
// ****************************************************************************

static void TAPPrintDirectory(void) {
	HWXLoadDirectory(tapeBuffer);
	char *p = (char *)tapeBuffer;
	while (*p != '\0') {
		if (strlen(p) >= 4 && strcmp(p+strlen(p)-4,".tap") == 0) {
			char *p1 = p;
			while (*p1 != '.') {
				CPUWriteScreenChar(*p1++);
			}
			CPUWriteScreenChar(32);
		}
		p = p + strlen(p) + 1;
	}
	CPUWriteScreenChar(13);
	CPUSetPC(0x00AD);
}
// ****************************************************************************
//
// 								Load in TAP file
//
// ****************************************************************************

BYTE8 TAPLoadTAPFile(WORD16 tapAddr) {
	char tapFileName[32];	
	_TAPGetName(tapFileName,tapAddr);
	if (tapFileName[0] == '\0') {
		TAPPrintDirectory();
		return 0;
	}
	for (int i = 0;i < sizeof(tapeBuffer);i++) tapeBuffer[i] = '\0';
	if (HWXLoadFile(tapFileName,tapeBuffer)) return TAPFail();

	nextBlock = tapeBuffer;
	return 0;
}

// ****************************************************************************
//
//									Tape Load
//
// ****************************************************************************

BYTE8 TAPELoadBlock(WORD16 hl,WORD16 de,BYTE8 carry) {
	if (nextBlock == NULL) return TAPFail();
	int block_size = nextBlock[0] + nextBlock[1] * 256;
	if (block_size == 0) return TAPFail();
	if (block_size > 0) {
		nextBlock += 2;
		for (int i = 0;i < block_size;i++) {
			if (carry) CPUWriteMemory(hl++,*nextBlock);
			nextBlock++;
		}
	}
	return -1;
}

// ****************************************************************************
//
//									Tape Save
//
// ****************************************************************************

void TAPSaveTAPBlock(WORD16 start,WORD16 size) {
	if (start == 0x2301 && size == 0x0019) {
		tapCount = tapPtr = 0;
		int i = 0;
		while (CPUReadMemory(start+i+1) != ' ') {
			fileSave[i] = tolower(CPUReadMemory(start+1+i));
			i++;
		}
		strcat(fileSave+i,".tap");
	} 
	tapeBuffer[tapPtr++] = size & 0xFF;
	tapeBuffer[tapPtr++] = size >> 8;
	for (int i = 0;i < size;i++) {
		tapeBuffer[tapPtr++] = CPUReadMemory(start+i);
	}
	tapCount++;
	if (tapCount == 2) {
		HWXSaveFile(fileSave,tapeBuffer,tapPtr);
	}
}

// ****************************************************************************
// 								Load spool file
// ****************************************************************************

void HWLoadSpoolFile(char *name) {
	for (int i = 0;i < sizeof(tapeBuffer);i++) tapeBuffer[i] = '\0';
	FILE *f = fopen(name,"rb");
	if (f != NULL) {
		//
		//		For some reason I don't quite understand it misses the first
		//		character, to do with the interplay between QUERY and the Interrupt ?
		//
		// 		Sticking a couple of CRs in front seems to fix it.
		//
		tapeBuffer[0] = tapeBuffer[1] = 0x0D;
		fread(tapeBuffer+2,1,sizeof(tapeBuffer),f);
		fclose(f);
		spoolIndex = 0;
	}
}

// ****************************************************************************
// 							 Read from spool file
// ****************************************************************************

BYTE8 HWReadSpoolCharacter(void) {
	BYTE8 c = 0;
	if (spoolIndex >= 0) {
		c = tapeBuffer[spoolIndex++];
		if (c == 0) spoolIndex = -1;
		if (c == 0x0A) c = 0x0D;
	}
	return c;
}

// ****************************************************************************
// 								Check is spooling
// ****************************************************************************

BYTE8 HWIsSpooling(void) {
	return (spoolIndex >= 0);
}

// ****************************************************************************
// 									Stop spooling
// ****************************************************************************

void HWStopSpooling(void) {
	spoolIndex = -1;
}
