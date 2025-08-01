/*
======================================================================
AmiGUS Mixer Utility (Kick 1.3)
Copyright (C) 2025 by Oliver Achten
======================================================================

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

//#define DEBUG

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/interrupts.h>

#include <graphics/gfxbase.h>

#include <libraries/configvars.h>
#include <libraries/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/expansion_protos.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>

#include <hardware/intbits.h>

#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)    { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

#define DIV_ROUND_CLOSEST(n, d) ((d) == 0) ? 0 : (((n) + (d)/2)/(d))

#define	FALSE	0
#define TRUE	1
#define BOOL	int

/* AmiGUS Zorro IDs */

#define	AMIGUS_MAIN_PRODUCT_ID	16
#define AMIGUS_MANUFACTURER_ID	2782

/* AmiGUS Main Register Definitions */

#define	MAIN_INTC			0x00
#define MAIN_INTE			0x02
#define MAIN_SMPL_FMT		0x04
#define MAIN_SMPL_RATE		0x06
#define MAIN_FIFO_RES		0x08
#define MAIN_FIFO_WTMK		0x0a
#define MAIN_FIFO_DATA		0x0c
#define MAIN_FIFO_USE		0x10
#define MAIN_SPI_ADDRESS	0x20
#define MAIN_SPI_WDATA		0x22
#define MAIN_SPI_WTRIG		0x24
#define MAIN_SPI_RDTRIG		0x26
#define MAIN_SPI_RDATA		0x28
#define MAIN_SPI_STATUS		0x2a

#define MAIN_ADC_VOLUME_LL	0x30
#define MAIN_ADC_VOLUME_RR	0x32
#define MAIN_MHI_VOLUME_LL	0x34
#define MAIN_MHI_VOLUME_RR	0x36
#define MAIN_WAV_VOLUME_LL	0x38
#define MAIN_WAV_VOLUME_RR	0x3a
#define MAIN_AHI_VOLUME_LL	0x3c
#define MAIN_AHI_VOLUME_RR	0x3e

#define MAIN_ADC_MIX_LR		0x40
#define MAIN_MHI_MIX_LR		0x42
#define MAIN_WAV_MIX_LR		0x44
#define MAIN_AHI_MIX_LR		0x46

#define MAIN_LEVELS_MODE	0x48
#define MAIN_LEVELS_RESET	0x4a
#define MAIN_LEVELS_LEFT	0x4c
#define MAIN_LEVELS_RIGHT	0x4e

#define	FLASH_DATA_ADDR			0x50
#define	FLASH_DATA_WRITE_PORT	0x52
#define	FLASH_DATA_WRITE_STROBE	0x56
#define FLASH_DATA_READ_STROBE	0x58
#define	FLASH_DATA_READ_PORT	0x5a
#define	FLASH_DATA_STATUS		0x5e

#define	FLASH_CTRL_ADDR			0x60
#define	FLASH_CTRL_WRITE_DATA	0x62
#define	FLASH_CTRL_WRITE_STROBE	0x66
#define	FLASH_CTRL_READ_STROBE	0x68
#define	FLASH_CTRL_READ_DATA	0x6a
#define	FLASH_CONFIG_STATUS		0x6e

#define	MAIN_TOSLINK_CTRL		0x70

/* Gadget Definitions */

#define MCMD_NONE	0
#define	MCMD_SAVE	1
#define	MCMD_USE	2
#define	MCMD_RESET	3


#define MYGAD_SLIDER_AHI_L    	(0)
#define MYGAD_SLIDER_AHI_R    	(1)
#define MYGAD_SLIDER_AHI_MIX    (2)

#define MYGAD_SLIDER_MHI_L    	(3)
#define MYGAD_SLIDER_MHI_R    	(4)
#define MYGAD_SLIDER_MHI_MIX    (5)

#define MYGAD_SLIDER_HGN_L    	(6)
#define MYGAD_SLIDER_HGN_R    	(7)
#define MYGAD_SLIDER_HGN_MIX    (8)

#define MYGAD_SLIDER_ADC_L    	(9)
#define MYGAD_SLIDER_ADC_R    	(10)
#define MYGAD_SLIDER_ADC_MIX    (11)

#define MYGAD_CHKBOX_PAULA		(12)
#define MYGAD_CHKBOX_CDROM		(13)
#define MYGAD_CHKBOX_LINE		(14)

#define MYGAD_CHKBOX_AHI		(15)
#define MYGAD_CHKBOX_MHI		(16)
#define MYGAD_CHKBOX_HGN		(17)
#define MYGAD_CHKBOX_ADC		(18)

#define MYGAD_BUTTON_SAVE    	(19)
#define MYGAD_BUTTON_USE    	(20)
#define MYGAD_BUTTON_RESET    	(21)

#define MYGAD_CYCLE_LEVELS		(22)
#define MYGAD_CYCLE_TOSLINK		(23)

#define	FLASH_CONFIG_START		0x4000
#define	FLASH_CONFIG_SIZE		0x4000

struct Library      *IntuitionBase;
struct Library      *ExpansionBase;
struct Library      *GfxBase;
struct Library      *GadToolsBase;
struct Library      *AslBase;

extern void intServer();

short myBorderData[] =
{
  0,0, 84,0, 84,96, 0,96, 0,0,50,0,50,96,
};

short myBorderData2[] =
{
  0,0, 168,0, 168,96, 0,96, 0,0,50,0,50,96,84,96,84,0
};

short myBorderData3[] =
{
  0,0, 84,0, 84,17, 0,17, 0,0,
};

short myBorderData4[] =
{
  0,0, 168,0, 168,17, 0,17, 0,0,
};

short myBorderData5[] =
{
  0,0, 48,0, 48,116, 0,116, 0,0,
};

short myBorderData6[] =
{
  0,0, 16,0, 16,96, 0,96, 0,0,
};

struct mixData {
	UWORD	ahi_vol_ll;
	UWORD	ahi_vol_rr;
	UWORD	mhi_vol_ll;
	UWORD	mhi_vol_rr;
	UWORD	wav_vol_ll;
	UWORD	wav_vol_rr;
	UWORD	adc_vol_ll;
	UWORD	adc_vol_rr;

	UWORD	adc_mix_lr;
	UWORD	mhi_mix_lr;
	UWORD	wav_mix_lr;
	UWORD	ahi_mix_lr;
	
	UWORD	toslink_srate;

	UBYTE	adc_enable;
	
	BOOL	ahi_locked;
	BOOL	mhi_locked;
	BOOL	wav_locked;
	BOOL	adc_locked;
};

struct intData {
	ULONG	int_rate;
	ULONG	counter;
	ULONG	rd_Signal;
	struct 	Task *rd_Task;
};

void WriteReg16(APTR base, ULONG offset, UWORD val)
{
#ifndef DEBUG
	*((UWORD *)((ULONG)base+offset)) = val;
#endif	
}

void WriteReg32(APTR base, ULONG offset, ULONG val)
{
#ifndef DEBUG
	*((ULONG *)((ULONG)base+offset)) = val;
#endif
}

UWORD ReadReg16(APTR base, ULONG offset)
{
#ifndef DEBUG	
	return *((UWORD *)((ULONG)base+offset));
#else
	return 0;
#endif
}

WORD ReadReg16S(APTR base, ULONG offset)
{
#ifndef DEBUG	
	return *((WORD *)((ULONG)base+offset));
#else
	return 0;
#endif
}

ULONG ReadReg32(APTR base, ULONG offset)
{
#ifndef DEBUG
	return *((ULONG *)((ULONG)base+offset));
#else
	return 0;
#endif
}

void WriteSPI(APTR base, UWORD regNum, UWORD regVal)
{
	while ((ReadReg16(base,MAIN_SPI_STATUS)&0x8000)!=0){};
	WriteReg16(base,MAIN_SPI_ADDRESS,regNum);
	WriteReg16(base,MAIN_SPI_WDATA,regVal);
	WriteReg16(base,MAIN_SPI_WTRIG,0);
}

UWORD ReadSPI(APTR base, UWORD regNum)
{
	UWORD regVal;
	while ((ReadReg16(base,MAIN_SPI_STATUS)&0x8000)!=0){};	
	WriteReg16(base,MAIN_SPI_ADDRESS,regNum);
	WriteReg16(base,MAIN_SPI_RDTRIG,0);
	while ((ReadReg16(base,MAIN_SPI_STATUS)&0x8000)!=0){};	
	regVal = ReadReg16(base,MAIN_SPI_RDATA);
	return regVal;
}


void SetMixer(APTR base,struct mixData *mixDat)
{
	WriteReg16(base,MAIN_ADC_VOLUME_LL,mixDat->adc_vol_ll);
	WriteReg16(base,MAIN_ADC_VOLUME_RR,mixDat->adc_vol_rr);
	WriteReg16(base,MAIN_MHI_VOLUME_LL,mixDat->mhi_vol_ll);
	WriteReg16(base,MAIN_MHI_VOLUME_RR,mixDat->mhi_vol_rr);
	WriteReg16(base,MAIN_WAV_VOLUME_LL,mixDat->wav_vol_ll);
	WriteReg16(base,MAIN_WAV_VOLUME_RR,mixDat->wav_vol_rr);
	WriteReg16(base,MAIN_AHI_VOLUME_LL,mixDat->ahi_vol_ll);
	WriteReg16(base,MAIN_AHI_VOLUME_RR,mixDat->ahi_vol_rr);
	
	WriteReg16(base,MAIN_ADC_MIX_LR,mixDat->adc_mix_lr);
	WriteReg16(base,MAIN_MHI_MIX_LR,mixDat->mhi_mix_lr);
	WriteReg16(base,MAIN_WAV_MIX_LR,mixDat->wav_mix_lr);
	WriteReg16(base,MAIN_AHI_MIX_LR,mixDat->ahi_mix_lr);

	WriteReg16(base,MAIN_TOSLINK_CTRL,mixDat->toslink_srate);
	
	WriteSPI(base, 0x06, (UWORD)mixDat->adc_enable);	// Enable PAULA Left
	WriteSPI(base, 0x07, (UWORD)mixDat->adc_enable);	// Enable PAULA Right
}


void EraseFlash(APTR base)
{	
	ULONG status,sector;
	
	status = 0xffffffff;
	sector = 0x00200000;
	
	WriteReg16(base, FLASH_CTRL_ADDR, 0x1);
	WriteReg32(base, FLASH_CTRL_WRITE_DATA, 0x0);				// Clear protection bits
	WriteReg16(base, FLASH_CTRL_WRITE_STROBE, 0x0);
		
	WriteReg32(base, FLASH_CTRL_WRITE_DATA, sector);	// Sector erase
	WriteReg16(base, FLASH_CTRL_WRITE_STROBE, 0x0);
	WriteReg16(base, FLASH_CTRL_ADDR, 0x0);						
		
	do {
		WriteReg16(base, FLASH_CTRL_READ_STROBE, 0x0);	
		
		status = (ULONG)(ReadReg32(base, FLASH_CTRL_READ_DATA) & (ULONG)0x3);	// Read Status register
		
	} while (status != 0x0);
}

void ProgramFlash(APTR base, APTR memory)
{
	ULONG	memData,flashOffset,status,cnt,length;
	
	flashOffset = (ULONG)(FLASH_CONFIG_START>>2);
	length = FLASH_CONFIG_SIZE;
	
	status = 0xffffffff;
	cnt = 0x0;
	
	WriteReg16(base, FLASH_CTRL_ADDR, 0x1);
	WriteReg32(base, FLASH_CTRL_WRITE_DATA, 0x0);				// Clear protection bits
	WriteReg16(base, FLASH_CTRL_WRITE_STROBE, 0x0);
	
	WriteReg16(base, FLASH_CTRL_ADDR, 0x0);
	
	do {
		memData = *((ULONG *)((ULONG)memory+cnt));
		
		WriteReg16(base, FLASH_DATA_ADDR, flashOffset);
		WriteReg32(base, FLASH_DATA_WRITE_PORT, memData);
		WriteReg16(base, FLASH_DATA_WRITE_STROBE, 0x0);
		
		do {
			WriteReg16(base, FLASH_CTRL_READ_STROBE, 0x0);	
		
			status = (ULONG)(ReadReg32(base, FLASH_CTRL_READ_DATA) & (ULONG)0x3);	// Read Status register
		
		} while (status != 0x0);		
		
		
		flashOffset++;
		cnt+=4;
	}
	while (cnt < length);
}

void InitCFGMem (APTR cfgMem)
{
	ULONG cnt = 0;
		
	do {
		*((ULONG *)((ULONG)cfgMem+cnt)) = 0xffffffff;
		cnt+=4;
	} while (cnt != 0x4000);
		
	*((ULONG *)((ULONG)cfgMem+0x0000)) = 0x414d4947;	// Magic Token - Unlock

/* Fix ADC Initialisation */

	// ADC Reset Registers
	*((ULONG *)((ULONG)cfgMem+0x0004)) = 0x00000020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0008)) = 0x00fe0022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x000c)) = 0x00000024;	// MAIN_SPI_WTRIG	
	
	// ADC Power-Down
	*((ULONG *)((ULONG)cfgMem+0x0010)) = 0x00700020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0014)) = 0x00750022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x0018)) = 0x00000024;	// MAIN_SPI_WTRIG	
		
	//  Set Manual Gain Control

	*((ULONG *)((ULONG)cfgMem+0x001c)) = 0x00190020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0020)) = 0x00ff0022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x0024)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Increase Left Gain
		
	*((ULONG *)((ULONG)cfgMem+0x0028)) = 0x00010020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x002c)) = 0x00200022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x0030)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Increase Right Gain
		
	*((ULONG *)((ULONG)cfgMem+0x0034)) = 0x00020020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0038)) = 0x00200022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x003c)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Enable Left Inputs
		
	*((ULONG *)((ULONG)cfgMem+0x0040)) = 0x00060020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00020022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x0048)) = 0x00000024;	// MAIN_SPI_WTRIG		
		
	// Enable Right Inputs
		
	*((ULONG *)((ULONG)cfgMem+0x004c)) = 0x00070020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00020022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x0054)) = 0x00000024;	// MAIN_SPI_WTRIG			
		
/* Mixer Settings */
	
	*((ULONG *)((ULONG)cfgMem+0x0058)) = 0x80000030;	// MAIN_ADC_VOLUME_LL
	*((ULONG *)((ULONG)cfgMem+0x005c)) = 0x80000032;	// MAIN_ADC_VOLUME_RR
	*((ULONG *)((ULONG)cfgMem+0x0060)) = 0x80000034;	// MAIN_MHI_VOLUME_LL
	*((ULONG *)((ULONG)cfgMem+0x0064)) = 0x80000036;	// MAIN_MHI_VOLUME_RR	
	*((ULONG *)((ULONG)cfgMem+0x0068)) = 0x80000038;	// MAIN_WAV_VOLUME_LL
	*((ULONG *)((ULONG)cfgMem+0x006c)) = 0x8000003a;	// MAIN_WAV_VOLUME_RR
	*((ULONG *)((ULONG)cfgMem+0x0070)) = 0x8000003c;	// MAIN_AHI_VOLUME_LL
	*((ULONG *)((ULONG)cfgMem+0x0074)) = 0x8000003e;	// MAIN_AHI_VOLUME_RR

	*((ULONG *)((ULONG)cfgMem+0x0078)) = 0x00000040;	// MAIN_ADC_MIX_LR
	*((ULONG *)((ULONG)cfgMem+0x007c)) = 0x00000042;	// MAIN_MHI_MIX_LR
	*((ULONG *)((ULONG)cfgMem+0x0080)) = 0x00000044;	// MAIN_WAV_MIX_LR
	*((ULONG *)((ULONG)cfgMem+0x0084)) = 0x00000046;	// MAIN_AHI_MIX_LR

/* TOSLINK Settings */

	*((ULONG *)((ULONG)cfgMem+0x0088)) = 0x00000070;	// MAIN_TOSLINK_CTRL

	// Enable ADC I2S Master Mode
	*((ULONG *)((ULONG)cfgMem+0x008c)) = 0x00200020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x0090)) = 0x00900022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x0094)) = 0x00000024;	// MAIN_SPI_WTRIG

	// Set DSP1 CLOCK
	*((ULONG *)((ULONG)cfgMem+0x0098)) = 0x00210020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x009c)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x00a0)) = 0x00000024;	// MAIN_SPI_WTRIG
	
	// Set DSP2 CLOCK
	*((ULONG *)((ULONG)cfgMem+0x00a4)) = 0x00220020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x00a8)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x00ac)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// Set ADC CLOCK
	*((ULONG *)((ULONG)cfgMem+0x00b0)) = 0x00230020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x00b4)) = 0x00070022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x00b8)) = 0x00000024;	// MAIN_SPI_WTRIG

	// Set BCLK = CLK/4 (192kHz sampling rate)
	*((ULONG *)((ULONG)cfgMem+0x00bc)) = 0x00260020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x00c0)) = 0x00030022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x00c4)) = 0x00000024;	// MAIN_SPI_WTRIG
	
	// Disable PLL
	*((ULONG *)((ULONG)cfgMem+0x00c8)) = 0x00280020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x00cc)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x00d0)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// ADC Power-Up
	*((ULONG *)((ULONG)cfgMem+0x00d4)) = 0x00700020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfgMem+0x00d8)) = 0x00700022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfgMem+0x00dc)) = 0x00000024;	// MAIN_SPI_WTRIG	

/* End of Stream */
	*((ULONG *)((ULONG)cfgMem+0x00e0)) = 0xffffffff;
}


struct NewWindow mainWindow = {
	0,0,528,170,				
	2,1,					
	REFRESHWINDOW | MOUSEMOVE | GADGETUP | GADGETDOWN | CLOSEWINDOW,	
	ACTIVATE | WINDOWDRAG | WINDOWDEPTH | SMART_REFRESH | WINDOWCLOSE,
	0,			
	0,					
	"AmiGUS Mixer V0.65 - (c)2025 by O. Achten",
	0,
	0,
	0,0,0,0,
	WBENCHSCREEN
};

struct Window *myWin;
struct Screen myScreen;
struct RastPort *rp;
struct TextFont *font;
struct TextAttr mainFont = { "topaz.font", 8, 0, 0, };

struct chkbox {
	struct Gadget gad;
	int on;
	int x, y, w, h;
	char *text;
};

struct button {
	struct Gadget gad;
	int on;
	int x, y, w, h;
	char *text;
};

struct cycle {
	struct Gadget gad;
	int on;
	int x, y, w, h;
	int numvals, val;
	char *text;
};

struct slider {
	struct Gadget gad;
	struct PropInfo prop;
	int x, y, w, h;
	int max,val,oldval;
	char *text;
};

struct IntuiText button_text = {
	1, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	4, 3,				/* leftedge,topedge */
	&mainFont,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct IntuiText cycle_text = {
	1, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	24, 3,				/* leftedge,topedge */
	&mainFont,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct IntuiText chkbox_text = {
	2, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	4, 3,				/* leftedge,topedge */
	&mainFont,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct IntuiText slider_text = {
	2, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	4, 3,				/* leftedge,topedge */
	&mainFont,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

USHORT __chip change_image_data[] = {
	0x0000, 0x0000,
	0x07f0, 0x2000,
	0x0c18, 0x2000,
	0x0c18, 0x2000,
	0x0c7e, 0x2000,
	0x0c3c, 0x2000,
	0x0c18, 0x2000,
	0x0c00, 0x2000,
	0x0c18, 0x2000,
	0x07f0, 0x2000,
	0x0000, 0x2000,

	0x0000, 0x0000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000
};

USHORT __chip chkbox_image_data[] = {
	0x0007,
	0x000c,
	0x0018,	
	0x0030,
	0x0060,
	0x00c0,
	0xe180,
	0x7300,
	0x3e00,
	0x1c00,
	0x0000,
	
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000, 
	0x0000,
	0x0000,
};

USHORT __chip slider_image_data[] = {
	0x0100,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300, 
	0x0300, 	
	0x7f00,

	0xfe00,
	0xc000,
	0xc000,	
	0xc000,
	0xc000,
	0xc000,
	0xc000,
	0xc000,
	0xc000,
	0xc000,	
	0x8000,

};

USHORT __chip slider_image_data13[] = {
	0xfe00,
	0xd400,
	0xe800,	
	0xd400,
	0xe800,	
	0xd400,
	0xe800,	
	0xd400,
	0xe800,	
	0xd400,
	0x8000,

	0x0100,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,
	0x0300,	
	0x0300, 
	0x7f00,
};

struct Image change_box_image = {
	0, 0,			/* upper left corner */
	21, 11, 2,		/* width, height, depth */
	&change_image_data[0],	/* data ptr */
	0x3, 0x0,		/* planepick, planeonoff */
	NULL			/* nextimage */
};

struct Image check_box_image = {
	2, 0,			/* upper left corner */
	16, 11, 2,		/* width, height, depth */
	&chkbox_image_data[0],	/* data ptr */
	0x3, 0x0,		/* planepick, planeonoff */
	NULL			/* nextimage */
};

struct Image slider_image = {
	0, 0,			/* upper left corner */
	8, 11, 2,		/* width, height, depth */
	&slider_image_data[0],	/* data ptr */
	0x3, 0x0,		/* planepick, planeonoff */
	NULL			/* nextimage */
};

struct Image slider_image13 = {
	0, 0,			/* upper left corner */
	8, 11, 2,		/* width, height, depth */
	&slider_image_data13[0],	/* data ptr */
	0x3, 0x0,		/* planepick, planeonoff */
	NULL			/* nextimage */
};

struct button button_save,button_use,button_reset;
struct cycle cycle_toslink,cycle_levels;
struct chkbox box_paula,box_cdrom,box_line,box_ahi,box_mhi,box_hgn,box_adc;

struct slider slider_ahi_l,slider_ahi_r,slider_ahi_mix;
struct slider slider_mhi_l,slider_mhi_r,slider_mhi_mix;
struct slider slider_hgn_l,slider_hgn_r,slider_hgn_mix;
struct slider slider_adc_l,slider_adc_r,slider_adc_mix;

BOOL	kick13;

int 	wbscr_barheight;
int 	wbscr_width;
int 	wbscr_height;
UWORD	topBorder;

void DrawLine(struct RastPort *rp, int x1, int y1, int x2, int y2)
{
	Move(rp, x1, y1);
	Draw(rp, x2, y2);
}

void createSlider(struct slider *b, int gadid, char *text, int x, int y, int w, int h, int max, int val)
{
	/* init button junk */
	b->x = x;
	b->y = y;
	b->w = w;
	b->h = h;
	b->text = text;
	b->max = max;
	b->val = val;

	/* init gadget */
	b->gad.NextGadget = 0;
	b->gad.Flags = GADGHCOMP;
	b->gad.Activation = RELVERIFY;
	b->gad.GadgetType = PROPGADGET;
	
	if (kick13 == TRUE)
		b->gad.GadgetRender = (APTR)&slider_image13;
	else
		b->gad.GadgetRender = (APTR)&slider_image;			
	
	b->gad.SelectRender = 0;
	b->gad.GadgetText = 0;
	b->gad.MutualExclude = 0;
	b->gad.SpecialInfo = (APTR)&b->prop;
	b->gad.GadgetID = gadid;
	b->gad.UserData = (APTR)b;

	/* gadget's select box is the box */
	b->gad.LeftEdge = x + ((w-8)>>1);
	b->gad.TopEdge = b->y;
	b->gad.Width = 8;
	b->gad.Height = b->h;
	
	b->prop.Flags = FREEVERT|PROPBORDERLESS;
	b->prop.HorizPot = 0;
	b->prop.VertPot = 0xffff-((b->val * 0xffff) / b->max);
	b->prop.HorizBody = 0xffff / b->max;
	b->prop.VertBody = 0;
}

void drawSliderValue(struct slider *b)
{
	int x, y, wid, h, width;
	char cval[5];
	int val;
	
	x = b->x;
	y = b->y-2;
	wid = b->w;
	h = b->h+4;

	sprintf (cval,"%d",b->max);
	slider_text.IText = cval;
	width = wid - 2 * slider_text.LeftEdge;
	
	SetAPen(rp, 0);	
	RectFill(rp, (width - IntuiTextLength(&slider_text)) / 2 + x + 4, y+h, ((width - IntuiTextLength(&slider_text)) / 2) + x + IntuiTextLength(&slider_text) + 4, y+h+9);
	
	val = DIV_ROUND_CLOSEST(((0xFFFF - b->prop.VertPot) * b->max),0xffff);
	b->val = val;

	sprintf (cval,"%d",b->val);
	button_text.IText = cval;

	SetAPen(rp, 3);		
	PrintIText(rp, &button_text,
		(width - IntuiTextLength(&button_text)) / 2 + x,
		y+h);

}

void setSliderValue(struct slider *b,int val)
{
	RemoveGadget(myWin, &b->gad);
	
	b->val = val;
	b->prop.VertPot = 0xffff-((b->val * 0xffff) / b->max);		
//	AddGadget(myWin, &b->gad, 0);

	AddGadget(myWin, &b->gad,-1);
	
	NewModifyProp(&b->gad,myWin,NULL,FREEVERT|PROPBORDERLESS,0,0xffff - ((b->val * 0xffff) / b->max),0,0xffff / b->max,-1);	
	RefreshGadgets(&b->gad,myWin,NULL);
	
	drawSliderValue(b);	
	
}

void drawSlider(struct slider *b)
{
	int x, y, wid, h, width;
	
	long myShadowPen;
	long myShinePen;
	
	if (kick13 == TRUE)
	{
		myShadowPen = 2;
		myShinePen  = 1;
	}
	else
	{
		myShadowPen = 1;
		myShinePen  = 2;
	}

	x = b->x;
	y = b->y-2;
	wid = b->w;
	h = b->h+4;

	SetDrMd(rp, JAM1);

	SetAPen(rp, 0);
	RectFill(rp, x, y, x+wid-1, y+h-1);

	SetAPen(rp, myShinePen);
	DrawLine(rp, x, y, x+wid-1, y);
	DrawLine(rp, x, y, x, y+h-1);
	DrawLine(rp, x+1, y+1, x+1, y+h-2);
	SetAPen(rp, myShadowPen);
	DrawLine(rp, x+1, y+h-1, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-1, y, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-2, y+1, x+wid-2, y+h-1);

	slider_text.IText = b->text;
	width = wid - 2 * slider_text.LeftEdge;
	PrintIText(rp, &slider_text,
		(width - IntuiTextLength(&slider_text)) / 2 + x,
		y-14);
	
	setSliderValue(b,b->val);

		
}

void createChkbox(struct chkbox *b, int gadid, char *text, int x, int y, int on)
{
	/* init button junk */
	b->x = x;
	b->y = y;
	b->w = 24;
	b->h = 1 + 1 + 8 + 2 + 1;
	b->text = text;
	b->on = on;

	/* init gadget */
	b->gad.NextGadget = 0;
	b->gad.Flags = GADGHNONE;
	b->gad.Activation = RELVERIFY;
	b->gad.GadgetType = BOOLGADGET;
	b->gad.GadgetRender = 0;
	b->gad.SelectRender = 0;
	b->gad.GadgetText = 0;
	b->gad.MutualExclude = 0;
	b->gad.SpecialInfo = 0;
	b->gad.GadgetID = gadid;
	b->gad.UserData = (APTR)b;

	/* gadget's select box is the box */
	b->gad.LeftEdge = b->x;
	b->gad.TopEdge = b->y;
	b->gad.Width = b->w;
	b->gad.Height = b->h;
}

void drawChkbox(struct chkbox *b)
{
	int x, y, wid, h, width;
	
	long myShadowPen;
	long myShinePen;
	
	if (kick13 == TRUE)
	{
		myShadowPen = 2;
		myShinePen  = 1;
	}
	else
	{
		myShadowPen = 1;
		myShinePen  = 2;
	}

	RemoveGadget(myWin, &b->gad);

	x = b->x;
	y = b->y;
	wid = b->w;
	h = b->h;

	SetDrMd(rp, JAM1);

	SetAPen(rp, 0);
	RectFill(rp, x, y, x+wid-1, y+h-1);

	SetAPen(rp, myShinePen);
	DrawLine(rp, x, y, x+wid-1, y);
	DrawLine(rp, x, y, x, y+h-1);
	DrawLine(rp, x+1, y+1, x+1, y+h-2);
	SetAPen(rp, myShadowPen);
	DrawLine(rp, x+1, y+h-1, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-1, y, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-2, y+1, x+wid-2, y+h-1);

	if (b->on != 0)
		DrawImage(rp, &check_box_image, 2 + x, 1 + y);

	chkbox_text.IText = b->text;
	width = wid - 2 * chkbox_text.LeftEdge;
	PrintIText(rp, &chkbox_text,
		(width - IntuiTextLength(&chkbox_text)) / 2 + x + 40,
		y);

	AddGadget(myWin, &b->gad, 0);
}


void createCycle(struct cycle *b, int gadid, char *text, int x, int y, int w, int numvals, int val)
{
	/* init button junk */
	b->x = x;
	b->y = y;
	b->w = w;
	b->h = 1 + 1 + 8 + 2 + 1;
	b->text = text;

	/* init gadget */
	b->gad.NextGadget = 0;
	b->gad.Flags = GADGHCOMP;
	b->gad.Activation = RELVERIFY;
	b->gad.GadgetType = BOOLGADGET;
	b->gad.GadgetRender = 0;
	b->gad.SelectRender = 0;
	b->gad.GadgetText = 0;
	b->gad.MutualExclude = 0;
	b->gad.SpecialInfo = 0;
	b->gad.GadgetID = gadid;
	b->gad.UserData = (APTR)b;

	/* gadget's select box is the box */
	b->gad.LeftEdge = b->x;
	b->gad.TopEdge = b->y;
	b->gad.Width = b->w;
	b->gad.Height = b->h;
	
	b->numvals = numvals;
	b->val = val;
}

void drawCycle(struct cycle *b)
{
	int x, y, wid, h, width;
	
	long myShadowPen;
	long myShinePen;
	
	if (kick13 == TRUE)
	{
		myShadowPen = 2;
		myShinePen  = 1;
	}
	else
	{
		myShadowPen = 1;
		myShinePen  = 2;
	}

	RemoveGadget(myWin, &b->gad);

	x = b->x;
	y = b->y;
	wid = b->w;
	h = b->h;

	SetDrMd(rp, JAM1);

	SetAPen(rp, 0);
	RectFill(rp, x, y, x+wid-1, y+h-1);

	DrawImage(rp, &change_box_image, 2 + x, 1 + y);

	cycle_text.IText = b->text;
	width = wid - cycle_text.LeftEdge - 4;
	PrintIText(rp, &cycle_text,
		(width - IntuiTextLength(&cycle_text)) / 2 + x,
		y);

	SetAPen(rp, myShinePen);
	DrawLine(rp, x, y, x+wid-1, y);
	DrawLine(rp, x, y, x, y+h-1);
	DrawLine(rp, x+1, y+1, x+1, y+h-2);
	SetAPen(rp, myShadowPen);
	DrawLine(rp, x+1, y+h-1, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-1, y, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-2, y+1, x+wid-2, y+h-1);

	AddGadget(myWin, &b->gad, -1);
}

void createButton(struct button *b, int gadid, char *text, int x, int y, int w)
{
	/* init button junk */
	b->x = x;
	b->y = y;
	b->w = w;
	b->h = 1 + 1 + 8 + 2 + 1;
	b->text = text;

	/* init gadget */
	b->gad.NextGadget = 0;
	b->gad.Flags = GADGHCOMP;
	b->gad.Activation = RELVERIFY;
	b->gad.GadgetType = BOOLGADGET;
	b->gad.GadgetRender = 0;
	b->gad.SelectRender = 0;
	b->gad.GadgetText = 0;
	b->gad.MutualExclude = 0;
	b->gad.SpecialInfo = 0;
	b->gad.GadgetID = gadid;
	b->gad.UserData = (APTR)b;

	/* gadget's select box is the box */
	b->gad.LeftEdge = b->x;
	b->gad.TopEdge = b->y;
	b->gad.Width = b->w;
	b->gad.Height = b->h;
}

void drawButton(struct button *b)
{
	int x, y, wid, h, width;
	
	long myShadowPen;
	long myShinePen;
	
	if (kick13 == TRUE)
	{
		myShadowPen = 2;
		myShinePen  = 1;
	}
	else
	{
		myShadowPen = 1;
		myShinePen  = 2;
	}

	RemoveGadget(myWin, &b->gad);

	x = b->x;
	y = b->y;
	wid = b->w;
	h = b->h;

	SetDrMd(rp, JAM1);

	SetAPen(rp, 0);
	RectFill(rp, x, y, x+wid-1, y+h-1);

	SetAPen(rp, myShinePen);
	DrawLine(rp, x, y, x+wid-1, y);
	DrawLine(rp, x, y, x, y+h-1);
	DrawLine(rp, x+1, y+1, x+1, y+h-2);
	SetAPen(rp, myShadowPen);
	DrawLine(rp, x+1, y+h-1, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-1, y, x+wid-1, y+h-1);
	DrawLine(rp, x+wid-2, y+1, x+wid-2, y+h-1);

	button_text.IText = b->text;
	width = wid - 2 * button_text.LeftEdge;
	PrintIText(rp, &button_text,
		(width - IntuiTextLength(&button_text)) / 2 + x,
		y);

	AddGadget(myWin, &b->gad, -1);
}

void drawBorders(struct RastPort *rp,UWORD topBorder,struct TextFont *font)
{
	struct Border	shineBorder;
	struct Border	shadowBorder;

	struct IntuiText	myIText;
	struct TextAttr		myTextAttr;

	long myShadowPen ;
	long myShinePen;

	long myTextPen = 1;
	long myBackgroundPen = 0;

	int startX = 12;

	if (kick13 == TRUE)
	{
		myShadowPen = 2;
		myShinePen  = 1;
	}
	else
	{
		myShadowPen = 1;
		myShinePen  = 2;
	}

	shadowBorder.LeftEdge   = 0;
	shadowBorder.TopEdge    = 0;
	shadowBorder.FrontPen   = myShadowPen;
	shadowBorder.NextBorder = &shineBorder;

	shineBorder.LeftEdge    = 0 + 1;
	shineBorder.TopEdge     = 0 + 1;
	shineBorder.FrontPen    = myShinePen;
	shineBorder.NextBorder  = NULL;

	shadowBorder.BackPen    = shineBorder.BackPen   = 0;
	shadowBorder.DrawMode   = shineBorder.DrawMode  = JAM1;
	shadowBorder.Count      = shineBorder.Count     = 7;
	shadowBorder.XY         = shineBorder.XY        = myBorderData;

	DrawBorder(rp,&shadowBorder,startX,topBorder+14);
	startX +=92;
	DrawBorder(rp,&shadowBorder,startX,topBorder+14);
	startX +=92;
	DrawBorder(rp,&shadowBorder,startX,topBorder+14);
	startX +=92;
	shadowBorder.Count      = shineBorder.Count     = 9;
	shadowBorder.XY         = shineBorder.XY        = myBorderData2;
	DrawBorder(rp,&shadowBorder,startX,topBorder+14);
	shadowBorder.Count      = shineBorder.Count     = 5;
	startX +=176;
	shadowBorder.XY         = shineBorder.XY        = myBorderData5;
	DrawBorder(rp,&shadowBorder,startX,topBorder+14);
	startX +=4;
	shadowBorder.XY         = shineBorder.XY        = myBorderData6;
	DrawBorder(rp,&shadowBorder,startX,topBorder+30);
	startX +=24;
	shadowBorder.XY         = shineBorder.XY        = myBorderData6;
	DrawBorder(rp,&shadowBorder,startX,topBorder+30);
	shadowBorder.XY         = shineBorder.XY        = myBorderData3;
	startX =12;
	DrawBorder(rp,&shadowBorder,startX,topBorder+114);
	startX +=92;
	DrawBorder(rp,&shadowBorder,startX,topBorder+114);
	startX +=92;
	DrawBorder(rp,&shadowBorder,startX,topBorder+114);
	startX +=92;
	shadowBorder.XY         = shineBorder.XY        = myBorderData4;
	DrawBorder(rp,&shadowBorder,startX,topBorder+114);

	/* Text labels */

	myTextAttr.ta_Name  = font->tf_Message.mn_Node.ln_Name;
	myTextAttr.ta_YSize = font->tf_YSize;
	myTextAttr.ta_Style = font->tf_Style;
	myTextAttr.ta_Flags = font->tf_Flags;
	myIText.FrontPen    = myTextPen;
	myIText.BackPen     = myBackgroundPen;
	myIText.DrawMode    = JAM2;
	myIText.LeftEdge    = 0;
	myIText.TopEdge     = 0;
	myIText.ITextFont   = &myTextAttr;
	myIText.IText       = "AHI Sound";
	myIText.NextText    = NULL;
	
	startX = 20;
	PrintIText(rp,&myIText,startX,topBorder+4);
	startX +=92;
	myIText.IText       = "MHI Sound";
	PrintIText(rp,&myIText,startX,topBorder+4);
	startX +=88;
	myIText.IText       = "Wave Sound";
	PrintIText(rp,&myIText,startX,topBorder+4);
	startX +=118;
	myIText.IText       = "External Sound";
	PrintIText(rp,&myIText,startX,topBorder+4);
	startX +=146;
	myIText.IText       = "Levels";
	PrintIText(rp,&myIText,startX,topBorder+4);
	startX+=8;
	myIText.IText       = "L  R";
	PrintIText(rp,&myIText,startX,topBorder+20);
	startX-=82;
	myIText.IText       = "Inputs";
	PrintIText(rp,&myIText,startX,topBorder+20);
	startX-=4;
	myIText.IText       = "TOSLINK";
	PrintIText(rp,&myIText,startX,topBorder+84);	
	
}

void createAllGadgets(UWORD topBorder)
{
	int offsetx,offsety;

	offsetx   = 20;
	offsety   = 32+topBorder;

	createSlider(&slider_ahi_l, MYGAD_SLIDER_AHI_L, "L", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_ahi_l.gad, -1);
	drawSlider(&slider_ahi_l);

	offsetx  += 20;
	offsety  += 0;

	createSlider(&slider_ahi_r, MYGAD_SLIDER_AHI_R, "R", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_ahi_r.gad, -1);	
	drawSlider(&slider_ahi_r);
	
	offsetx  += 32;
	offsety   += 0;

	createSlider(&slider_ahi_mix, MYGAD_SLIDER_AHI_MIX, "Mix", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_ahi_mix.gad, -1);	
	drawSlider(&slider_ahi_mix);
	
	offsetx  += 40;
	offsety   += 0;

	createSlider(&slider_mhi_l, MYGAD_SLIDER_MHI_L, "L", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_mhi_l.gad, -1);
	drawSlider(&slider_mhi_l);

	offsetx  += 20;
	offsety   += 0;

	createSlider(&slider_mhi_r, MYGAD_SLIDER_MHI_R, "R", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_mhi_r.gad, -1);	
	drawSlider(&slider_mhi_r);
	
	offsetx  += 32;
	offsety   += 0;

	createSlider(&slider_mhi_mix, MYGAD_SLIDER_MHI_MIX, "Mix", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_mhi_mix.gad, -1);	
	drawSlider(&slider_mhi_mix);

	offsetx  += 40;
	offsety   += 0;

	createSlider(&slider_hgn_l, MYGAD_SLIDER_HGN_L, "L", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_hgn_l.gad, -1);
	drawSlider(&slider_hgn_l);

	offsetx  += 20;
	offsety   += 0;

	createSlider(&slider_hgn_r, MYGAD_SLIDER_HGN_R, "R", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_hgn_r.gad, -1);
	drawSlider(&slider_hgn_r);
	
	offsetx  += 32;
	offsety   += 0;

	createSlider(&slider_hgn_mix, MYGAD_SLIDER_HGN_MIX, "Mix", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_hgn_mix.gad, -1);
	drawSlider(&slider_hgn_mix);


	offsetx  += 40;
	offsety   += 0;

	createSlider(&slider_adc_l, MYGAD_SLIDER_ADC_L, "L", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_adc_l.gad, -1);
	drawSlider(&slider_adc_l);

	offsetx  += 20;
	offsety   += 0;

	createSlider(&slider_adc_r, MYGAD_SLIDER_ADC_R, "R", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_adc_r.gad, -1);
	drawSlider(&slider_adc_r);

	offsetx  += 32;
	offsety   += 0;

	createSlider(&slider_adc_mix, MYGAD_SLIDER_ADC_MIX, "Mix", offsetx,offsety, 16, 64, 99, 50);
	AddGadget(myWin, &slider_adc_mix.gad, -1);
	drawSlider(&slider_adc_mix);

	/* ========== Checkbox Gadgets =========== */
//box_paula,box_cdrom,box_line,box_ahi,box_mhi,box_hgn,box_adc;

	offsetx  += 30;
	offsety   += 0;

	createChkbox(&box_paula, MYGAD_CHKBOX_PAULA,"Paula", offsetx, offsety, 0);		
	AddGadget(myWin, &box_paula.gad, -1);
	drawChkbox(&box_paula);
						
	offsetx  += 0;
	offsety   += 16;

	createChkbox(&box_cdrom, MYGAD_CHKBOX_CDROM,"CDROM", offsetx, offsety, 0);		
	AddGadget(myWin, &box_cdrom.gad, -1);
	drawChkbox(&box_cdrom);
	
	offsetx  += 0;
	offsety   += 16;

	createChkbox(&box_line, MYGAD_CHKBOX_LINE,"Line", offsetx, offsety, 0);		
	AddGadget(myWin, &box_line.gad, -1);
	drawChkbox(&box_line);
	
	offsetx  = 24;
	offsety   = topBorder + 132-15;

	createChkbox(&box_ahi, MYGAD_CHKBOX_AHI,"Lock", offsetx, offsety, 0);		
	AddGadget(myWin, &box_ahi.gad, -1);
	drawChkbox(&box_ahi);

	offsetx  += 92;
	offsety   += 0;

	createChkbox(&box_mhi, MYGAD_CHKBOX_MHI,"Lock", offsetx, offsety, 0);		
	AddGadget(myWin, &box_mhi.gad, -1);
	drawChkbox(&box_mhi);
	
	offsetx  += 92;
	offsety   += 0;

	createChkbox(&box_hgn, MYGAD_CHKBOX_HGN,"Lock", offsetx, offsety, 0);		
	AddGadget(myWin, &box_hgn.gad, -1);
	drawChkbox(&box_hgn);
	
	offsetx  += 92;
	offsety   += 0;

	createChkbox(&box_adc, MYGAD_CHKBOX_ADC,"Lock", offsetx, offsety, 0);		
	AddGadget(myWin, &box_adc.gad, -1);
	drawChkbox(&box_adc);
	
	/* ========== Button Gadgets =========== */

	offsetx  = 98;
	offsety   += 20;
	
	createButton(&button_save, MYGAD_BUTTON_SAVE,"Save", offsetx, offsety, 72);		
	AddGadget(myWin, &button_save.gad, -1);
	drawButton(&button_save);
						
	offsetx  +=96;
	offsety   += 0;

	createButton(&button_use, MYGAD_BUTTON_USE,"Use", offsetx, offsety, 72);		
	AddGadget(myWin, &button_use.gad, -1);
	drawButton(&button_use);
						
	offsetx  += 96;
	offsety   += 0;

	createButton(&button_reset, MYGAD_BUTTON_RESET,"Reset", offsetx, offsety, 72);		
	AddGadget(myWin, &button_reset.gad, -1);
	drawButton(&button_reset);

	/* ========== Cycle Gadgets =========== */

	offsetx  += 166;
	offsety   += 0;

	createCycle(&cycle_levels, MYGAD_CYCLE_LEVELS,"SLOW", offsetx, offsety, 66,4,1);		
	AddGadget(myWin, &cycle_levels.gad, -1);
	drawCycle(&cycle_levels);		

	offsetx  -= 78;
	offsety   -= 42;

	createCycle(&cycle_toslink, MYGAD_CYCLE_TOSLINK,"48kHz", offsetx, offsety, 74,3,0);		
	AddGadget(myWin, &cycle_toslink.gad, -1);
	drawCycle(&cycle_toslink);			
}

UWORD handleGadgetEvent(struct Gadget *gad,struct mixData *mixDat,APTR boardBase,APTR cfgMem,struct intData *intData)
{
	int code;
	switch (gad->GadgetID)
	{	
		case MYGAD_BUTTON_SAVE:
			return MCMD_SAVE;
			break;
		case MYGAD_BUTTON_USE:
			return MCMD_USE;
			break;
		case MYGAD_BUTTON_RESET:
			return MCMD_RESET;
			break;		
		case MYGAD_SLIDER_AHI_L:
			drawSliderValue(&slider_ahi_l);
			code = slider_ahi_l.val;
			mixDat->ahi_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0070)) = 0x0000003c | (ULONG)((ULONG)mixDat->ahi_vol_ll << 16);	// MAIN_AHI_VOLUME_LL
			if (mixDat->ahi_locked == TRUE)
			{
				mixDat->ahi_vol_rr = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x0074)) = 0x0000003e | (ULONG)((ULONG)mixDat->ahi_vol_rr << 16);
				setSliderValue(&slider_ahi_r,code);
			}		
			break;		
		case MYGAD_SLIDER_AHI_R:
			drawSliderValue(&slider_ahi_r);
			code = slider_ahi_r.val;			
			mixDat->ahi_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0074)) = 0x0000003e | (ULONG)((ULONG)mixDat->ahi_vol_rr << 16);	// MAIN_AHI_VOLUME_RR
			
			if (mixDat->ahi_locked == TRUE)
			{
				mixDat->ahi_vol_ll = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x0070)) = 0x0000003c | (ULONG)((ULONG)mixDat->ahi_vol_ll << 16);
				setSliderValue(&slider_ahi_l,code);
			}	
			break;		
		case MYGAD_SLIDER_AHI_MIX:
			drawSliderValue(&slider_ahi_mix);
			code = slider_ahi_mix.val;			
			mixDat->ahi_mix_lr = (UWORD)((66196*code)/100);		
			*((ULONG *)((ULONG)cfgMem+0x0084)) = 0x00000046 | (ULONG)((ULONG)mixDat->ahi_mix_lr << 16);	// MAIN_AHI_MIX_LR		
			break;
		case MYGAD_SLIDER_MHI_L:
			drawSliderValue(&slider_mhi_l);
			code = slider_mhi_l.val;			
			mixDat->mhi_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0060)) = 0x00000034 | (ULONG)((ULONG)mixDat->mhi_vol_ll << 16);	// MAIN_MHI_VOLUME_LL
			
			if (mixDat->mhi_locked == TRUE)
			{
				mixDat->mhi_vol_rr = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x0064)) = 0x00000036 | (ULONG)((ULONG)mixDat->mhi_vol_rr << 16);
				setSliderValue(&slider_mhi_r,code);
			}		
			break;		
		case MYGAD_SLIDER_MHI_R:
			drawSliderValue(&slider_mhi_r);
			code = slider_mhi_r.val;			
			mixDat->mhi_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0064)) = 0x00000036 | (ULONG)((ULONG)mixDat->mhi_vol_rr << 16);	// MAIN_MHI_VOLUME_RR
			
			if (mixDat->mhi_locked == TRUE)
			{
				mixDat->mhi_vol_ll = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x0060)) = 0x00000034 | (ULONG)((ULONG)mixDat->mhi_vol_ll << 16);
				setSliderValue(&slider_mhi_l,code);
			}		
			break;		
		case MYGAD_SLIDER_MHI_MIX:
			drawSliderValue(&slider_mhi_mix);
			code = slider_mhi_mix.val;			
			mixDat->mhi_mix_lr = (UWORD)((66196*code)/100);		
			*((ULONG *)((ULONG)cfgMem+0x007c)) = 0x00000042 | (ULONG)((ULONG)mixDat->mhi_mix_lr << 16);	// MAIN_MHI_MIX_LR		
			break;
		case MYGAD_SLIDER_HGN_L:
			drawSliderValue(&slider_hgn_l);
			code = slider_hgn_l.val;			
			mixDat->wav_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0068)) = 0x00000038 | (ULONG)((ULONG)mixDat->wav_vol_ll << 16);	// MAIN_WAV_VOLUME_LL			
			if (mixDat->wav_locked == TRUE)
			{
				mixDat->wav_vol_rr = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x006c)) = 0x0000003a | (ULONG)((ULONG)mixDat->wav_vol_rr << 16);
				setSliderValue(&slider_hgn_r,code);
			}		
			break;		
		case MYGAD_SLIDER_HGN_R:
			drawSliderValue(&slider_hgn_r);	
			code = slider_hgn_r.val;			
			mixDat->wav_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x006c)) = 0x0000003a | (ULONG)((ULONG)mixDat->wav_vol_rr << 16);	// MAIN_WAV_VOLUME_RR
			
			if (mixDat->wav_locked == TRUE)
			{
				mixDat->wav_vol_ll = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x0068)) = 0x00000038 | (ULONG)((ULONG)mixDat->wav_vol_ll << 16);
				setSliderValue(&slider_hgn_l,code);
			}		
			break;		
		case MYGAD_SLIDER_HGN_MIX:
			drawSliderValue(&slider_hgn_mix);	
			code = slider_hgn_mix.val;			
			mixDat->wav_mix_lr = (UWORD)((66196*code)/100);		
			*((ULONG *)((ULONG)cfgMem+0x0080)) = 0x00000044 | (ULONG)((ULONG)mixDat->wav_mix_lr << 16);	// MAIN_WAV_MIX_LR
			break;
		case MYGAD_SLIDER_ADC_L:
			drawSliderValue(&slider_adc_l);	
			code = slider_adc_l.val;			
			mixDat->adc_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0058)) = 0x00000030 | (ULONG)((ULONG)mixDat->adc_vol_ll << 16);	// MAIN_ADC_VOLUME_LL
			
			if (mixDat->adc_locked == TRUE)
			{
				mixDat->adc_vol_rr = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x005c)) = 0x00000032 | (ULONG)((ULONG)mixDat->adc_vol_rr << 16);
				setSliderValue(&slider_adc_r,code);
			}		
			break;		
		case MYGAD_SLIDER_ADC_R:
			drawSliderValue(&slider_adc_r);		
			code = slider_adc_r.val;
			mixDat->adc_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x005c)) = 0x00000032 | (ULONG)((ULONG)mixDat->adc_vol_rr << 16);	// MAIN_ADC_VOLUME_RR
			
			if (mixDat->adc_locked == TRUE)
			{
				mixDat->adc_vol_ll = (UWORD)((66196*code)/100);
				*((ULONG *)((ULONG)cfgMem+0x0058)) = 0x00000030 | (ULONG)((ULONG)mixDat->adc_vol_ll << 16);
				setSliderValue(&slider_adc_l,code);
			}		
			break;		
		case MYGAD_SLIDER_ADC_MIX:
			drawSliderValue(&slider_adc_mix);		
			code = slider_adc_mix.val;			
			mixDat->adc_mix_lr = (UWORD)((66196*code)/100);		
			*((ULONG *)((ULONG)cfgMem+0x0078)) = 0x00000040 | (ULONG)((ULONG)mixDat->adc_mix_lr << 16);	// MAIN_WAV_MIX_LR			
			break;			
		case MYGAD_CHKBOX_PAULA:
			if (box_paula.on != 0)
			{				
				box_paula.on = 0;
				mixDat->adc_enable &= 0xfd;
				*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
				*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);					
			}				
			else
			{				
				box_paula.on = 1;
				mixDat->adc_enable |= 0x02;
				*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
				*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);					
			}				
			drawChkbox(&box_paula);
			break;		
		case MYGAD_CHKBOX_CDROM:
			if (box_cdrom.on != 0)
			{				
				box_cdrom.on = 0;
				mixDat->adc_enable &= 0xfb;
				*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
				*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);					
			}
			else
			{				
				box_cdrom.on = 1;
				mixDat->adc_enable |= 0x04;
				*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
				*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);						
			}				
			drawChkbox(&box_cdrom);
			break;
		case MYGAD_CHKBOX_LINE:
			if (box_line.on != 0)
			{				
				box_line.on = 0;
				mixDat->adc_enable &= 0xf7;	
				*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
				*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);					
			}				
			else
			{				
				box_line.on = 1;
				mixDat->adc_enable |= 0x08;
				*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
				*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);					
			}				
			drawChkbox(&box_line);		
			break;
		case MYGAD_CHKBOX_AHI:
			if (box_ahi.on != 0)
			{
				box_ahi.on = 0;
				mixDat->ahi_locked = FALSE;
			}
			else
			{
				box_ahi.on = 1;
				mixDat->ahi_locked = TRUE;
			}
			drawChkbox(&box_ahi);		
			break;
		case MYGAD_CHKBOX_MHI:
			if (box_mhi.on != 0)
			{				
				box_mhi.on = 0;
				mixDat->mhi_locked = FALSE;
			}
			else
			{				
				box_mhi.on = 1;
				mixDat->mhi_locked = TRUE;
			}
			drawChkbox(&box_mhi);		
			break;
		case MYGAD_CHKBOX_HGN:
			if (box_hgn.on != 0)
			{	
				box_hgn.on = 0;
				mixDat->wav_locked = FALSE;				
			}
			else
			{
				box_hgn.on = 1;
				mixDat->wav_locked = TRUE;				
			}
			drawChkbox(&box_hgn);		
			break;
		case MYGAD_CHKBOX_ADC:
			if (box_adc.on != 0)
			{	
				box_adc.on = 0;
				mixDat->adc_locked = FALSE;
			}
			else
			{
				box_adc.on = 1;
				mixDat->adc_locked = TRUE;
			}
			drawChkbox(&box_adc);		
			break;			
		case MYGAD_CYCLE_LEVELS:
			cycle_levels.val++;
			if (cycle_levels.val >= (cycle_levels.numvals))
				cycle_levels.val = 0;
			switch (cycle_levels.val)
			{
				case 0:
					cycle_levels.text = "NONE";
					intData->int_rate = 0;
					intData->counter = 0;
					break;
				case 1:
					cycle_levels.text = "SLOW";
					intData->int_rate = 5;
					intData->counter = 0;	
					break;
				case 2:
					cycle_levels.text = "MEDI";
					intData->int_rate = 3;
					intData->counter = 0;	
					break;	
				case 3:
					cycle_levels.text = "FAST";
					intData->int_rate = 1;
					intData->counter = 0;
					break;						
			}
			drawCycle(&cycle_levels);
			break;
		case MYGAD_CYCLE_TOSLINK:
			cycle_toslink.val++;
			if (cycle_toslink.val >= (cycle_toslink.numvals))
				cycle_toslink.val = 0;
			switch (cycle_toslink.val)
			{
				case 0:
					cycle_toslink.text = "48kHz";
					break;
				case 1:
					cycle_toslink.text = "96kHz";
					break;
				case 2:
					cycle_toslink.text = "192kHz";
					break;						
			}
			drawCycle(&cycle_toslink);
			mixDat->toslink_srate = cycle_toslink.val;
			*((ULONG *)((ULONG)cfgMem+0x0088)) = 0x00000070 |(ULONG)((ULONG)mixDat->toslink_srate << 16);
			break;	
		default:
			return 0;
			break;
	}
	SetMixer(boardBase,mixDat);
	return MCMD_NONE;
}

void InitGadgets(struct mixData *mixDat, APTR base, APTR cfgMem)
{
	UWORD regVal;

	/* Initialise config structure from flash */
	
	regVal = ReadReg16(base,MAIN_ADC_VOLUME_LL);
	mixDat->adc_vol_ll = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0058)) = 0x00000030 | (ULONG)((ULONG)regVal << 16);	// MAIN_ADC_VOLUME_LL
		
	regVal = ReadReg16(base,MAIN_ADC_VOLUME_RR);
	mixDat->adc_vol_rr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x005C)) = 0x00000032 | (ULONG)((ULONG)regVal << 16);	// MAIN_ADC_VOLUME_RR
	
	regVal = ReadReg16(base,MAIN_MHI_VOLUME_LL);
	mixDat->mhi_vol_ll = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0060)) = 0x00000034 | (ULONG)((ULONG)regVal << 16);	// MAIN_MHI_VOLUME_LL
		
	regVal = ReadReg16(base,MAIN_MHI_VOLUME_RR);
	mixDat->mhi_vol_rr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0064)) = 0x00000036 | (ULONG)((ULONG)regVal << 16);	// MAIN_MHI_VOLUME_RR	
	
	regVal = ReadReg16(base,MAIN_WAV_VOLUME_LL);
	mixDat->wav_vol_ll = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0068)) = 0x00000038 | (ULONG)((ULONG)regVal << 16);	// MAIN_WAV_VOLUME_LL
		
	regVal = ReadReg16(base,MAIN_WAV_VOLUME_RR);
	mixDat->wav_vol_rr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x006c)) = 0x0000003a | (ULONG)((ULONG)regVal << 16);	// MAIN_WAV_VOLUME_RR
	
	regVal = ReadReg16(base,MAIN_AHI_VOLUME_LL);
	mixDat->ahi_vol_ll = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0070)) = 0x0000003c | (ULONG)((ULONG)regVal << 16);	// MAIN_AHI_VOLUME_LL
		
	regVal = ReadReg16(base,MAIN_AHI_VOLUME_RR);
	mixDat->ahi_vol_rr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0074)) = 0x0000003e | (ULONG)((ULONG)regVal << 16);	// MAIN_AHI_VOLUME_RR

	regVal = ReadReg16(base,MAIN_ADC_MIX_LR);
	mixDat->adc_mix_lr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0078)) = 0x00000040 | (ULONG)((ULONG)regVal << 16);	// MAIN_ADC_MIX_LR
	
	regVal = ReadReg16(base,MAIN_MHI_MIX_LR);
	mixDat->mhi_mix_lr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x007C)) = 0x00000042 | (ULONG)((ULONG)regVal << 16);	// MAIN_MHI_MIX_LR
	
	regVal = ReadReg16(base,MAIN_WAV_MIX_LR);
	mixDat->wav_mix_lr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0080)) = 0x00000044 | (ULONG)((ULONG)regVal << 16);	// MAIN_WAV_MIX_LR
	
	regVal = ReadReg16(base,MAIN_AHI_MIX_LR);
	mixDat->ahi_mix_lr = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0084)) = 0x00000046 | (ULONG)((ULONG)regVal << 16);	// MAIN_AHI_MIX_LR
	
	/* Initialise sliders from config structure */
	
	setSliderValue(&slider_ahi_l,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->ahi_vol_ll * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_ahi_r,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->ahi_vol_rr * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_ahi_mix,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->ahi_mix_lr * (ULONG)100), (ULONG)66196)));

	setSliderValue(&slider_mhi_l,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->mhi_vol_ll * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_mhi_r,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->mhi_vol_rr * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_mhi_mix,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->mhi_mix_lr * (ULONG)100), (ULONG)66196)));

	setSliderValue(&slider_hgn_l,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->wav_vol_ll * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_hgn_r,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->wav_vol_rr * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_hgn_mix,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->wav_mix_lr * (ULONG)100), (ULONG)66196)));

	setSliderValue(&slider_adc_l,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->adc_vol_ll * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_adc_r,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->adc_vol_rr * (ULONG)100), (ULONG)66196)));
	setSliderValue(&slider_adc_mix,(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->adc_mix_lr * (ULONG)100), (ULONG)66196)));	
	
	/* Initialise checkboxes from ADC settings */
							
	regVal = (ReadSPI(base,0x06)&0xe);
	mixDat->adc_enable = (UBYTE)regVal;
	*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)regVal << 16);
	*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)regVal << 16);
	
	if ((regVal & 0x2) == 0x02)
	{	
		box_paula.on = 1;		
	}
	else
	{
		box_paula.on = 0;
	}
	drawChkbox(&box_paula);
	
	if ((regVal & 0x4) == 0x04)
	{	
		box_cdrom.on = 1;	
	}
	else
	{
		box_cdrom.on = 0;
	}
	drawChkbox(&box_cdrom);
	
	if ((regVal & 0x8) == 0x08)
	{
		box_line.on = 1;	
	}
	else
	{
		box_line.on = 0;
	}
	drawChkbox(&box_line);
	
	if (mixDat->ahi_vol_ll == mixDat->ahi_vol_rr)
	{
		box_ahi.on = 1;
		mixDat->ahi_locked = TRUE;		
	}
	else
	{
		box_ahi.on = 0;		
		mixDat->ahi_locked = FALSE;
	}
	drawChkbox(&box_ahi);
	
	if (mixDat->mhi_vol_ll == mixDat->mhi_vol_rr)
	{
		box_mhi.on = 1;
		mixDat->mhi_locked = TRUE;
	}
	else
	{
		box_mhi.on = 0;
		mixDat->mhi_locked = FALSE;
	}
	drawChkbox(&box_mhi);
	
	if (mixDat->wav_vol_ll == mixDat->wav_vol_rr)
	{
		box_hgn.on = 1;
		mixDat->wav_locked = TRUE;		
	}
	else
	{
		box_hgn.on = 0;		
		mixDat->wav_locked = FALSE;
	}	
	drawChkbox(&box_hgn);
	
	if (mixDat->adc_vol_ll == mixDat->adc_vol_rr)
	{
		box_adc.on = 1;	
		mixDat->adc_locked = TRUE;		
	}
	else
	{
		box_adc.on = 0;
		mixDat->adc_locked = FALSE;
	}	
	drawChkbox(&box_adc);
	/* Initialise Cycle Gadget from TOSLINK settings */
	
	regVal = ReadReg16(base,MAIN_TOSLINK_CTRL);
	mixDat->toslink_srate = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0088)) = 0x00000070 | (ULONG)((ULONG)regVal << 16);	// MAIN_TOSLINK_CTRL
	
	cycle_toslink.val = mixDat->toslink_srate;
	
	switch (cycle_toslink.val)
	{
		case 0:
			cycle_toslink.text = "48kHz";
			break;
		case 1:
			cycle_toslink.text = "96kHz";
			break;
		case 2:
			cycle_toslink.text = "192kHz";
			break;						
	}
	drawCycle(&cycle_toslink);
}

void processWindowEvents(struct Window *mywin,struct mixData *mixDat,APTR boardBase,APTR cfgMem,ULONG waitMask,struct intData *intData)
{
	ULONG	mainMsg;
	BOOL	terminated = FALSE;
	struct 	IntuiMessage *winMsg;
	UWORD 	command;	
	WORD regVals;
	ULONG levelLeft = 0;
	ULONG levelRight = 0;
	
	while (!terminated)
	{
		command = 0;
		
		mainMsg = Wait((1L << mywin->UserPort->mp_SigBit)|waitMask);
		
		if (waitMask & mainMsg)
		{
			#ifdef DEBUG
				if (levelLeft >= 92)
					levelLeft = 0;
				else
					levelLeft+=1;
				
				if (levelRight >= 92)
					levelRight = 0;
				else
					levelRight+=2;
			#else
				WriteReg16(boardBase,MAIN_LEVELS_RESET,0);
			
				regVals = ReadReg16S(boardBase,MAIN_LEVELS_LEFT);
				if (regVals < 0)
					regVals *= -1;
				levelLeft = regVals / 356;
				
				regVals = ReadReg16S(boardBase,MAIN_LEVELS_RIGHT);	
				if (regVals < 0)
					regVals *= -1;
				
				levelRight = regVals / 356;
			#endif
				
			/* Draw Negative Bar */
			SetAPen(myWin->RPort, 0);
			RectFill(myWin->RPort, 471, topBorder+46, 471+11, topBorder+46+(92-levelLeft));	
			RectFill(myWin->RPort, 495, topBorder+46, 495+11, topBorder+46+(92-levelRight));
			/* Draw Positive Bar */
			SetAPen(myWin->RPort, 3);
			RectFill(myWin->RPort, 471, topBorder+46+(92-levelLeft), 471+11, topBorder+46+92);	
			RectFill(myWin->RPort, 495, topBorder+46+(92-levelRight), 495+11, topBorder+46+92);
			

		}
		else if ((1L << mywin->UserPort->mp_SigBit) & mainMsg)
		{
			winMsg = (struct IntuiMessage *)GetMsg(mywin->UserPort);
			switch (winMsg->Class)
			{
				case IDCMP_GADGETDOWN:
				case IDCMP_MOUSEMOVE:
				case IDCMP_GADGETUP:
					command = handleGadgetEvent((struct Gadget *)winMsg->IAddress,mixDat,boardBase,cfgMem,intData);
					break;				
				case CLOSEWINDOW:
					terminated = TRUE;
					break;
				case REFRESHWINDOW:
					BeginRefresh(mywin);
					EndRefresh(mywin, TRUE);
					drawBorders(mywin->RPort,topBorder,font);
					break;
			}
		}
		switch (command)
		{
			case MCMD_USE:
				terminated = TRUE;
				break;
			case MCMD_SAVE:
				EraseFlash(boardBase);
				ProgramFlash(boardBase,cfgMem);			
				break;
			case MCMD_RESET:
				setSliderValue(&slider_ahi_l,50);
				setSliderValue(&slider_ahi_r,50);
				setSliderValue(&slider_ahi_mix,0);

				setSliderValue(&slider_mhi_l,50);
				setSliderValue(&slider_mhi_r,50);
				setSliderValue(&slider_mhi_mix,0);

				setSliderValue(&slider_hgn_l,50);
				setSliderValue(&slider_hgn_r,50);
				setSliderValue(&slider_hgn_mix,0);

				setSliderValue(&slider_adc_l,50);
				setSliderValue(&slider_adc_r,50);
				setSliderValue(&slider_adc_mix,0);	
			
				box_paula.on = 1;
				drawChkbox(&box_paula);
				box_cdrom.on = 0;
				drawChkbox(&box_cdrom);
				box_line.on = 0;
				drawChkbox(&box_line);				
				box_ahi.on = 1;
				drawChkbox(&box_ahi);
				box_mhi.on = 1;
				drawChkbox(&box_mhi);
				box_hgn.on = 1;
				drawChkbox(&box_hgn);
				box_adc.on = 1;
				drawChkbox(&box_adc);

				cycle_toslink.val = 0;
				cycle_toslink.text = "48kHz";
				drawCycle(&cycle_toslink);
				
				InitCFGMem (cfgMem);					
				mixDat->ahi_vol_ll = 0x8000;
				mixDat->ahi_vol_rr = 0x8000;
				mixDat->mhi_vol_ll = 0x8000;
				mixDat->mhi_vol_rr = 0x8000;
				mixDat->wav_vol_ll = 0x8000;
				mixDat->wav_vol_rr = 0x8000;
				mixDat->adc_vol_ll = 0x8000;
				mixDat->adc_vol_rr = 0x8000;
			
				mixDat->ahi_mix_lr = 0x0000;
				mixDat->mhi_mix_lr = 0x0000;
				mixDat->wav_mix_lr = 0x0000;
				mixDat->adc_mix_lr = 0x0000;		
		
				mixDat->adc_enable = 0x0002;
		
				mixDat->toslink_srate = 0x0;

				mixDat->ahi_locked = TRUE;
				mixDat->mhi_locked = TRUE;
				mixDat->wav_locked = TRUE;
				mixDat->adc_locked = TRUE;
				SetMixer(boardBase,mixDat);				
				break;
		}				
			
	}
}


void drawWindow(void)
{
	struct Screen   *mysc;
	void            *vi;

	struct mixData		*mixDat;
	struct ConfigDev 	*myCD;

	struct Interrupt 	*mixerInt;
	struct intData 		*intData;
	
	UBYTE	boardProdId;
	UWORD	boardManufacId;
	APTR	boardBase;
	
	UWORD	topBorder;
	APTR	cfgMem;
	ULONG	waitMask;
	BYTE	sigNr;	

#ifndef DEBUG
	BOOL	boardFound = FALSE;
#else
	BOOL	boardFound = TRUE;
#endif

	/* ================ Find AmiGUS card ================ */
	
	myCD = NULL;
    while(myCD=FindConfigDev(myCD,-1L,-1L)) /* search for all ConfigDevs */	
	{
		boardManufacId = myCD->cd_Rom.er_Manufacturer;
		boardProdId = myCD->cd_Rom.er_Product;
		boardBase = myCD->cd_BoardAddr;
		if (boardManufacId == AMIGUS_MANUFACTURER_ID && boardProdId == AMIGUS_MAIN_PRODUCT_ID)
		{
			boardFound = TRUE;
			break;
		}
	}
	
	if (boardFound == TRUE)
	{
		//printf("AmiGUS found at $%lx\n",boardBase);
	}
	else
	{
		printf("ERROR: no AmiGUS board found!\n");	
		return;
	}	

	if (cfgMem = AllocMem(FLASH_CONFIG_SIZE,MEMF_ANY))
    {
		InitCFGMem(cfgMem);
	}
	else 
	{
		return;
	}
	
	if (mixDat = AllocMem(sizeof(struct mixData), MEMF_PUBLIC|MEMF_CLEAR))
    {
		mixDat->ahi_vol_ll = 0x8000;
		mixDat->ahi_vol_rr = 0x8000;
		mixDat->mhi_vol_ll = 0x8000;
		mixDat->mhi_vol_rr = 0x8000;	
		mixDat->wav_vol_ll = 0x8000;
		mixDat->wav_vol_rr = 0x8000;
		mixDat->adc_vol_ll = 0x8000;
		mixDat->adc_vol_rr = 0x8000;
		mixDat->ahi_mix_lr = 0x0000;
		mixDat->mhi_mix_lr = 0x0000;
		mixDat->wav_mix_lr = 0x0000;
		mixDat->adc_mix_lr = 0x0000;
		
		mixDat->toslink_srate = 0x0000;
		
		mixDat->adc_enable = 0x2;		
	}
	else
	{
		return;
	}	

	// Check if reset sequence of AmiGUS was successful
	if ((ReadReg16(boardBase,FLASH_CONFIG_STATUS)&0x8000) == 0x8000)
	{
		EraseFlash(boardBase);
		ProgramFlash(boardBase,cfgMem);
		SetMixer(boardBase,mixDat);		
	}

	/* ================ Configure ADC ================ */

	WriteSPI(boardBase, 0x70, 0x75);	// Enable ADC I2S Master Mode

	WriteSPI(boardBase, 0x20, 0x90);	// Enable ADC I2S Master Mode
	WriteSPI(boardBase, 0x21, 0x0);		// DSP1 Clock
	WriteSPI(boardBase, 0x22, 0x0);		// DSP2 Clock
	WriteSPI(boardBase, 0x23, 0x7);		// ADC Clock

	WriteSPI(boardBase, 0x26, 0x3);		// Set BCLK = CLK/2 (192kHz sampling rate)	
	
	WriteSPI(boardBase, 0x28, 0x0);		// Disable PLL
	
	WriteSPI(boardBase, 0x19, 0xff);	// Set Manual Gain Control
	WriteSPI(boardBase, 0x01, 0x20);	// Increase Left Gain
	WriteSPI(boardBase, 0x02, 0x20);	// Increase Right Gain

	WriteSPI(boardBase, 0x70, 0x70);	// Enable ADC I2S Master Mode

	/* ================== Interrupt Routine =============== */
	
	if ((sigNr = AllocSignal(-1)) == -1)          /* Allocate a signal bit for the   */
    {                                             /* interrupt handler to signal us. */
        printf("ERROR: can't allocate signal\n");
        return;			
	}

	if (intData = AllocMem(sizeof(struct intData), MEMF_PUBLIC|MEMF_CLEAR))
    {
		intData->int_rate = 5;
		intData->counter = 0;
		intData->rd_Signal = 1L << sigNr;
		intData->rd_Task = FindTask(NULL);		
	}
	else 
	{
		FreeSignal(sigNr);
        printf("ERROR: can't allocate memory for interrupt node\n");
        return;		
	}

	waitMask = intData->rd_Signal;
	
    if (mixerInt = AllocMem(sizeof(struct Interrupt), 
                         MEMF_PUBLIC|MEMF_CLEAR))
    {
		mixerInt->is_Node.ln_Type = NT_INTERRUPT;
        mixerInt->is_Node.ln_Pri = -60;
        mixerInt->is_Node.ln_Name = "AmiGUS-Mixer";
        mixerInt->is_Data = (APTR)intData;
        mixerInt->is_Code = intServer;
		
		AddIntServer(INTB_VERTB, mixerInt);
	}
	else 
	{
        printf("ERROR: can't allocate memory for interrupt node\n");
		FreeMem(intData,sizeof(struct intData));
		FreeSignal(sigNr);
		
        return;		
	}

	
	if (kick13 == FALSE)
	{
		if (NULL == (GadToolsBase = OpenLibrary("gadtools.library", 37)))
		{
			printf( "Requires V37 gadtools.library");
			return;
		}
		else			
		{
			if (NULL == (mysc = LockPubScreen(NULL)))
			{
				printf( "Couldn't lock default public screen");
				return;
			}
			else
			{
				if (NULL == (vi = GetVisualInfo(mysc, TAG_END)))
				{	
					printf( "GetVisualInfo() failed");
					return;
				}
				else
				{
					topBorder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);
				}
			}
			CloseLibrary(GadToolsBase);
		}
		if (NULL == (AslBase = OpenLibrary("asl.library", 37)))
		{
			printf( "Requires V37 asl.library");
			return;
		}
	}
	else
	{
		topBorder = 14;
	}

	if (NULL == (font = OpenFont(&mainFont)))
		printf("Failed to open Topaz 80");
	else
	{
		if (GetScreenData((APTR)&myScreen, sizeof(struct Screen), WBENCHSCREEN, NULL)) {
			wbscr_barheight = myScreen.BarHeight;
			wbscr_height = myScreen.Height;
			wbscr_width = myScreen.Width;
		} else {
			wbscr_barheight = 10;
			wbscr_height = 200;
			wbscr_width = 640;
		}
		mainWindow.LeftEdge = (wbscr_width - mainWindow.Width) / 2;
		mainWindow.TopEdge = (wbscr_height - mainWindow.Height) / 2;

		if ((myWin = (struct Window *)OpenWindow(&mainWindow) ) == NULL) 
			return;
		
		rp = myWin->RPort;
		
		createAllGadgets(topBorder);
		InitGadgets(mixDat,boardBase,cfgMem);		
		drawBorders(myWin->RPort,topBorder,font);
		
		processWindowEvents(myWin,mixDat,boardBase,cfgMem,waitMask,intData);
		CloseWindow(myWin);
	}

	RemIntServer(INTB_VERTB, mixerInt);
	FreeMem(cfgMem,FLASH_CONFIG_SIZE);
	FreeMem(mixDat,sizeof(struct mixData));
	FreeMem(intData,sizeof(struct intData));
	FreeMem(mixerInt, sizeof(struct Interrupt));		
	FreeSignal(sigNr);
}	
		

void main(void)
{	
	SetTaskPri(FindTask(NULL),30);
	if (NULL == (IntuitionBase = OpenLibrary("intuition.library", 34)))
		printf( "Requires V34 intuition.library\n");
	else
	{
		if (NULL == (ExpansionBase = OpenLibrary("expansion.library", 34)))
			printf("Requires V34 expansion.library\n");
		else	
		{
			if (NULL == (GfxBase = OpenLibrary("graphics.library", 36)))
			{
				if (NULL == (GfxBase = OpenLibrary("graphics.library", 34)))
					printf( "Requires V34 graphics.library\n");
				else
				{
					kick13 = TRUE;
					drawWindow();
				}
				CloseLibrary(GfxBase);
			}
			else
			{
				kick13 = FALSE;
				drawWindow();
			}
			CloseLibrary(GfxBase);
		}
		CloseLibrary(ExpansionBase);
	}
	CloseLibrary(IntuitionBase);
}
