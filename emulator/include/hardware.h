// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hardware.h
//		Purpose:	Hardware Emulation Header
//		Created:	22nd October 2021
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#ifndef _HARDWARE_H
#define _HARDWARE_H

void HWReset(void);
void HWSync(void);
BYTE8 HWReadPort(WORD16 addr);
void HWWritePort(WORD16 addr,BYTE8 data);
int HWGetKeyboardRow(int row);
void HWWriteDisplay(WORD16 pos,BYTE8 data,BYTE8 colour);
void HWInvalidateScreen(void);
BYTE8 HWReadColourRAM(WORD16 pos);
void HWLoadSpoolFile(char *name);
BYTE8 HWReadSpoolCharacter(void);
BYTE8 HWIsSpooling(void);
void HWStopSpooling(void);

void HWXSyncImplementation(LONG32 iCount);
BYTE8 HWXIsKeyPressed(WORD16 n);
WORD16 HWXLoadFile(char * fileName,BYTE8 *target);
WORD16 HWXSaveFile(char *fileName,BYTE8 *data,WORD16 size);
void HWXLoadDirectory(BYTE8 *target);
LONG32 HWXGetSystemClock(void);
void HWXSetFrequency(int frequency);
void HWWriteDisplay(WORD16 pos,BYTE8 data,BYTE8 colour);
void HWXWriteDisplay(WORD16 pos,BYTE8 data,BYTE8 colour);
char *HWXGetStorageType(void);

void TAPEReset(void);
BYTE8 TAPELoadBlock(WORD16 hl,WORD16 de,BYTE8 carry);
BYTE8 TAPLoadTAPFile(WORD16 tapAddr);
void TAPSaveTAPBlock(WORD16 start,WORD16 size);

#ifdef LINUX
#define FILESEP '/'
#else
#define FILESEP '\\'
#endif

#endif
