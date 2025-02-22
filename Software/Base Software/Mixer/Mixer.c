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

#define INTUI_V36_NAMES_ONLY

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


/* Gadget defines of our choosing, to be used as GadgetID's,
** also used as the index into the gadget array my_gads[].
*/
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

struct MixData {
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

struct IntData {
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
	return 0x0;
#endif
}

WORD ReadReg16S(APTR base, ULONG offset)
{
#ifndef DEBUG	
	return *((WORD *)((ULONG)base+offset));
#else
	return 0x0;
#endif
}

ULONG ReadReg32(APTR base, ULONG offset)
{
#ifndef DEBUG
	return *((ULONG *)((ULONG)base+offset));
#else
	return 0x0;
#endif
}

void WriteSPI(APTR base, UWORD regnum, UWORD regval)
{
	while ((ReadReg16(base,MAIN_SPI_STATUS)&0x8000)!=0){};
	WriteReg16(base,MAIN_SPI_ADDRESS,regnum);
	WriteReg16(base,MAIN_SPI_WDATA,regval);
	WriteReg16(base,MAIN_SPI_WTRIG,0);
}

UWORD ReadSPI(APTR base, UWORD regnum)
{
	UWORD regval;
	while ((ReadReg16(base,MAIN_SPI_STATUS)&0x8000)!=0){};	
	WriteReg16(base,MAIN_SPI_ADDRESS,regnum);
	WriteReg16(base,MAIN_SPI_RDTRIG,0);
	while ((ReadReg16(base,MAIN_SPI_STATUS)&0x8000)!=0){};	
	regval = ReadReg16(base,MAIN_SPI_RDATA);
	return regval;
}


void setMixer(APTR base,struct MixData *mixdat)
{
	WriteReg16(base,MAIN_ADC_VOLUME_LL,mixdat->adc_vol_ll);
	WriteReg16(base,MAIN_ADC_VOLUME_RR,mixdat->adc_vol_rr);
	WriteReg16(base,MAIN_MHI_VOLUME_LL,mixdat->mhi_vol_ll);
	WriteReg16(base,MAIN_MHI_VOLUME_RR,mixdat->mhi_vol_rr);
	WriteReg16(base,MAIN_WAV_VOLUME_LL,mixdat->wav_vol_ll);
	WriteReg16(base,MAIN_WAV_VOLUME_RR,mixdat->wav_vol_rr);
	WriteReg16(base,MAIN_AHI_VOLUME_LL,mixdat->ahi_vol_ll);
	WriteReg16(base,MAIN_AHI_VOLUME_RR,mixdat->ahi_vol_rr);
	
	WriteReg16(base,MAIN_ADC_MIX_LR,mixdat->adc_mix_lr);
	WriteReg16(base,MAIN_MHI_MIX_LR,mixdat->mhi_mix_lr);
	WriteReg16(base,MAIN_WAV_MIX_LR,mixdat->wav_mix_lr);
	WriteReg16(base,MAIN_AHI_MIX_LR,mixdat->ahi_mix_lr);

	WriteReg16(base,MAIN_TOSLINK_CTRL,mixdat->toslink_srate);
	
	WriteSPI(base, 0x06, mixdat->adc_enable);	// Enable PAULA Left
	WriteSPI(base, 0x07, mixdat->adc_enable);	// Enable PAULA Right
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
	ULONG	memdata,flashoffset,status,cnt,length;
	
	flashoffset = (ULONG)(FLASH_CONFIG_START>>2);
	length = FLASH_CONFIG_SIZE;
	
	status = 0xffffffff;
	cnt = 0x0;
	
	WriteReg16(base, FLASH_CTRL_ADDR, 0x1);
	WriteReg32(base, FLASH_CTRL_WRITE_DATA, 0x0);				// Clear protection bits
	WriteReg16(base, FLASH_CTRL_WRITE_STROBE, 0x0);
	
	WriteReg16(base, FLASH_CTRL_ADDR, 0x0);
	
	do {
		memdata = *((ULONG *)((ULONG)memory+cnt));
		
		WriteReg16(base, FLASH_DATA_ADDR, flashoffset);
		WriteReg32(base, FLASH_DATA_WRITE_PORT, memdata);
		WriteReg16(base, FLASH_DATA_WRITE_STROBE, 0x0);
		
		do {
			WriteReg16(base, FLASH_CTRL_READ_STROBE, 0x0);	
		
			status = (ULONG)(ReadReg32(base, FLASH_CTRL_READ_DATA) & (ULONG)0x3);	// Read Status register
		
		} while (status != 0x0);		
		
		
		flashoffset++;
		cnt+=4;
	}
	while (cnt < length);
}

void drawBorders(struct RastPort *rp,UWORD topborder,struct TextFont *font)
{

  struct Border    shineBorder;
  struct Border    shadowBorder;
  
  struct IntuiText  myIText;
  struct TextAttr   myTextAttr;

  long mySHADOWPEN = 1;  /* set default values for pens */
  long mySHINEPEN  = 2;  /* in case can't get info...   */ 

	long myTEXTPEN = 1;
	long myBACKGROUNDPEN = 0;
	
	int startx = 12;

      shadowBorder.LeftEdge   = 0;
      shadowBorder.TopEdge    = 0;
      shadowBorder.FrontPen   = mySHADOWPEN;
      shadowBorder.NextBorder = &shineBorder;
	  
      shineBorder.LeftEdge    = 0 + 1;
      shineBorder.TopEdge     = 0 + 1;
      shineBorder.FrontPen    = mySHINEPEN;
      shineBorder.NextBorder  = NULL;	  

      shadowBorder.BackPen    = shineBorder.BackPen   = 0;
      shadowBorder.DrawMode   = shineBorder.DrawMode  = JAM1;
      shadowBorder.Count      = shineBorder.Count     = 7;
      shadowBorder.XY         = shineBorder.XY        = myBorderData;
	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+14);
	  startx +=92;
	  DrawBorder(rp,&shadowBorder,startx,topborder+14);
	  startx +=92;
	  DrawBorder(rp,&shadowBorder,startx,topborder+14);
	  startx +=92;
	  
	  
      shadowBorder.Count      = shineBorder.Count     = 9;
      shadowBorder.XY         = shineBorder.XY        = myBorderData2;	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+14);	
      shadowBorder.Count      = shineBorder.Count     = 5;	  	  
	  startx +=176;
      shadowBorder.XY         = shineBorder.XY        = myBorderData5;	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+14);	
	  startx +=4;
      shadowBorder.XY         = shineBorder.XY        = myBorderData6;	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+30);		  
	  startx +=24;
      shadowBorder.XY         = shineBorder.XY        = myBorderData6;	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+30);	
	  
	  shadowBorder.XY         = shineBorder.XY        = myBorderData3;	  
	  startx =12;
	  DrawBorder(rp,&shadowBorder,startx,topborder+114);
	  startx +=92;	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+114);
	  startx +=92;	  
	  DrawBorder(rp,&shadowBorder,startx,topborder+114);
	  startx +=92;
	  shadowBorder.XY         = shineBorder.XY        = myBorderData4;		  
	  DrawBorder(rp,&shadowBorder,startx,topborder+114);
	  
	  myTextAttr.ta_Name  = font->tf_Message.mn_Node.ln_Name;
      myTextAttr.ta_YSize = font->tf_YSize;
      myTextAttr.ta_Style = font->tf_Style;
      myTextAttr.ta_Flags = font->tf_Flags;
	  
	  myIText.FrontPen    = myTEXTPEN;
	  myIText.BackPen     = myBACKGROUNDPEN;
      myIText.DrawMode    = JAM2;
      myIText.LeftEdge    = 0;
      myIText.TopEdge     = 0;
      myIText.ITextFont   = &myTextAttr;
      myIText.IText       = "AHI Sound";
      myIText.NextText    = NULL;
	  
	  startx = 20;
	  PrintIText(rp,&myIText,startx,topborder+4);
	  startx +=92;
	  myIText.IText       = "MHI Sound";
	  PrintIText(rp,&myIText,startx,topborder+4);
	  startx +=88;
	  myIText.IText       = "Wave Sound";
	  PrintIText(rp,&myIText,startx,topborder+4);	  
	  startx +=118;
	  myIText.IText       = "External Sound";
	  PrintIText(rp,&myIText,startx,topborder+4);	
	  startx +=146;
	  myIText.IText       = "Levels";
	  PrintIText(rp,&myIText,startx,topborder+4);	
	  startx+=8;
	  myIText.IText       = "L  R";
	  PrintIText(rp,&myIText,startx,topborder+20);	
		startx-=82;
	  myIText.IText       = "Inputs";
	  PrintIText(rp,&myIText,startx,topborder+20);
		startx-=4;
	  myIText.IText       = "TOSLINK";
	  PrintIText(rp,&myIText,startx,topborder+84);		  
}

/* Print any error message.  We could do more fancy handling (like
** an EasyRequest()), but this is only a demo.
*/
void errorMessage(STRPTR error)
{
if (error)
    printf("Error: %s\n", error);
}

void initCfgMem (APTR cfg_mem)
{
	ULONG cnt = 0;
		
	do {
		*((ULONG *)((ULONG)cfg_mem+cnt)) = 0xffffffff;
		cnt+=4;
	} while (cnt != 0x4000);
		
	*((ULONG *)((ULONG)cfg_mem+0x0000)) = 0x414d4947;	// Magic Token - Unlock

/* Fix ADC Initialisation */
	
	// Enable ADC I2S Master Mode

	*((ULONG *)((ULONG)cfg_mem+0x0004)) = 0x00200020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0008)) = 0x001f0022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x000c)) = 0x00000024;	// MAIN_SPI_WTRIG
		
	// Set BCLK = CLK/2 (192kHz sampling rate)
		
	*((ULONG *)((ULONG)cfg_mem+0x0010)) = 0x00260020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0014)) = 0x00010022;	// MAIN_SPI_WDATA = regval
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

/* End of Stream */
	*((ULONG *)((ULONG)cfg_mem+0x008c)) = 0xffffffff;
}


void updateSliders(struct Window *win, struct Gadget *my_gads[], struct MixData *mixdat)
{
	UWORD regval;
	
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->ahi_vol_ll * (ULONG)100), (ULONG)66196));
	GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_L], win, NULL,
                           GTSL_Level, regval,
                            TAG_END);
	
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->ahi_vol_rr * (ULONG)100), (ULONG)66196));	
    GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_R], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);
	
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->mhi_vol_ll * (ULONG)100), (ULONG)66196));
    GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_L], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);

	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->mhi_vol_rr * (ULONG)100), (ULONG)66196));		
	GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_R], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);   
	
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->wav_vol_ll * (ULONG)100), (ULONG)66196));
	GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_L], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);
							
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->wav_vol_rr * (ULONG)100), (ULONG)66196));					
    GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_R], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);

	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->adc_vol_ll * (ULONG)100), (ULONG)66196));					
    GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_L], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);
					
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->adc_vol_rr * (ULONG)100), (ULONG)66196));							
    GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_R], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);	
							
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->ahi_mix_lr * (ULONG)100), (ULONG)66196));			
	GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_MIX], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);
							
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->mhi_mix_lr * (ULONG)100), (ULONG)66196));	
	GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_MIX], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);

	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->wav_mix_lr * (ULONG)100), (ULONG)66196));	
    GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_MIX], win, NULL,
                            GTSL_Level, regval,
                            TAG_END);
    
	regval = (UWORD)(DIV_ROUND_CLOSEST((ULONG)((ULONG)mixdat->adc_mix_lr * (ULONG)100), (ULONG)66196));	
	GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_MIX], win, NULL,
                            GTSL_Level, regval,
							TAG_END);
}

void initGadgets(struct Window *win, struct Gadget *my_gads[], struct MixData *mixdat, APTR base, APTR cfg_mem)
{
	UWORD regval,regval2;

	/* Initialise config structure from flash */
	
	regval = ReadReg16(base,MAIN_ADC_VOLUME_LL);
	mixdat->adc_vol_ll = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0058)) = 0x00000030 | (ULONG)((ULONG)regval << 16);	// MAIN_ADC_VOLUME_LL
		
	regval = ReadReg16(base,MAIN_ADC_VOLUME_RR);
	mixdat->adc_vol_rr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x005C)) = 0x00000032 | (ULONG)((ULONG)regval << 16);	// MAIN_ADC_VOLUME_RR
	
	regval = ReadReg16(base,MAIN_MHI_VOLUME_LL);
	mixdat->mhi_vol_ll = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0060)) = 0x00000034 | (ULONG)((ULONG)regval << 16);	// MAIN_MHI_VOLUME_LL
		
	regval = ReadReg16(base,MAIN_MHI_VOLUME_RR);
	mixdat->mhi_vol_rr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0064)) = 0x00000036 | (ULONG)((ULONG)regval << 16);	// MAIN_MHI_VOLUME_RR	
	
	regval = ReadReg16(base,MAIN_WAV_VOLUME_LL);
	mixdat->wav_vol_ll = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0068)) = 0x00000038 | (ULONG)((ULONG)regval << 16);	// MAIN_WAV_VOLUME_LL
		
	regval = ReadReg16(base,MAIN_WAV_VOLUME_RR);
	mixdat->wav_vol_rr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x006c)) = 0x0000003a | (ULONG)((ULONG)regval << 16);	// MAIN_WAV_VOLUME_RR
	
	regval = ReadReg16(base,MAIN_AHI_VOLUME_LL);
	mixdat->ahi_vol_ll = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0070)) = 0x0000003c | (ULONG)((ULONG)regval << 16);	// MAIN_AHI_VOLUME_LL
		
	regval = ReadReg16(base,MAIN_AHI_VOLUME_RR);
	mixdat->ahi_vol_rr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0074)) = 0x0000003e | (ULONG)((ULONG)regval << 16);	// MAIN_AHI_VOLUME_RR

	regval = ReadReg16(base,MAIN_ADC_MIX_LR);
	mixdat->adc_mix_lr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0078)) = 0x00000040 | (ULONG)((ULONG)regval << 16);	
	
	regval = ReadReg16(base,MAIN_MHI_MIX_LR);
	mixdat->mhi_mix_lr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x007C)) = 0x00000042 | (ULONG)((ULONG)regval << 16);	
	
	regval = ReadReg16(base,MAIN_WAV_MIX_LR);
	mixdat->wav_mix_lr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0080)) = 0x00000044 | (ULONG)((ULONG)regval << 16);	
	
	regval = ReadReg16(base,MAIN_AHI_MIX_LR);
	mixdat->ahi_mix_lr = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0084)) = 0x00000046 | (ULONG)((ULONG)regval << 16);	
	
	/* Initialise sliders from config structure */
	
	updateSliders(win,my_gads,mixdat);
	
	/* Initialise checkboxes from ADC settings */
							
	regval = (ReadSPI(base,0x06)&0xe);
	mixdat->adc_enable = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)regval << 16);
	*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)regval << 16);
	
	if ((regval & 0x2) == 0x02)
	{	
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_PAULA], win, NULL,
								GTCB_Checked, 1,
								TAG_END);
	}
	else
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_PAULA], win, NULL,
								GTCB_Checked, 0,
								TAG_END);
	}
	
	if ((regval & 0x4) == 0x04)
	{	
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_CDROM], win, NULL,
								GTCB_Checked, 1,
								TAG_END);
	}
	else
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_CDROM], win, NULL,
								GTCB_Checked, 0,
								TAG_END);
	}
	
	if ((regval & 0x8) == 0x08)
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_LINE], win, NULL,
								GTCB_Checked, 1,
								TAG_END);
	}
	else
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_LINE], win, NULL,
								GTCB_Checked, 0,
								TAG_END);
	}
	
	if (mixdat->ahi_vol_ll == mixdat->ahi_vol_rr)
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_AHI], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixdat->ahi_locked = TRUE;
	}
	else
	{
		mixdat->ahi_locked = FALSE;
	}
	
	if (mixdat->mhi_vol_ll == mixdat->mhi_vol_rr)
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_MHI], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixdat->mhi_locked = TRUE;							
	}
	else
	{
		mixdat->mhi_locked = FALSE;
	}

	if (mixdat->wav_vol_ll == mixdat->wav_vol_rr)
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_HGN], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixdat->wav_locked = TRUE;					
	}
	else
	{
		mixdat->wav_locked = FALSE;
	}	

	if (mixdat->adc_vol_ll == mixdat->adc_vol_rr)
	{
		GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_ADC], win, NULL,
							GTCB_Checked, 1,
							TAG_END);
		mixdat->adc_locked = TRUE;
	}
	else
	{
		mixdat->adc_locked = FALSE;
	}	

	/* Initialise Cycle Gadget from TOSLINK settings */
	
	regval = ReadReg16(base,MAIN_TOSLINK_CTRL);
	mixdat->toslink_srate = regval;
	*((ULONG *)((ULONG)cfg_mem+0x0088)) = 0x00000070 | (ULONG)((ULONG)regval << 16);	// MAIN_TOSLINK_CTRL
	
	GT_SetGadgetAttrs(my_gads[MYGAD_CYCLE_TOSLINK], win, NULL,
				GTCY_Active, mixdat->toslink_srate,
				TAG_END);	
}

/*
** Function to handle a GADGETUP or GADGETDOWN event.  For GadTools gadgets,
** it is possible to use this function to handle MOUSEMOVEs as well, with
** little or no work.
*/

VOID handleGadgetEvent(struct Window *win, struct Gadget *gad, UWORD code,
    WORD *slider_level, struct Gadget *my_gads[], BOOL *terminated,struct MixData *mixdat,APTR board_base,APTR cfg_mem,struct IntData *intdata)
{
switch (gad->GadgetID)
    {
    case MYGAD_SLIDER_AHI_L:
		mixdat->ahi_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x0070)) = 0x0000003c | (ULONG)((ULONG)mixdat->ahi_vol_ll << 16);	// MAIN_AHI_VOLUME_LL
		if (mixdat->ahi_locked == TRUE)
		{
			mixdat->ahi_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x0074)) = 0x0000003c | (ULONG)((ULONG)mixdat->ahi_vol_rr << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_AHI_R:
		mixdat->ahi_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x0074)) = 0x0000003e | (ULONG)((ULONG)mixdat->ahi_vol_rr << 16);	// MAIN_AHI_VOLUME_RR
		
		if (mixdat->ahi_locked == TRUE)
		{
			mixdat->ahi_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x0070)) = 0x0000003c | (ULONG)((ULONG)mixdat->ahi_vol_ll << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_AHI_MIX:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->ahi_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfg_mem+0x0084)) = 0x00000046 | (ULONG)((ULONG)mixdat->ahi_mix_lr << 16);	// MAIN_AHI_MIX_LR
        break;
    case MYGAD_SLIDER_MHI_L:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->mhi_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x0060)) = 0x00000034 | (ULONG)((ULONG)mixdat->mhi_vol_ll << 16);	// MAIN_MHI_VOLUME_LL
		
		if (mixdat->mhi_locked == TRUE)
		{
			mixdat->mhi_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x0064)) = 0x0000003c | (ULONG)((ULONG)mixdat->mhi_vol_rr << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_MHI_R:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->mhi_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x0064)) = 0x00000036 | (ULONG)((ULONG)mixdat->mhi_vol_rr << 16);	// MAIN_MHI_VOLUME_RR
		
		if (mixdat->mhi_locked == TRUE)
		{
			mixdat->mhi_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x0060)) = 0x0000003c | (ULONG)((ULONG)mixdat->mhi_vol_ll << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_MHI_MIX:
       /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->mhi_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfg_mem+0x007c)) = 0x00000042 | (ULONG)((ULONG)mixdat->mhi_mix_lr << 16);	// MAIN_MHI_MIX_LR
        //printf("Slider at level %lx, %lx\n", mixdat->mhi_vol_lr,mixdat->mhi_vol_rl);
        break;	
    case MYGAD_SLIDER_HGN_L:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->wav_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x0068)) = 0x00000038 | (ULONG)((ULONG)mixdat->wav_vol_ll << 16);	// MAIN_WAV_VOLUME_LL
		
		if (mixdat->wav_locked == TRUE)
		{
			mixdat->wav_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x006c)) = 0x0000003c | (ULONG)((ULONG)mixdat->wav_vol_rr << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_HGN_R:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->wav_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x006c)) = 0x0000003a | (ULONG)((ULONG)mixdat->wav_vol_rr << 16);	// MAIN_WAV_VOLUME_RR
		
		if (mixdat->wav_locked == TRUE)
		{
			mixdat->wav_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x0068)) = 0x0000003c | (ULONG)((ULONG)mixdat->wav_vol_ll << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_HGN_MIX:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->wav_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfg_mem+0x0080)) = 0x00000044 | (ULONG)((ULONG)mixdat->wav_mix_lr << 16);	// MAIN_WAV_MIX_LR
        //printf("Slider at level %lx, %lx\n", mixdat->wav_vol_lr,mixdat->wav_vol_rl);
        break;			
    case MYGAD_SLIDER_ADC_L:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->adc_vol_ll = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x0058)) = 0x00000030 | (ULONG)((ULONG)mixdat->adc_vol_ll << 16);	// MAIN_ADC_VOLUME_LL
		
		if (mixdat->adc_locked == TRUE)
		{
			mixdat->adc_vol_rr = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x005c)) = 0x0000003c | (ULONG)((ULONG)mixdat->adc_vol_rr << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_R], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;
    case MYGAD_SLIDER_ADC_R:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->adc_vol_rr = (UWORD)((66196*code)/100);
		*((ULONG *)((ULONG)cfg_mem+0x005c)) = 0x00000032 | (ULONG)((ULONG)mixdat->adc_vol_rr << 16);	// MAIN_ADC_VOLUME_RR
		
		if (mixdat->adc_locked == TRUE)
		{
			mixdat->adc_vol_ll = (UWORD)((66196*code)/100);
			*((ULONG *)((ULONG)cfg_mem+0x0058)) = 0x0000003c | (ULONG)((ULONG)mixdat->adc_vol_ll << 16);

			 GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_L], win, NULL,
				GTSL_Level, code,
				TAG_END);
		}
        break;		
    case MYGAD_SLIDER_ADC_MIX:
        /* Sliders report their level in the IntuiMessage Code field: */
		mixdat->adc_mix_lr = (UWORD)((66196*code)/100);		
		*((ULONG *)((ULONG)cfg_mem+0x0078)) = 0x00000040 | (ULONG)((ULONG)mixdat->adc_mix_lr << 16);	// MAIN_WAV_MIX_LR	
        break;		
    case MYGAD_CHKBOX_PAULA:
		if (code == 1)
		{
			mixdat->adc_enable |= 0x02;
			*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);
			*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);		
		}
		else
		{			
			mixdat->adc_enable &= 0xfd;
			*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);
			*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);			
		}
		break;
    case MYGAD_CHKBOX_CDROM:
		if (code == 1)
		{
			mixdat->adc_enable |= 0x04;
			*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);
			*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);			
		}	
		else
		{			
			mixdat->adc_enable &= 0xfb;
			*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);
			*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);			
		}		
		break;
    case MYGAD_CHKBOX_LINE:
		if (code == 1)
		{
			mixdat->adc_enable |= 0x08;
			*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);
			*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);			
		}
		else
		{			
			mixdat->adc_enable &= 0xf7;	
			*((ULONG *)((ULONG)cfg_mem+0x0044)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);
			*((ULONG *)((ULONG)cfg_mem+0x0050)) = 0x00000022 | (ULONG)((ULONG)mixdat->adc_enable << 16);			
		}
		break;
    case MYGAD_CHKBOX_AHI:
		if (code == 1)
		{
			mixdat->ahi_locked = TRUE;
		}
		else
		{
			mixdat->ahi_locked = FALSE;
		}
		break;
    case MYGAD_CHKBOX_MHI:
		if (code == 1)
		{
			mixdat->mhi_locked = TRUE;
		}
		else
		{
			mixdat->mhi_locked = FALSE;
		}		
		break;
    case MYGAD_CHKBOX_HGN:
		if (code == 1)
		{
			mixdat->wav_locked = TRUE;
		}
		else
		{
			mixdat->wav_locked = FALSE;
		}		
		break;		
    case MYGAD_CHKBOX_ADC:
		if (code == 1)
		{
			mixdat->adc_locked = TRUE;
		}
		else
		{
			mixdat->adc_locked = FALSE;
		}		
		break;		
    case MYGAD_BUTTON_SAVE:
		EraseFlash(board_base);
		ProgramFlash(board_base,cfg_mem);
		break;
    case MYGAD_BUTTON_USE:
		*terminated = TRUE;
		break;
    case MYGAD_BUTTON_RESET:
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_L], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_R], win, NULL,
                            GTSL_Level, 50,
                            TAG_END);	
							
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_MHI_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_HGN_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_ADC_MIX], win, NULL,
                            GTSL_Level, 0,
                            TAG_END);
							
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_PAULA], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);		
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_CDROM], win, NULL,
                            GTCB_Checked, 0,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_LINE], win, NULL,
                            GTCB_Checked, 0,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_AHI], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_MHI], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_HGN], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
       GT_SetGadgetAttrs(my_gads[MYGAD_CHKBOX_ADC], win, NULL,
                            GTCB_Checked, 1,
                            TAG_END);
		
		GT_SetGadgetAttrs(my_gads[MYGAD_CYCLE_TOSLINK], win, NULL,
				GTCY_Active, 0,
				TAG_END);
				
		initCfgMem (cfg_mem);					
		mixdat->ahi_vol_ll = 0x8000;
		mixdat->ahi_vol_rr = 0x8000;
		mixdat->mhi_vol_ll = 0x8000;
		mixdat->mhi_vol_rr = 0x8000;
		mixdat->wav_vol_ll = 0x8000;
		mixdat->wav_vol_rr = 0x8000;
		mixdat->adc_vol_ll = 0x8000;
		mixdat->adc_vol_rr = 0x8000;
	
		mixdat->ahi_mix_lr = 0x0000;
		mixdat->mhi_mix_lr = 0x0000;
		mixdat->wav_mix_lr = 0x0000;
		mixdat->adc_mix_lr = 0x0000;		
		
		mixdat->adc_enable = 0x0002;
		
		mixdat->toslink_srate = 0x0;

		mixdat->ahi_locked = TRUE;
		mixdat->mhi_locked = TRUE;
		mixdat->wav_locked = TRUE;
		mixdat->adc_locked = TRUE;		
        break;
	case MYGAD_CYCLE_LEVELS:
		switch (code)
		{
			case 0:
				intdata->int_rate = 0;
				intdata->counter = 0;
				break;
			case 1:
				intdata->int_rate = 5;
				intdata->counter = 0;			
				break;
			case 2:
				intdata->int_rate = 3;
				intdata->counter = 0;			
				break;
			case 3:
				intdata->int_rate = 1;
				intdata->counter = 0;			
				break;
		}
		break;
	case MYGAD_CYCLE_TOSLINK:
		mixdat->toslink_srate = (UWORD)code;
		*((ULONG *)((ULONG)cfg_mem+0x0088)) = 0x00000070 |(ULONG)((ULONG)mixdat->toslink_srate << 16);	// MAIN_TOSLINK_CTRL
		break;
	
    }
	setMixer(board_base,mixdat);
}


/*
** Function to handle vanilla keys.
*/
VOID handleVanillaKey(struct Window *win, UWORD code,
    WORD *slider_level, struct Gadget *my_gads[])
{
switch (code)
    {
    case 'v':
        /* increase slider level, but not past maximum */
        if (++*slider_level > SLIDER_MAX)
            *slider_level = SLIDER_MAX;
        GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_L], win, NULL,
                            GTSL_Level, *slider_level,
                            TAG_END);
        break;
    case 'V':
        /* decrease slider level, but not past minimum */
        if (--*slider_level < SLIDER_MIN)
            *slider_level = SLIDER_MIN;
        GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_L], win, NULL,
                            GTSL_Level, *slider_level,
                            TAG_END);
        break;
    case 'c':
    case 'C':
        /* button resets slider to 10 */
        *slider_level = 10;
        GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER_AHI_L], win, NULL,
                            GTSL_Level, *slider_level,
                            TAG_END);
        break;
    }
}


/*
** Here is where all the initialization and creation of GadTools gadgets
** take place.  This function requires a pointer to a NULL-initialized
** gadget list pointer.  It returns a pointer to the last created gadget,
** which can be checked for success/failure.
*/
struct Gadget *createAllGadgets(struct Gadget **glistptr, void *vi,
    UWORD topborder, WORD slider_level, struct Gadget *my_gads[])
{
	struct NewGadget ng;
	struct Gadget *gad;

   static const char *levels_options[] =
    {
	"OFF",
	"SLOW",
	"NORM",
	"FAST",
	NULL
    };
	
	static const char *toslink_options[] =
    {
	"48kHz",
	"96kHz",
	"192kHz",
	NULL
    };

	/* All the gadget creation calls accept a pointer to the previous gadget, and
	** link the new gadget to that gadget's NextGadget field.  Also, they exit
	** gracefully, returning NULL, if any previous gadget was NULL.  This limits
	** the amount of checking for failure that is needed.  You only need to check
	** before you tweak any gadget structure or use any of its fields, and finally
	** once at the end, before you add the gadgets.
	*/

	/* The following operation is required of any program that uses GadTools.
	** It gives the toolkit a place to stuff context data.
	*/
	gad = CreateContext(glistptr);

	/* Since the NewGadget structure is unmodified by any of the CreateGadget()
	** calls, we need only change those fields which are different.
	*/
	ng.ng_LeftEdge   = 20;
	ng.ng_TopEdge    = 32+topborder;
	ng.ng_Width      = 16;
	ng.ng_Height     = 64;
	ng.ng_GadgetText = "L";
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID   = MYGAD_SLIDER_AHI_L;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_ABOVE;

	my_gads[MYGAD_SLIDER_AHI_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_AHI_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_AHI_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_MHI_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_MHI_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_MHI_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_HGN_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_HGN_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_HGN_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_ADC_L] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_ADC_R] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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

	my_gads[MYGAD_SLIDER_ADC_MIX] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
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
	my_gads[MYGAD_CHKBOX_PAULA] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);
						
	ng.ng_LeftEdge  += 0;
	ng.ng_TopEdge   += 16;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "CDROM";
	ng.ng_GadgetID   = MYGAD_CHKBOX_CDROM;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	my_gads[MYGAD_CHKBOX_CDROM] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);	
						
	ng.ng_LeftEdge  += 0;
	ng.ng_TopEdge   += 16;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Line";
	ng.ng_GadgetID   = MYGAD_CHKBOX_LINE;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	my_gads[MYGAD_CHKBOX_LINE] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);	

	ng.ng_LeftEdge  = 24;
	ng.ng_TopEdge   = topborder + 132-14;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_AHI;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	my_gads[MYGAD_CHKBOX_AHI] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);							

	ng.ng_LeftEdge  += 92;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_MHI;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	my_gads[MYGAD_CHKBOX_MHI] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);

	ng.ng_LeftEdge  += 92;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_HGN;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	my_gads[MYGAD_CHKBOX_HGN] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);
						
	ng.ng_LeftEdge  += 92;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 100;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "Lock";
	ng.ng_GadgetID   = MYGAD_CHKBOX_ADC;
	ng.ng_Flags      = NG_HIGHLABEL | PLACETEXT_RIGHT;
	my_gads[MYGAD_CHKBOX_ADC] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
						TAG_END);

	/* ========== Button Gadgets =========== */

	ng.ng_LeftEdge  = 98;
	ng.ng_TopEdge   += 20;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "_Save";
	ng.ng_GadgetID   = MYGAD_BUTTON_SAVE;
	ng.ng_Flags      = 0;
	my_gads[MYGAD_BUTTON_SAVE] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
						GT_Underscore, '_',
						TAG_END);
						
	ng.ng_LeftEdge  +=96;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "_Use";
	ng.ng_GadgetID   = MYGAD_BUTTON_USE;
	ng.ng_Flags      = 0;
	my_gads[MYGAD_BUTTON_USE] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
						GT_Underscore, '_',
						TAG_END);
						
	ng.ng_LeftEdge  += 96;
	ng.ng_TopEdge   += 0;
	ng.ng_Width      = 72;
	ng.ng_Height     = 12;
	ng.ng_GadgetText = "_Reset";
	ng.ng_GadgetID   = MYGAD_BUTTON_RESET;
	ng.ng_Flags      = 0;
	my_gads[MYGAD_BUTTON_RESET] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
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
	my_gads[MYGAD_CYCLE_LEVELS] = gad = CreateGadget(CYCLE_KIND, gad, &ng,
						GTCY_Labels, (ULONG)levels_options,
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
	my_gads[MYGAD_CYCLE_TOSLINK] = gad = CreateGadget(CYCLE_KIND, gad, &ng,
						GTCY_Labels, (ULONG)toslink_options,
						GTCY_Active, 0,
						GA_Disabled, FALSE,
						TAG_END);							
						
	return(gad);
}

/*
** Standard message handling loop with GadTools message handling functions
** used (GT_GetIMsg() and GT_ReplyIMsg()).MHI
*/
VOID process_window_events(struct Window *mywin,
    WORD *slider_level, struct Gadget *my_gads[],struct TextFont *font,UWORD topborder,struct MixData *mixdat,APTR board_base,APTR cfg_mem,ULONG WaitMask,struct IntData *intdata)
{
	struct IntuiMessage *imsg;
	ULONG imsgClass;
	UWORD imsgCode;
	struct Gadget *gad;
	BOOL terminated = FALSE;
	ULONG Temp;
	WORD regvals;
	ULONG level_left = 0;
	ULONG level_right = 0;
	
	while (!terminated)
	{
		Temp = Wait((1L << mywin->UserPort->mp_SigBit) | WaitMask);
		
		if (WaitMask & Temp)
		{
			#ifdef DEBUG
				if (level_left >= 92)
					level_left = 0;
				else
					level_left+=1;
				
				if (level_right >= 92)
					level_right = 0;
				else
					level_right+=2;
			#else
				WriteReg16(board_base,MAIN_LEVELS_RESET,0);
			
				regvals = ReadReg16S(board_base,MAIN_LEVELS_LEFT);
				if (regvals < 0)
					regvals *= -1;
				level_left = regvals / 356;
				
				regvals = ReadReg16S(board_base,MAIN_LEVELS_RIGHT);	
				if (regvals < 0)
					regvals *= -1;
				
				level_right = regvals / 356;
			#endif
				
			/* Draw Negative Bar */
			SetAPen(mywin->RPort, 0);
			RectFill(mywin->RPort, 471, topborder-14+46, 471+11, topborder-14+46+(92-level_left));	
			RectFill(mywin->RPort, 495, topborder-14+46, 495+11, topborder-14+46+(92-level_right));
			/* Draw Positive Bar */
			SetAPen(mywin->RPort, 3);
			RectFill(mywin->RPort, 471, topborder-14+46+(92-level_left), 471+11, topborder-14+46+92);	
			RectFill(mywin->RPort, 495, topborder-14+46+(92-level_right), 495+11, topborder-14+46+92);
			

		}
		else if ((1L << mywin->UserPort->mp_SigBit) & Temp)
		{	
			/* GT_GetIMsg() returns an IntuiMessage with more friendly information for
			** complex gadget classes.  Use it wherever you get IntuiMessages where
			** using GadTools gadgets.
			*/
			while ((!terminated) &&
				   (imsg = GT_GetIMsg(mywin->UserPort)))
			{
				/* Presuming a gadget, of course, but no harm...
				** Only dereference this value (gad) where the Class specifies
				** that it is a gadget event.
				*/
				gad = (struct Gadget *)imsg->IAddress;

				imsgClass = imsg->Class;
				imsgCode = imsg->Code;

				/* Use the toolkit message-replying function here... */
				GT_ReplyIMsg(imsg);

				switch (imsgClass)
					{
					/*  --- WARNING --- WARNING --- WARNING --- WARNING --- WARNING ---
					** GadTools puts the gadget address into IAddress of IDCMP_MOUSEMOVE
					** messages.  This is NOT true for standard Intuition messages,
					** but is an added feature of GadTools.
					*/
					case IDCMP_GADGETDOWN:
					case IDCMP_GADGETUP:					
					case IDCMP_MOUSEMOVE:					
						handleGadgetEvent(mywin, gad, imsgCode, slider_level, my_gads, &terminated,mixdat,board_base,cfg_mem,intdata);
						updateSliders(mywin, my_gads,mixdat);
						
						break;
					case IDCMP_VANILLAKEY:
						handleVanillaKey(mywin, imsgCode, slider_level, my_gads);
						break;
					case IDCMP_CLOSEWINDOW:
						terminated = TRUE;
						break;
					case IDCMP_REFRESHWINDOW:
					case IDCMP_ACTIVEWINDOW:
						/* With GadTools, the application must use GT_BeginRefresh()
						** where it would normally have used BeginRefresh()
						*/
						updateSliders(mywin, my_gads,mixdat);
						GT_BeginRefresh(mywin);
						GT_EndRefresh(mywin, TRUE);
						drawBorders(mywin->RPort,topborder,font);
					break;
				}
			}
		}
	}
}

/*
** Prepare for using GadTools, set up gadgets and open window.
** Clean up and when done or on error.
*/
VOID gadtoolsWindow(VOID)
{
	struct TextFont *font;
	struct Screen   *mysc;
	struct Window   *mywin;
	struct Gadget   *glist, *my_gads[24];
	void            *vi;
	WORD            slider_level = 50;
	UWORD           topborder;
	struct MixData			*mixdat;
	struct ConfigDev *myCD;
	
	struct Interrupt *levelsint;
	struct IntData *intdata;	
	APTR	cfg_mem;
	
	UBYTE	board_product_id;
	UWORD	board_manufacturer_id;
	APTR	board_base;
	
	ULONG	WaitMask;
	
	BYTE	signr;
	
#ifndef DEBUG
	BOOL	board_found = FALSE;
#else
	BOOL	board_found = TRUE;
#endif

	/* ================ Find AmiGUS card ================ */
	
	myCD = NULL;
    while(myCD=FindConfigDev(myCD,-1L,-1L)) /* search for all ConfigDevs */	
	{
		board_manufacturer_id = myCD->cd_Rom.er_Manufacturer;
		board_product_id = myCD->cd_Rom.er_Product;
		board_base = myCD->cd_BoardAddr;
		if (board_manufacturer_id == AMIGUS_MANUFACTURER_ID && board_product_id == AMIGUS_MAIN_PRODUCT_ID)
		{
			board_found = TRUE;
			break;
		}
	}
	
	if (board_found == TRUE)
	{
		//printf("AmiGUS found at $%lx\n",board_base);
	}
	else
	{
		printf("ERROR: no AmiGUS board found!\n");	
		return;
	}	

	if (cfg_mem = AllocMem(FLASH_CONFIG_SIZE,MEMF_ANY))
    {
		initCfgMem (cfg_mem);
	}
	else 
	{
		return;
	}
	
	if (mixdat = AllocMem(sizeof(struct MixData), MEMF_PUBLIC|MEMF_CLEAR))
    {
		mixdat->ahi_vol_ll = 0x8000;
		mixdat->ahi_vol_rr = 0x8000;
		mixdat->mhi_vol_ll = 0x8000;
		mixdat->mhi_vol_rr = 0x8000;	
		mixdat->wav_vol_ll = 0x8000;
		mixdat->wav_vol_rr = 0x8000;
		mixdat->adc_vol_ll = 0x8000;
		mixdat->adc_vol_rr = 0x8000;
		mixdat->ahi_mix_lr = 0x0000;
		mixdat->mhi_mix_lr = 0x0000;
		mixdat->wav_mix_lr = 0x0000;
		mixdat->adc_mix_lr = 0x0000;
		
		mixdat->toslink_srate = 0x0000;
		
		mixdat->adc_enable = 0x2;		
	}
	else
	{
		return 0;
	}	

	// Check if reset sequence of AmiGUS was successful
	if ((ReadReg16(board_base,FLASH_CONFIG_STATUS)&0x8000) == 0x8000)
	{
		EraseFlash(board_base);
		ProgramFlash(board_base,cfg_mem);
		setMixer(board_base,mixdat);		
	}

	/* ================ Configure ADC ================ */


	WriteSPI(board_base, 0x20, 0x1f);	// Enable ADC I2S Master Mode
	WriteSPI(board_base, 0x26, 0x1);	// Set BCLK = CLK/2 (192kHz sampling rate)
	WriteSPI(board_base, 0x19, 0xff);	// Set Manual Gain Control
	WriteSPI(board_base, 0x01, 0x20);	// Increase Left Gain
	WriteSPI(board_base, 0x02, 0x20);	// Increase Right Gain

	/* ================== Interrupt Routine =============== */
	
	if ((signr = AllocSignal(-1)) == -1)          /* Allocate a signal bit for the   */
    {                                             /* interrupt handler to signal us. */
        printf("ERROR: can't allocate signal\n");
        return 0;			
	}

	if (intdata = AllocMem(sizeof(struct IntData), MEMF_PUBLIC|MEMF_CLEAR))
    {
		intdata->int_rate = 5;
		intdata->counter = 0;
		intdata->rd_Signal = 1L << signr;
		intdata->rd_Task = FindTask(NULL);		
	}
	else 
	{
		FreeSignal(signr);
        printf("ERROR: can't allocate memory for interrupt node\n");
        return 0;		
	}

	WaitMask = intdata->rd_Signal;
	
    if (levelsint = AllocMem(sizeof(struct Interrupt), 
                         MEMF_PUBLIC|MEMF_CLEAR))
    {
		levelsint->is_Node.ln_Type = NT_INTERRUPT;
        levelsint->is_Node.ln_Pri = -60;
        levelsint->is_Node.ln_Name = "AmiGUS-Mixer";
        levelsint->is_Data = (APTR)intdata;
        levelsint->is_Code = intServer;
		
		AddIntServer(INTB_VERTB, levelsint);
	}
	else 
	{
        printf("ERROR: can't allocate memory for interrupt node\n");
		FreeMem(intdata,sizeof(struct IntData));
		FreeSignal(signr);
		
        return 0;		
	}
	
/* Open topaz 8 font, so we can be sure it's openable
** when we later set ng_TextAttr to &Topaz80:
*/
	if (NULL == (font = OpenFont(&Topaz80)))
		errorMessage( "Failed to open Topaz 80");
	else
    {
		if (NULL == (mysc = LockPubScreen(NULL)))
			errorMessage( "Couldn't lock default public screen");
		else
        {
			if (NULL == (vi = GetVisualInfo(mysc, TAG_END)))
				errorMessage( "GetVisualInfo() failed");
			else
            {
				/* Here is how we can figure out ahead of time how tall the  */
				/* window's title bar will be:                               */
				topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);

				if (NULL == createAllGadgets(&glist, vi, topborder,
											 slider_level, my_gads))
					errorMessage( "createAllGadgets() failed");
				else
					{
					if (NULL == (mywin = OpenWindowTags(NULL,
							WA_Title,     "AmiGUS Mixer V0.63 - (c)2025 by O. Achten",
							WA_Gadgets,   glist,      WA_AutoAdjust,    TRUE,
							WA_Width,       528,      WA_MinWidth,        50,
							WA_InnerHeight, 154,      WA_MinHeight,       50,
							WA_DragBar,    TRUE,      WA_DepthGadget,   TRUE,
							WA_Activate,   TRUE,      WA_CloseGadget,   TRUE,
							WA_SizeGadget, FALSE,     WA_SmartRefresh, TRUE,
							WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |IDCMP_ACTIVEWINDOW|
								IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
								BUTTONIDCMP,
							WA_PubScreen, mysc,
							TAG_END)))
						errorMessage( "OpenWindow() failed");
					else
						{
						/* After window is open, gadgets must be refreshed with a
						** call to the GadTools refresh window function.
						*/
						initGadgets(mywin, my_gads, mixdat,board_base,cfg_mem);
						drawBorders(mywin->RPort,topborder,font);
						
						GT_RefreshWindow(mywin, NULL);

						process_window_events(mywin, &slider_level, my_gads, font,topborder,mixdat,board_base,cfg_mem,WaitMask,intdata);

						CloseWindow(mywin);
						}
					}
				/* FreeGadgets() even if createAllGadgets() fails, as some
				** of the gadgets may have been created...If glist is NULL
				** then FreeGadgets() will do nothing.
				*/
				FreeGadgets(glist);
				FreeVisualInfo(vi);
            }
        UnlockPubScreen(NULL, mysc);
        }
    CloseFont(font);
    }
	RemIntServer(INTB_VERTB, levelsint);
	FreeMem(cfg_mem,FLASH_CONFIG_SIZE);
	FreeMem(mixdat,sizeof(struct MixData));
	FreeMem(intdata,sizeof(struct IntData));
	FreeMem(levelsint, sizeof(struct Interrupt));		
	FreeSignal(signr);
}


/*
** Open all libraries and run.  Clean up when finished or on error..
*/
void main(void)
{
	SetTaskPri(FindTask(NULL),30);	
	if (NULL == (IntuitionBase = OpenLibrary("intuition.library", 37)))
		errorMessage( "Requires V37 intuition.library");
	else
		{
		if (NULL == (ExpansionBase = OpenLibrary("expansion.library", 37)))
			errorMessage( "Requires V37 expansion.library");
		else	
			{
			if (NULL == (GfxBase = OpenLibrary("graphics.library", 37)))
				errorMessage( "Requires V37 graphics.library");
			else
				{
				if (NULL == (GadToolsBase = OpenLibrary("gadtools.library", 37)))
					errorMessage( "Requires V37 gadtools.library");
				else			
					{
					gadtoolsWindow();

					CloseLibrary(GadToolsBase);
					}
				CloseLibrary(GfxBase);
				}
			CloseLibrary(ExpansionBase);
			}
		CloseLibrary(IntuitionBase);	
		}
}
