// ****************************************************************************
// ****************************************************************************
//
//		Name:		espvideo.cpp
//		Purpose:	Video Routines
//		Created:	22nd October 2021
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// ****************************************************************************
// ****************************************************************************

#include "espinclude.h"

// ****************************************************************************
//
//								Static objects
//
// ****************************************************************************

fabgl::VGAController DisplayController;
fabgl::Canvas        Canvas(&DisplayController);

#define PALETTE_SIZE 	16

static RGB888 pColours[PALETTE_SIZE];
static uint8_t rawPixels[PALETTE_SIZE];

#define DWIDTH 			320
#define DHEIGHT 		240 
#define XOFFSET 		32
#define YOFFSET 		24

// ****************************************************************************
//
//						Initialise video subsystem
//
// ****************************************************************************

void HWESPVideoInitialise(void) {
	#if USE_8_COLORS
	DisplayController.begin(VGA_RED, VGA_GREEN, VGA_BLUE, VGA_HSYNC, VGA_VSYNC);
	#elif USE_64_COLORS
	DisplayController.begin(VGA_RED1, VGA_RED0, VGA_GREEN1, VGA_GREEN0, VGA_BLUE1, VGA_BLUE0, VGA_HSYNC, VGA_VSYNC);
	#endif
	DisplayController.setResolution(QVGA_320x240_60Hz);
	DisplayController.enableBackgroundPrimitiveExecution(false);

	for (int i = 0;i < PALETTE_SIZE;i++) {
		pColours[i].R = (i & 0x02) ? 0xFF:0x00;
		pColours[i].G = (i & 0x04) ? 0xFF:0x00;
		pColours[i].B = (i & 0x01) ? 0xFF:0x00;
	 	rawPixels[i] = DisplayController.createRawPixel(RGB222(pColours[i].R>>6,pColours[i].G>>6,pColours[i].B>>6));
	}

	for (int x = 0;x < 320;x++)
		for (int y = 0;y < 240;y++)
			DisplayController.setRawPixel(x,y,rawPixels[0]);

}


void HWXWriteDisplay(WORD16 pos,BYTE8 ch,BYTE8 colour) { 
	pos = pos & 0x3FF;
	if (pos >= 32*24) return;
	int x = (pos & 0x1F) * 8 + XOFFSET;
	int y = (pos >> 5) * 8 + YOFFSET;
 	BYTE8 xr = (ch & 0x80) ? 0xFF:0x00;
 	BYTE8 bgr = (colour >> 4) & 7;	
 	int fontAddr = (ch & 0x7F)*8 + 0x2C00;
 	for (int y1 = 0;y1 < 8;y1++) {
 		BYTE8 b = CPUReadMemory(fontAddr+y1) ^ xr;
 		for (int x1 = 0;x1 < 8;x1++) {
 			DisplayController.setRawPixel(x+x1,y+y1,rawPixels[(b & 0x80) ? colour & 7:bgr]);
 			b = b << 1;
 		}
 	}
}
