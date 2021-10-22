// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		sys_processor.cpp
//		Purpose:	Processor Emulation.
//		Created:	22nd October 2021
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdio.h>
#include <string.h>
#include "sys_processor.h"
#include "sys_debug_system.h"
#include "hardware.h"

// *******************************************************************************************************************************
//														   Timing
// *******************************************************************************************************************************

#define FRAME_RATE		(50)														// Frames per second (50Hz)
#define CYCLES_PER_FRAME (CYCLE_RATE / FRAME_RATE) 									// Cycles per frame.

// *******************************************************************************************************************************
//														CPU / Memory
// *******************************************************************************************************************************

static BYTE8 A,B,C,D,E,H,L;  														// Standard register
static WORD16 AFalt,BCalt,DEalt,HLalt; 												// Alternate data set.
static WORD16 PC,SP; 																// 16 bit registers
static WORD16 IX,IY; 																// IX IY accessed indirectly.

static BYTE8 s_Flag,z_Flag,c_Flag,h_Flag,n_Flag,p_Flag; 							// Flag Registers
static BYTE8 I,R,intEnabled; 														// We don't really bother with these much.

static BYTE8 ramMemory[0x10000];													// Memory at $0000 upwards
static BYTE8 colourLatch; 															// ETI Colour latch at $2700

static LONG32 temp32;
static WORD16 temp16,temp16a,*pIXY;
static BYTE8 temp8,oldCarry;
static int frameCount = 0;

static int cycles;																	// Cycle Count.
static WORD16 cyclesPerFrame = CYCLES_PER_FRAME;									// Cycles per frame
static BYTE8 fastMode = 0; 															// Running in fast mode.

#define CYCLES(n) cycles -= (n)

// *******************************************************************************************************************************
//														ROM Image
// *******************************************************************************************************************************

static const BYTE8 _monitor_rom[] = {
	#include "default_rom.h"
};

static const BYTE8 _fixed_rom[] = {
	#include "fixed_rom.h"
};

static const BYTE8 _char_rom[] = {
	#include "char_rom.h"
};

// *******************************************************************************************************************************
//											 Memory and I/O read and write macros.
// *******************************************************************************************************************************

#define READ8(a) 	_Read(a)														// Basic Read
#define WRITE8(a,d)	_Write(a,d)														// Basic Write

#define READ16(a) 	(READ8(a) | ((READ8((a)+1) << 8)))								// Read 16 bits.
#define WRITE16(a,d) { WRITE8(a,(d) & 0xFF);WRITE8((a)+1,(d) >> 8); } 				// Write 16 bits

#define FETCH8() 	ramMemory[PC++]													// Fetch byte
#define FETCH16()	_Fetch16()	 													// Fetch word

static inline BYTE8 _Read(WORD16 address);											// Need to be forward defined as 
static inline void _Write(WORD16 address,BYTE8 data);								// used in support functions.

#define INPORT(p) 	HWReadPort(p)
#define OUTPORT(p,d) HWWritePort(p,d)

// *******************************************************************************************************************************
//											   Read and Write Inline Functions
// *******************************************************************************************************************************

static inline BYTE8 _Read(WORD16 address) {
	if (address < RAMSIZE) {
		return ramMemory[address];							
	}
	return 0xFF;
}

static void _Write(WORD16 address,BYTE8 data) {
	if (address >= 0x3000 && address <= RAMSIZE) {
		ramMemory[address] = data;
		return;
	}

	if (address >= 0x2000 && address < 0x2800) { 								// 2000-27FF is mirrored twice.
		WORD16 n = address & 0x3FF;
		ramMemory[0x2000+n] = data;
		ramMemory[0x2400+n] = data;
		HWWriteDisplay(n,data,colourLatch);
	}
	if (address >= 0x2800 && address < 0x3000) { 								// 2800-2FFF is mirrored twice.
		WORD16 n = address & 0x3FF;
		ramMemory[0x2800+n] = data;
		ramMemory[0x2C00+n] = data;
		HWInvalidateScreen();
	}
	if (address >= 0x3000 && address < 0x4000) { 								// 3000-3FFF is mirrored four times.
		WORD16 n = address & 0x3FF;
		ramMemory[0x3000+n] = data;
		ramMemory[0x3400+n] = data;
		ramMemory[0x3800+n] = data;
		ramMemory[0x3C00+n] = data;
	}
	if (address == 0x2700) { 													// Writing to the colour latch
		if (data & 0x80) colourLatch = data; 									// Only when bit 7 set.
	}	
	if (address == 0xFF) {
		fastMode = data; 														// Control fast mode.
	}
}

static inline WORD16 _Fetch16(void) {
	WORD16 w = ramMemory[PC] + ((ramMemory[PC+1]) << 8);
	PC += 2;
	return w;
}


// *******************************************************************************************************************************
//											 Support macros and functions
// *******************************************************************************************************************************

#ifdef INCLUDE_DEBUGGING_SUPPORT
#include <stdlib.h>
#define FAILOPCODE(g,n) exit(fprintf(stderr,"Opcode %02x in group %s\n",n,g))
#else
#define FAILOPCODE(g,n) {}
#endif

#include "cpu_support.h"

WORD16 CPUGetCycles(void) {
	return cycles;
}

void CPUSetPC(WORD16 newPC) {
	PC = newPC;
}

void CPUCopyCharacterSet(void) {
	for (int i = 0;i < 128*8;i++) { 												// We initialise Char RAM space in reset
		ramMemory[0x2800+i] = _char_rom[i]; 										// Freeing up 1D7B => 1FFC.
		ramMemory[0x2C00+i] = _char_rom[i];		 									// For possible twiddling.
	}
}

void CPUWriteScreenChar(int c) {
	WORD16 cStack = SP; 	
	PUSH(PC);PC = 8;A = c;
	while (SP != cStack) { CPUExecuteInstruction(); }									
}

void CPUWriteString(char *msg,int colour) {
	CPUWriteMemory(0x2700,colour|0x80);
	CPUWriteMemory(0x2700,0x00);
	while (*msg != '\0') CPUWriteScreenChar(*msg++);
}

void CPUNewPrompt(void)
{
	char buffer[40];
	sprintf(buffer,"*** Jupiter Ace v%s ***\r\r",EMU_VERSION);
	CPUWriteString(buffer,0x06);
	sprintf(buffer,"%dk RAM available\r",(RAMSIZE/1024)-13);
	CPUWriteString(buffer,0x05);
	strcpy(buffer,"ETI Colour Board (Apr84)\r");
	CPUWriteString(buffer,0x05);
	sprintf(buffer,"%s storage\r",HWXGetStorageType());
	CPUWriteString(buffer,0x05);
	strcpy(buffer,"Fast ROM/CPU\r");
	CPUWriteString(buffer,0x05);
	strcpy(buffer,"\r");
	CPUWriteString(buffer,0x04);
}

BYTE8 CPUIsFastMode(void) {
	return fastMode != 0;
}

// *******************************************************************************************************************************
//														Reset the CPU
// *******************************************************************************************************************************

#ifdef INCLUDE_DEBUGGING_SUPPORT
static void CPULoadChunk(FILE *f,BYTE8* memory,int count);
#endif

void CPUReset(void) {
	HWReset();																		// Reset Hardware
	for (int i = 0;i < sizeof(_monitor_rom);i++) { 									// Reset the ROM image
		#ifdef STANDARD_MACHINE
		ramMemory[i] = _monitor_rom[i];
		#else
		ramMemory[i] = _fixed_rom[i];
		#endif
	}
	BuildParityTable();																// Build the parity flag table.
	PC = 0; 																		// Zero PC.
	cycles = CYCLES_PER_FRAME;
	colourLatch = 0x85; 
	fastMode = 0;
}

// *******************************************************************************************************************************
//					Called on exit, does nothing on ESP32 but required for compilation
// *******************************************************************************************************************************

#ifdef INCLUDE_DEBUGGING_SUPPORT
#include "gfx.h"
void CPUExit(void) {	
	printf("Exited via $FFFF");
	GFXExit();
}
#else
void CPUExit(void) {}
#endif

// *******************************************************************************************************************************
//												Execute a single instruction
// *******************************************************************************************************************************

BYTE8 CPUExecuteInstruction(void) {
	#ifdef INCLUDE_DEBUGGING_SUPPORT
	if (PC == 0xFFFF) CPUExit();
	#endif
	BYTE8 opcode = FETCH8();														// Fetch opcode.
	switch(opcode) {																// Execute it.
		#include "_code_group_0.h"
		default:
			FAILOPCODE("-",opcode);
	}
	if (cycles >= 0 ) return 0;														// Not completed a frame.
	cycles = cycles + cyclesPerFrame;												// Adjust this frame rate, up to x16 on HS
	#ifndef ESP32
	if (fastMode != 0) cycles = 33 * 1024 * 1024 / FRAME_RATE; 						// Clocked at about 33Mhz fast mode.
	#endif
	HWSync();																		// Update any hardware
	frameCount++;
	if (intEnabled) { 																// Interrupt enabled.
		PUSH(PC);PC = 0x38; 														// Do RST 38h
		intEnabled = 0;
	}
	//
	// 		If spooling, we still need interrupts because the keyboard needs them to work, but we don't want to 
	//		synchronise to the internal clock, because that will make it run slowly. So this means we do this every
	//	 	1024th frame we execute, which means it spools in very fast, effectively only limited by PC speed.
	//
	if (HWIsSpooling() && (frameCount & 1023) != 0) return 0;

	return FRAME_RATE;																// Return frame rate.
}

// *******************************************************************************************************************************
//												Read/Write Memory
// *******************************************************************************************************************************

BYTE8 CPUReadMemory(WORD16 address) {
	return READ8(address);
}

void CPUWriteMemory(WORD16 address,BYTE8 data) {
	WRITE8(address,data);
}

#ifdef INCLUDE_DEBUGGING_SUPPORT

// *******************************************************************************************************************************
//		Execute chunk of code, to either of two break points or frame-out, return non-zero frame rate on frame, breakpoint 0
// *******************************************************************************************************************************

BYTE8 CPUExecute(WORD16 breakPoint1,WORD16 breakPoint2) { 
	BYTE8 next;
	do {
		BYTE8 r = CPUExecuteInstruction();											// Execute an instruction
		if (r != 0) return r; 														// Frame out.
		next = CPUReadMemory(PC);
	} while (PC != breakPoint1 && PC != breakPoint2 && next != 0x40);				// Stop on breakpoint or $40 LD B,B break
	return 0; 
}

// *******************************************************************************************************************************
//									Return address of breakpoint for step-over, or 0 if N/A
// *******************************************************************************************************************************

WORD16 CPUGetStepOverBreakpoint(void) {
	BYTE8 op = CPUReadMemory(PC); 	
	if (op == 0xCD || (op & 0xC7) == 0xC4) return PC+3; 							// CALL/CALL xx
	if ((op & 0xC7) == 0xC7) return PC+1;											// RST
	return 0;																		// Do a normal single step
}

void CPUEndRun(void) {
	FILE *f = fopen("memory.dump","wb");
	fwrite(ramMemory,1,RAMSIZE,f);
	fclose(f);
}

void CPULoadBinary(char *fileName) {
	HWLoadSpoolFile(fileName);
}

// *******************************************************************************************************************************
//											Retrieve a snapshot of the processor
// *******************************************************************************************************************************

static CPUSTATUS st;																	// Status area

CPUSTATUS *CPUGetStatus(void) {
	st.AF = AF();
	st.BC = BC();
	st.DE = DE();
	st.HL = HL();
	st.AFalt = AFalt;
	st.BCalt = BCalt;
	st.DEalt = DEalt;
	st.HLalt = HLalt;
	st.PC = PC;
	st.SP = SP;
	st.IX = IX;
	st.IY = IY;
	st.IE = intEnabled;
	st.cycles = cycles;
	return &st;
}

#endif