/*
======================================================================
AmiGUS Mixer Utility
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

#include <libraries/configvars.h>
#include <libraries/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/expansion_protos.h>
#include <hardware/intbits.h>
#include <proto/expansion.h>

#include <stdio.h>

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

/* Range for the slider: */
#define SLIDER_MIN  (0)
#define SLIDER_MAX (99)

#define	FLASH_CONFIG_START		0x4000
#define	FLASH_CONFIG_SIZE		0x4000

extern void intServer();

struct TextAttr Topaz80 = { "topaz.font", 8, 0, 0, };

struct Library      *IntuitionBase;
struct Library      *ExpansionBase;
struct Library      *GfxBase;
struct Library      *GadToolsBase;

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

void DrawBorders(struct RastPort *rp,UWORD topBorder,struct TextFont *font)
{
	struct Border	shineBorder;
	struct Border	shadowBorder;

	struct IntuiText	myIText;
	struct TextAttr		myTextAttr;

	long myShadowPen = 1;
	long myShinePen  = 2;

	long myTextPen = 1;
	long myBackgroundPen = 0;

	int startX = 12;

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

void InitCFGMem (APTR cfgMem)
{
	ULONG cnt = 0;
		
	do {
		*((ULONG *)((ULONG)cfgMem+cnt)) = 0xffffffff;
		cnt+=4;
	} while (cnt != 0x4000);
		
	*((ULONG *)((ULONG)cfgMem+0x0000)) = 0x414d4947;	// Magic Token - Unlock

/* Fix ADC Initialisation */

	// ADC Power-Down
	*((ULONG *)((ULONG)cfg_mem+0x0004)) = 0x00700020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0008)) = 0x00700022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x000c)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// Enable ADC I2S Master Mode

	*((ULONG *)((ULONG)cfg_mem+0x0010)) = 0x00200020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0014)) = 0x00900022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0018)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	//  Set Manual Gain Control

	*((ULONG *)((ULONG)cfg_mem+0x001c)) = 0x00190020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0020)) = 0x00ff0022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0024)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Increase Left Gain
		
	*((ULONG *)((ULONG)cfg_mem+0x0028)) = 0x00010020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x002c)) = 0x00200022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0030)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Increase Right Gain
		
	*((ULONG *)((ULONG)cfg_mem+0x0034)) = 0x00020020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0038)) = 0x00200022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x003c)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Enable Left Inputs
		
	*((ULONG *)((ULONG)cfg_mem+0x0040)) = 0x00060020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00020022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0048)) = 0x00000024;	// MAIN_SPI_WTRIG		
		
	// Enable Right Inputs
		
	*((ULONG *)((ULONG)cfg_mem+0x004c)) = 0x00070020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00020022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0054)) = 0x00000024;	// MAIN_SPI_WTRIG			
		
/* Mixer Settings */
	
	*((ULONG *)((ULONG)cfg_mem+0x0058)) = 0x80000030;	// MAIN_ADC_VOLUME_LL
	*((ULONG *)((ULONG)cfg_mem+0x005c)) = 0x80000032;	// MAIN_ADC_VOLUME_RR
	*((ULONG *)((ULONG)cfg_mem+0x0060)) = 0x80000034;	// MAIN_MHI_VOLUME_LL
	*((ULONG *)((ULONG)cfg_mem+0x0064)) = 0x80000036;	// MAIN_MHI_VOLUME_RR	
	*((ULONG *)((ULONG)cfg_mem+0x0068)) = 0x80000038;	// MAIN_WAV_VOLUME_LL
	*((ULONG *)((ULONG)cfg_mem+0x006c)) = 0x8000003a;	// MAIN_WAV_VOLUME_RR
	*((ULONG *)((ULONG)cfg_mem+0x0070)) = 0x8000003c;	// MAIN_AHI_VOLUME_LL
	*((ULONG *)((ULONG)cfg_mem+0x0074)) = 0x8000003e;	// MAIN_AHI_VOLUME_RR

	*((ULONG *)((ULONG)cfg_mem+0x0078)) = 0x00000040;	// MAIN_ADC_MIX_LR
	*((ULONG *)((ULONG)cfg_mem+0x007c)) = 0x00000042;	// MAIN_MHI_MIX_LR
	*((ULONG *)((ULONG)cfg_mem+0x0080)) = 0x00000044;	// MAIN_WAV_MIX_LR
	*((ULONG *)((ULONG)cfg_mem+0x0084)) = 0x00000046;	// MAIN_AHI_MIX_LR

/* TOSLINK Settings */

	*((ULONG *)((ULONG)cfg_mem+0x0088)) = 0x00000070;	// MAIN_TOSLINK_CTRL

	// Set DSP1 CLOCK
	*((ULONG *)((ULONG)cfg_mem+0x008c)) = 0x00210020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0090)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0094)) = 0x00000024;	// MAIN_SPI_WTRIG
	
	// Set DSP2 CLOCK
	*((ULONG *)((ULONG)cfg_mem+0x0098)) = 0x00220020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x009c)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00a0)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// Set ADC CLOCK
	*((ULONG *)((ULONG)cfg_mem+0x00a4)) = 0x00230020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00a8)) = 0x00070022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00ac)) = 0x00000024;	// MAIN_SPI_WTRIG

	// Set BCLK = CLK/4 (192kHz sampling rate)
	*((ULONG *)((ULONG)cfg_mem+0x00b0)) = 0x00260020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00b4)) = 0x00030022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00b8)) = 0x00000024;	// MAIN_SPI_WTRIG
	
	// Disable PLL
	*((ULONG *)((ULONG)cfg_mem+0x00bc)) = 0x00280020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00e0)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00e4)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// ADC Power-Up
	*((ULONG *)((ULONG)cfg_mem+0x00e8)) = 0x00700020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00ec)) = 0x00700022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00f0)) = 0x00000024;	// MAIN_SPI_WTRIG	

/* End of Stream */
	*((ULONG *)((ULONG)cfg_mem+0x00f4)) = 0xffffffff;
}

void UpdateSliders(struct Window *win, struct Gadget *myGads[], struct mixData *mixDat)
{
	UWORD regVal;
	
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->ahi_vol_ll * (ULONG)100), (ULONG)66196));
	GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_L], win, NULL,
                           GTSL_Level, regVal,
                            TAG_END);
	
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->ahi_vol_rr * (ULONG)100), (ULONG)66196));	
    GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_R], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);
	
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->mhi_vol_ll * (ULONG)100), (ULONG)66196));
    GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_L], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);

	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->mhi_vol_rr * (ULONG)100), (ULONG)66196));		
	GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_R], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);   
	
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->wav_vol_ll * (ULONG)100), (ULONG)66196));
	GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_L], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);
							
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->wav_vol_rr * (ULONG)100), (ULONG)66196));					
    GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_R], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);

	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->adc_vol_ll * (ULONG)100), (ULONG)66196));					
    GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_L], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);
					
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->adc_vol_rr * (ULONG)100), (ULONG)66196));							
    GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_R], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);	
							
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->ahi_mix_lr * (ULONG)100), (ULONG)66196));			
	GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_MIX], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);
							
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->mhi_mix_lr * (ULONG)100), (ULONG)66196));	
	GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_MIX], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);

	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->wav_mix_lr * (ULONG)100), (ULONG)66196));	
    GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_MIX], win, NULL,
                            GTSL_Level, regVal,
                            TAG_END);
    
	regVal = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixDat->adc_mix_lr * (ULONG)100), (ULONG)66196));	
	GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_MIX], win, NULL,
                            GTSL_Level, regVal,
							TAG_END);
}

void InitGadgets(struct Window *win, struct Gadget *myGads[], struct mixData *mixDat, APTR base, APTR cfgMem)
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
	
	UpdateSliders(win,myGads,mixDat);
	
	/* Initialise checkboxes from ADC settings */
							
	regVal = (ReadSPI(base,0x06)&0xe);
	mixDat->adc_enable = (UBYTE)regVal;
	*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)regVal << 16);
	*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)regVal << 16);
	
	if ((regVal & 0x2) == 0x02)
	{	
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_PAULA], win, NULL,
								GTCB_Checked, 1,
								TAG_END);
	}
	else
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_PAULA], win, NULL,
								GTCB_Checked, 0,
								TAG_END);
	}
	
	if ((regVal & 0x4) == 0x04)
	{	
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_CDROM], win, NULL,
								GTCB_Checked, 1,
								TAG_END);
	}
	else
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_CDROM], win, NULL,
								GTCB_Checked, 0,
								TAG_END);
	}
	
	if ((regVal & 0x8) == 0x08)
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_LINE], win, NULL,
								GTCB_Checked, 1,
								TAG_END);
	}
	else
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_LINE], win, NULL,
								GTCB_Checked, 0,
								TAG_END);
	}
	
	if (mixDat->ahi_vol_ll == mixDat->ahi_vol_rr)
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_AHI], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixDat->ahi_locked = TRUE;
	}
	else
	{
		mixDat->ahi_locked = FALSE;
	}
	
	if (mixDat->mhi_vol_ll == mixDat->mhi_vol_rr)
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_MHI], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixDat->mhi_locked = TRUE;							
	}
	else
	{
		mixDat->mhi_locked = FALSE;
	}

	if (mixDat->wav_vol_ll == mixDat->wav_vol_rr)
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_HGN], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixDat->wav_locked = TRUE;					
	}
	else
	{
		mixDat->wav_locked = FALSE;
	}	

	if (mixDat->adc_vol_ll == mixDat->adc_vol_rr)
	{
		GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_ADC], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixDat->adc_locked = TRUE;
	}
	else
	{
		mixDat->adc_locked = FALSE;
	}	

	/* Initialise Cycle Gadget from TOSLINK settings */
	
	regVal = ReadReg16(base,MAIN_TOSLINK_CTRL);
	mixDat->toslink_srate = regVal;
	*((ULONG *)((ULONG)cfgMem+0x0088)) = 0x00000070 | (ULONG)((ULONG)regVal << 16);	// MAIN_TOSLINK_CTRL
	
	GT_SetGadgetAttrs(myGads[MYGAD_CYCLE_TOSLINK], win, NULL,
				GTCY_Active, mixDat->toslink_srate,
				TAG_END);	
}

void HandleGadgetEvent(struct Window *win, struct Gadget *gad, UWORD code,
		struct Gadget *myGads[], BOOL *terminated,struct mixData *mixDat,APTR boardBase,APTR cfgMem,struct intData *intData)
{
switch (gad->GadgetID)
    {
    case MYGAD_SLIDER_AHI_L:
		mixDat->ahi_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x0070)) = 0x0000003c | (ULONG)((ULONG)mixDat->ahi_vol_ll << 16);	// MAIN_AHI_VOLUME_LL
		if (mixDat->ahi_locked == TRUE)
		{
			mixDat->ahi_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0074)) = 0x0000003e | (ULONG)((ULONG)mixDat->ahi_vol_rr << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_AHI_R:
		mixDat->ahi_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x0074)) = 0x0000003e | (ULONG)((ULONG)mixDat->ahi_vol_rr << 16);	// MAIN_AHI_VOLUME_RR
		
		if (mixDat->ahi_locked == TRUE)
		{
			mixDat->ahi_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0070)) = 0x0000003c | (ULONG)((ULONG)mixDat->ahi_vol_ll << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_AHI_MIX:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->ahi_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfgMem+0x0084)) = 0x00000046 | (ULONG)((ULONG)mixDat->ahi_mix_lr << 16);	// MAIN_AHI_MIX_LR
        break;
    case MYGAD_SLIDER_MHI_L:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->mhi_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x0060)) = 0x00000034 | (ULONG)((ULONG)mixDat->mhi_vol_ll << 16);	// MAIN_MHI_VOLUME_LL
		
		if (mixDat->mhi_locked == TRUE)
		{
			mixDat->mhi_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0064)) = 0x00000036 | (ULONG)((ULONG)mixDat->mhi_vol_rr << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_MHI_R:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->mhi_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x0064)) = 0x00000036 | (ULONG)((ULONG)mixDat->mhi_vol_rr << 16);	// MAIN_MHI_VOLUME_RR
		
		if (mixDat->mhi_locked == TRUE)
		{
			mixDat->mhi_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0060)) = 0x00000034 | (ULONG)((ULONG)mixDat->mhi_vol_ll << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_MHI_MIX:
       /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->mhi_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfgMem+0x007c)) = 0x00000042 | (ULONG)((ULONG)mixDat->mhi_mix_lr << 16);	// MAIN_MHI_MIX_LR
        //printf("Slider at level %lx, %lx\n", mixDat->mhi_vol_lr,mixDat->mhi_vol_rl);
        break;	
    case MYGAD_SLIDER_HGN_L:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->wav_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x0068)) = 0x00000038 | (ULONG)((ULONG)mixDat->wav_vol_ll << 16);	// MAIN_WAV_VOLUME_LL
		
		if (mixDat->wav_locked == TRUE)
		{
			mixDat->wav_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x006c)) = 0x0000003a | (ULONG)((ULONG)mixDat->wav_vol_rr << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_HGN_R:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->wav_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x006c)) = 0x0000003a | (ULONG)((ULONG)mixDat->wav_vol_rr << 16);	// MAIN_WAV_VOLUME_RR
		
		if (mixDat->wav_locked == TRUE)
		{
			mixDat->wav_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0068)) = 0x00000038 | (ULONG)((ULONG)mixDat->wav_vol_ll << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_HGN_MIX:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->wav_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfgMem+0x0080)) = 0x00000044 | (ULONG)((ULONG)mixDat->wav_mix_lr << 16);	// MAIN_WAV_MIX_LR
        //printf("Slider at level %lx, %lx\n", mixDat->wav_vol_lr,mixDat->wav_vol_rl);
        break;			
    case MYGAD_SLIDER_ADC_L:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->adc_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x0058)) = 0x00000030 | (ULONG)((ULONG)mixDat->adc_vol_ll << 16);	// MAIN_ADC_VOLUME_LL
		
		if (mixDat->adc_locked == TRUE)
		{
			mixDat->adc_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x005c)) = 0x00000032 | (ULONG)((ULONG)mixDat->adc_vol_rr << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_ADC_R:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->adc_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfgMem+0x005c)) = 0x00000032 | (ULONG)((ULONG)mixDat->adc_vol_rr << 16);	// MAIN_ADC_VOLUME_RR
		
		if (mixDat->adc_locked == TRUE)
		{
			mixDat->adc_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfgMem+0x0058)) = 0x00000030 | (ULONG)((ULONG)mixDat->adc_vol_ll << 16);

			 GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_ADC_MIX:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixDat->adc_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfgMem+0x0078)) = 0x00000040 | (ULONG)((ULONG)mixDat->adc_mix_lr << 16);	// MAIN_WAV_MIX_LR	
        break;		
    case MYGAD_CHKBOX_PAULA:
		if ((myGads[MYGAD_CHKBOX_PAULA]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->adc_enable |= 0x02;
			*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
			*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);		
		}
		else
		{			
			mixDat->adc_enable &= 0xfd;
			*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
			*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);			
		}
		break;
    case MYGAD_CHKBOX_CDROM:
		if ((myGads[MYGAD_CHKBOX_CDROM]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->adc_enable |= 0x04;
			*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
			*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);			
		}	
		else
		{			
			mixDat->adc_enable &= 0xfb;
			*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
			*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);			
		}		
		break;
    case MYGAD_CHKBOX_LINE:
		if ((myGads[MYGAD_CHKBOX_LINE]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->adc_enable |= 0x08;
			*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
			*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);			
		}
		else
		{			
			mixDat->adc_enable &= 0xf7;	
			*((ULONG *)((ULONG)cfgMem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);
			*((ULONG *)((ULONG)cfgMem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixDat->adc_enable << 16);			
		}
		break;
    case MYGAD_CHKBOX_AHI:
		if ((myGads[MYGAD_CHKBOX_AHI]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->ahi_locked = TRUE;
		}
		else
		{
			mixDat->ahi_locked = FALSE;
		}
		break;
    case MYGAD_CHKBOX_MHI:
		if ((myGads[MYGAD_CHKBOX_MHI]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->mhi_locked = TRUE;
		}
		else
		{
			mixDat->mhi_locked = FALSE;
		}		
		break;
    case MYGAD_CHKBOX_HGN:
		if ((myGads[MYGAD_CHKBOX_HGN]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->wav_locked = TRUE;
		}
		else
		{
			mixDat->wav_locked = FALSE;
		}		
		break;		
    case MYGAD_CHKBOX_ADC:
		if ((myGads[MYGAD_CHKBOX_ADC]->Flags & GFLG_SELECTED) != 0)
		{
			mixDat->adc_locked = TRUE;
		}
		else
		{
			mixDat->adc_locked = FALSE;
		}		
		break;		
    case MYGAD_BUTTON_SAVE:
		EraseFlash(boardBase);
		ProgramFlash(boardBase,cfgMem);
		break;
    case MYGAD_BUTTON_USE:
		*terminated = TRUE;
		break;
    case MYGAD_BUTTON_RESET:
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);	
							
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_AHI_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_MHI_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_HGN_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_SLIDER_ADC_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
							
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_PAULA], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);		
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_CDROM], win, NULL,
                            GTCB_Checked, 0,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_LINE], win, NULL,
                            GTCB_Checked, 0,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_AHI], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_MHI], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_HGN], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
       GT_SetGadgetAttrs(myGads[MYGAD_CHKBOX_ADC], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
		
		GT_SetGadgetAttrs(myGads[MYGAD_CYCLE_TOSLINK], win, NULL,
				GTCY_Active, 0,
				TAG_END);
				
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
        break;
	case MYGAD_CYCLE_LEVELS:
		switch (code)
		{
			case 0:
				intData->int_rate = 0;
				intData->counter = 0;
				break;
			case 1:
				intData->int_rate = 5;
				intData->counter = 0;			
				break;
			case 2:
				intData->int_rate = 3;
				intData->counter = 0;			
				break;
			case 3:
				intData->int_rate = 1;
				intData->counter = 0;			
				break;
		}
		break;
	case MYGAD_CYCLE_TOSLINK:
		mixDat->toslink_srate = (UWORD)code;
		*((ULONG *)((ULONG)cfgMem+0x0088)) = 0x00000070 |(ULONG)((ULONG)mixDat->toslink_srate << 16);	// MAIN_TOSLINK_CTRL
		break;
	
    }
	SetMixer(boardBase,mixDat);
}


/*
** Function to handle vanilla keys.
*/
void HandleVanillaKey(struct Window *win, UWORD code, struct Gadget *myGads[])
{
switch (code)
    {
    case 's':
    case 'S':

        break;
    case 'u':
    case 'U':
	
        break;
    case 'r':
    case 'R':

        break;
    }
}


/*
** Here is where all the initialization and creation of GadTools gadgets
** take place.  This function requires a pointer to a NULL-initialized
** gadget list pointer.  It returns a pointer to the last created gadget,
** which can be checked for success/failure.
*/
struct Gadget *CreateAllGadgets(struct Gadget **gadListptr, void *vi,
    UWORD topBorder, struct Gadget *myGads[])
{
	struct NewGadget ng;
	struct Gadget *gad;

   static const char *levelsOptions[] =
    {
		"OFF",
		"SLOW",
		"NORM",
		"FAST",
		NULL
    };
	
	static const char *toslinkOptions[] =
    {
		"48kHz",
		"96kHz",
		"192kHz",
		NULL
    };

	gad = CreateContext(gadListptr);

	ng.ng_LeftEdge   = 20;
	ng.ng_TopEdge    = 32+topBorder;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "L";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_AHI_L;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_AHI_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);
						
	ng.ng_LeftEdge  += 20;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "R";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_AHI_R;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_AHI_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);				

	ng.ng_LeftEdge  += 32;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "Mix";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_AHI_MIX;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_AHI_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       0,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);	
						
	ng.ng_LeftEdge  += 40;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "L";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_MHI_L;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_MHI_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);	

	ng.ng_LeftEdge  += 20;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "R";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_MHI_R;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_MHI_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);

	ng.ng_LeftEdge  += 32;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "Mix";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_MHI_MIX;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_MHI_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       0,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);	

	ng.ng_LeftEdge  += 40;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "L";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_HGN_L;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_HGN_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);	

	ng.ng_LeftEdge  += 20;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "R";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_HGN_R;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_HGN_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);

	ng.ng_LeftEdge  += 32;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "Mix";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_HGN_MIX;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_HGN_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       0,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);						

	ng.ng_LeftEdge  += 40;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "L";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_ADC_L;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_ADC_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);	

	ng.ng_LeftEdge  += 20;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "R";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_ADC_R;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_ADC_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       (SLIDER_MAX>>1)+1,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);

	ng.ng_LeftEdge  += 32;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "Mix";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_ADC_MIX;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	myGads[MYGAD_SLIDER_ADC_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
						GTSL_Min,         SLIDER_MIN,
						GTSL_Max,         SLIDER_MAX,
						GTSL_Level,       0,
						GTSL_LevelFormat, "%2ld",
						GTSL_MaxLevelLen, 2,
						GTSL_LevelPlace, PLACETEXT_BELOW,
						GT_Underscore,    '_',
						PGA_Freedom,	LORIENT_VERT,
						TAG_END);	

	/* ========== Checkbox Gadgets =========== */

	ng.ng_LeftEdge  += 30;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Paula";
	ng.ng_GadgetID   = MYGAD_CHKBOX_PAULA;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_PAULA] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);
						
	ng.ng_LeftEdge  += 0;
	ng.ng_TopEdge   += 16;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "CDROM";
	ng.ng_GadgetID   = MYGAD_CHKBOX_CDROM;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_CDROM] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);	
						
	ng.ng_LeftEdge  += 0;
	ng.ng_TopEdge   += 16;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Line";
	ng.ng_GadgetID   = MYGAD_CHKBOX_LINE;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_LINE] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);	

	ng.ng_LeftEdge  = 24;
	ng.ng_TopEdge   = topBorder + 132-14;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_AHI;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_AHI] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);							

	ng.ng_LeftEdge  += 92;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_MHI;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_MHI] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);

	ng.ng_LeftEdge  += 92;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_HGN;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_HGN] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);
						
	ng.ng_LeftEdge  += 92;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_ADC;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	myGads[MYGAD_CHKBOX_ADC] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);

	/* ========== Button Gadgets =========== */

	ng.ng_LeftEdge  = 98;
	ng.ng_TopEdge   += 20;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "_Save";
	ng.ng_GadgetID   = MYGAD_BUTTON_SAVE;
	ng.ng_Flags      = 0;
	myGads[MYGAD_BUTTON_SAVE] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
						GT_Underscore, '_',
						TAG_END);
						
	ng.ng_LeftEdge  +=96;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "_Use";
	ng.ng_GadgetID   = MYGAD_BUTTON_USE;
	ng.ng_Flags      = 0;
	myGads[MYGAD_BUTTON_USE] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
						GT_Underscore, '_',
						TAG_END);
						
	ng.ng_LeftEdge  += 96;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "_Reset";
	ng.ng_GadgetID   = MYGAD_BUTTON_RESET;
	ng.ng_Flags      = 0;
	myGads[MYGAD_BUTTON_RESET] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
						GT_Underscore, '_',
						TAG_END);	
	/* ========== Cycle Gadgets =========== */

	ng.ng_LeftEdge  += 166;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 64;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "";
	ng.ng_GadgetID   = MYGAD_CYCLE_LEVELS;
	ng.ng_Flags      = 0;
	myGads[MYGAD_CYCLE_LEVELS] = gad = CreateGadget(CYCLE_KIND, gad, &ng,
						GTCY_Labels, (ULONG)levelsOptions,
						GTCY_Active, 1,
						GA_Disabled, FALSE,
						TAG_END);		

	ng.ng_LeftEdge  -= 76;
	ng.ng_TopEdge   -= 42;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "";
	ng.ng_GadgetID   = MYGAD_CYCLE_TOSLINK;
	ng.ng_Flags      = 0;
	myGads[MYGAD_CYCLE_TOSLINK] = gad = CreateGadget(CYCLE_KIND, gad, &ng,
						GTCY_Labels, (ULONG)toslinkOptions,
						GTCY_Active, 0,
						GA_Disabled, FALSE,
						TAG_END);							
						
	return(gad);
}




void ProcessWindowEvents(struct Window *myWin,
	struct Gadget *myGads[],struct TextFont *font,UWORD topBorder,struct mixData *mixDat,APTR boardBase,APTR cfgMem,ULONG waitMask,struct intData *intData)
{
	struct IntuiMessage *imsg;
	ULONG imsgClass;
	UWORD imsgCode;
	struct Gadget *gad;
	BOOL terminated = FALSE;
	ULONG signVal;
	WORD regVals;
	ULONG levelLeft = 0;
	ULONG levelRight = 0;
	
	while (!terminated)
	{
		signVal = Wait((1L << myWin->UserPort->mp_SigBit) | waitMask);
		
		if (waitMask & signVal)
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
			RectFill(myWin->RPort, 471, topBorder-14+46, 471+11, topBorder-14+46+(92-levelLeft));	
			RectFill(myWin->RPort, 495, topBorder-14+46, 495+11, topBorder-14+46+(92-levelRight));
			/* Draw Positive Bar */
			SetAPen(myWin->RPort, 3);
			RectFill(myWin->RPort, 471, topBorder-14+46+(92-levelLeft), 471+11, topBorder-14+46+92);	
			RectFill(myWin->RPort, 495, topBorder-14+46+(92-levelRight), 495+11, topBorder-14+46+92);
			

		}
		else if ((1L << myWin->UserPort->mp_SigBit) & signVal)
		{	
			while ((!terminated) &&
				   (imsg = GT_GetIMsg(myWin->UserPort)))
			{
				gad = (struct Gadget *)imsg->IAddress;

				imsgClass = imsg->Class;
				imsgCode = imsg->Code;


				GT_ReplyIMsg(imsg);

				switch (imsgClass)
				{

					case IDCMP_GADGETDOWN:
					case IDCMP_GADGETUP:					
					case IDCMP_MOUSEMOVE:					
						HandleGadgetEvent(myWin, gad, imsgCode, myGads, &terminated,mixDat,boardBase,cfgMem,intData);
						UpdateSliders(myWin, myGads,mixDat);
						
						break;
					case IDCMP_VANILLAKEY:
						HandleVanillaKey(myWin, imsgCode, myGads);
						break;
					case IDCMP_CLOSEWINDOW:
						terminated = TRUE;
						break;
					case IDCMP_REFRESHWINDOW:
					case IDCMP_ACTIVEWINDOW:
						UpdateSliders(myWin, myGads,mixDat);
						GT_BeginRefresh(myWin);
						GT_EndRefresh(myWin, TRUE);
						DrawBorders(myWin->RPort,topBorder,font);
					break;
				}
			}
		}
	}
}

void GadToolsWindow(void)
{
	struct TextFont 	*font;
	struct Screen   	*myScreen;
	struct Window   	*myWin;
	struct Gadget   	*gadList, *myGads[24];
	struct mixData		*mixDat;
	struct ConfigDev 	*myCD;
	
	struct Interrupt 	*mixerInt;
	struct intData 		*intData;		
	
	void            	*vi;

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
		InitCFGMem (cfgMem);
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


	WriteSPI(boardBase, 0x20, 0x1f);	// Enable ADC I2S Master Mode
	WriteSPI(boardBase, 0x26, 0x1);	// Set BCLK = CLK/2 (192kHz sampling rate)
	WriteSPI(boardBase, 0x19, 0xff);	// Set Manual Gain Control
	WriteSPI(boardBase, 0x01, 0x20);	// Increase Left Gain
	WriteSPI(boardBase, 0x02, 0x20);	// Increase Right Gain

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

	if (NULL == (font = OpenFont(&Topaz80)))
		printf("ERROR: Could not open topaz font\n");
	else
    {
		if (NULL == (myScreen = LockPubScreen(NULL)))
			printf("ERROR: Could not get lock on screen\n");
		else
        {
			if (NULL == (vi = GetVisualInfo(myScreen, TAG_END)))
				printf("ERROR: Could not get visual info\n");
			else
            {
				topBorder = myScreen->WBorTop + (myScreen->Font->ta_YSize + 1);

				if (NULL == CreateAllGadgets(&gadList, vi, topBorder, myGads))
					printf("ERROR: Could not create gadgets\n");
				else
				{
					if (NULL == (myWin = OpenWindowTags(NULL,
							WA_Title,     "AmiGUS Mixer V0.65 - (c)2025 by O. Achten",
							WA_Gadgets,   gadList,      WA_AutoAdjust,    TRUE,
							WA_Width,       528,      WA_MinWidth,        50,
							WA_InnerHeight, 154,      WA_MinHeight,       50,
							WA_DragBar,    TRUE,      WA_DepthGadget,   TRUE,
							WA_Activate,   TRUE,      WA_CloseGadget,   TRUE,
							WA_SizeGadget, FALSE,     WA_SmartRefresh, TRUE,
							WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |IDCMP_ACTIVEWINDOW|
								IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
								BUTTONIDCMP,
							WA_PubScreen, myScreen,
							TAG_END)))
						printf("ERROR: Could not open window\n");
					else
						{
						InitGadgets(myWin, myGads, mixDat,boardBase,cfgMem);
						DrawBorders(myWin->RPort,topBorder,font);
						
						GT_RefreshWindow(myWin, NULL);

						ProcessWindowEvents(myWin, myGads, font,topBorder,mixDat,boardBase,cfgMem,waitMask,intData);

						CloseWindow(myWin);
						}
					}
				FreeGadgets(gadList);
				FreeVisualInfo(vi);
            }
        UnlockPubScreen(NULL, myScreen);
        }
    CloseFont(font);
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
		printf( "Requires V37 intuition.library\n");
	else
	{
		if (NULL == (ExpansionBase = OpenLibrary("expansion.library", 34)))
			printf("Requires V37 expansion.library\n");
		else	
		{
			if (NULL == (GfxBase = OpenLibrary("graphics.library", 34)))
				printf("Requires V37 graphics.library\n");
			else
			{
				if (NULL == (GadToolsBase = OpenLibrary("gadtools.library", 34)))
					printf("Requires V37 gadtools.library\n");
				else			
				{
					GadToolsWindow();

					CloseLibrary(GadToolsBase);
				}
				CloseLibrary(GfxBase);
			}
			CloseLibrary(ExpansionBase);
		}
		CloseLibrary(IntuitionBase);	
	}
}
