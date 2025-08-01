/*
======================================================================
AmiGUS FPGA Flash Utility (Kickstart 1.3)
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

#include <graphics/gfxbase.h>

#include <libraries/configvars.h>
#include <libraries/asl.h>
#include <libraries/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/asl_protos.h>
#include <clib/expansion_protos.h>
#include <clib/reqtools_protos.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>

#include <libraries/reqtools.h>
#include <proto/reqtools.h>

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

#define CMD_FLASH	1
#define CMD_INFO	2
#define	CMD_INIT	3
#define	CMD_LOAD	4
#define	CMD_SAVE	5
#define	CMD_QUIT	6

#define MYLEFTEDGE 0
#define MYTOPEDGE  0
#define MYWIDTH    320
#define MYHEIGHT   400

/* Gaget definitions */

#define BUTTON_FLASH (0)
#define BUTTON_INFO  (1)
#define BUTTON_INIT  (2)
#define BUTTON_SAVE  (3)
#define BUTTON_LOAD  (4)
#define BUTTON_QUIT  (5)

#define	AMIGUS_MAIN_PRODUCT_ID	16
#define AMIGUS_MANUFACTURER_ID	2782

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

#define	FPGA_ID_HIGH			0xe8
#define	FPGA_ID_LOW				0xec

// 10M08SC144 Device Flash Mapping (nowhere officially documented - THANKS INTEL!)
// Sector 1: UFM0 (16K)		0x0 - 0x3fff
// Sector 2: UFM1 (16K)		0x4000 - 0x7fff
// Sector 3: CFM0 (140K)	0x8000 - 0x2afff
// Sector 4: CFM1 (58K)		0x2b000 - 0x397ff

#define	FLASH_CONFIG_START		0x4000	// UFM1 Start
#define	FLASH_CONFIG_SIZE		0x4000

#define	FLASH_CORE_START		0x8000	// CFM1 Start
#define	FLASH_CORE_SIZE			0x31800	// Single compressed image: CFM1 & CFM0 (99 pages * 2K)

struct Library      *IntuitionBase;
struct Library      *ExpansionBase;
struct Library      *GfxBase;
struct Library      *GadToolsBase;
struct Library      *AslBase;

struct ReqToolsBase *ReqToolsBase;

struct rtFileRequester *filereq;

struct NewWindow mainWindow = {
	0,0,392,168,				
	2,1,					
	REFRESHWINDOW | GADGETUP | GADGETDOWN | CLOSEWINDOW,	
	ACTIVATE | WINDOWDRAG | WINDOWDEPTH | SMART_REFRESH | WINDOWCLOSE,
	0,			
	0,					
	"AmiGUS Flash Tool",
	0,
	0,
	0,0,0,0,
	WBENCHSCREEN
};

short myBorderData[] =
{
  0,0, 100,0, 100,44, 0,44, 0,0,
};

short myBorderData2[] =
{
  0,0, 100,0, 100,60, 0,60, 0,0,
};

short myBorderData3[] =
{
  0,0, 256,0, 256,112, 0,112, 0,0,
};

short myBorderData4[] =
{
  0,0, 256,0, 256,14, 0,14, 0,0,
};

struct TagItem frtags_flash[] =
{
    ASL_Hail,       (ULONG)"Select FPGA flash file",
    ASL_Height,     MYHEIGHT,
    ASL_Width,      MYWIDTH,
    ASL_LeftEdge,   MYLEFTEDGE,
    ASL_TopEdge,    MYTOPEDGE,
    ASL_OKText,     (ULONG)"OK",
    ASL_CancelText, (ULONG)"Cancel",
    ASL_File,       (ULONG)"AmiGUS_FPGA.upd",
    ASL_Dir,        (ULONG)"PROGDIR:",
    TAG_DONE
};

struct TagItem frtags_load[] =
{
    ASL_Hail,       (ULONG)"Load FPGA config file",
    ASL_Height,     MYHEIGHT,
    ASL_Width,      MYWIDTH,
    ASL_LeftEdge,   MYLEFTEDGE,
    ASL_TopEdge,    MYTOPEDGE,
    ASL_OKText,     (ULONG)"OK",
    ASL_CancelText, (ULONG)"Cancel",
    ASL_File,       (ULONG)"AmiGUS_config.acf",
    ASL_Dir,        (ULONG)"PROGDIR:",
    TAG_DONE
};

struct TagItem frtags_save[] =
{
    ASL_Hail,       (ULONG)"Save FPGA config file",
    ASL_Height,     MYHEIGHT,
    ASL_Width,      MYWIDTH,
    ASL_LeftEdge,   MYLEFTEDGE,
    ASL_TopEdge,    MYTOPEDGE,
    ASL_OKText,     (ULONG)"OK",
    ASL_CancelText, (ULONG)"Cancel",
    ASL_File,       (ULONG)"AmiGUS_config.acf",
    ASL_Dir,        (ULONG)"PROGDIR:",
    TAG_DONE
};

struct Window *myWin;
struct Screen myScreen;
struct RastPort *rp;
struct TextFont *font;
struct TextAttr mainFont = { "topaz.font", 8, 0, 0, };

struct button {
	struct Gadget gad;
	int on;
	int x, y, w, h;
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

struct button flashButton,infoButton,initButton,loadButton,saveButton,quitButton;

BOOL	kick13;

int 	wbscr_barheight;
int 	wbscr_width;
int 	wbscr_height;
UWORD	topborder;

/* ============================================================================================================== */

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

void EraseConfigFlash(APTR base)
{	
	ULONG status,sector;
	
	status = 0xffffffff;
	sector = 0x00200000;	// 0x1800 - 0x2fff
	
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

void ProgramConfigFlash(APTR base, APTR memory)
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

void ReadConfigFlash(APTR base, APTR memory)
{
	ULONG	memdata,flashoffset,status,cnt,length;
	
	flashoffset = (ULONG)(FLASH_CONFIG_START>>2);
	length = FLASH_CONFIG_SIZE;
	
	status = 0xffffffff;
	cnt = 0x0;
	
	do {	
		WriteReg16(base, FLASH_DATA_ADDR, flashoffset);
		WriteReg16(base, FLASH_DATA_READ_STROBE, 0x0);
		
		memdata = ReadReg32(base, FLASH_DATA_READ_PORT);
		*((ULONG *)((ULONG)memory+cnt)) = memdata;
		
		flashoffset++;
		cnt+=4;		
	}
	while (cnt < length);		
}


void EraseCoreFlash(APTR base)
{	
	ULONG status,sector;
	
	status = 0xffffffff;
	sector = 0x00100000;
	
	WriteReg16(base, FLASH_CTRL_ADDR, 0x1);
	WriteReg32(base, FLASH_CTRL_WRITE_DATA, 0x0);				// Clear protection bits
	WriteReg16(base, FLASH_CTRL_WRITE_STROBE, 0x0);
		
	do {
		WriteReg32(base, FLASH_CTRL_WRITE_DATA, sector);	// Sector erase
		WriteReg16(base, FLASH_CTRL_WRITE_STROBE, 0x0);
		WriteReg16(base, FLASH_CTRL_ADDR, 0x0);						
		
		do {
			WriteReg16(base, FLASH_CTRL_READ_STROBE, 0x0);	
		
			status = (ULONG)(ReadReg32(base, FLASH_CTRL_READ_DATA) & (ULONG)0x3);	// Read Status register
		
		} while (status != 0x0);

		sector += 0x00100000;
		
		if (sector == 0x00200000)	// We skip 2nd sector, which contains the configuration data!
			sector += 0x00100000;
		
		WriteReg16(base, FLASH_CTRL_ADDR, 0x1);
		
	} while (sector != 0x00500000);
	printf(" ");
}

void ProgramCoreFlash(APTR base, APTR memory, UWORD topborder, struct Window *mywin)
{
	ULONG	memdata,flashoffset,status,cnt,pcnt,length,startx;
	
	flashoffset = (ULONG)(FLASH_CORE_START>>2);
	length = (ULONG)FLASH_CORE_SIZE;
	status = 0xffffffff;
	cnt = 0x0;
	pcnt = 0x0;
	startx = 0;
	
	SetAPen(mywin->RPort, 3);
	
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
		
		pcnt++;
		if (pcnt==200)
		{
			pcnt = 0;
			RectFill(mywin->RPort, 120+startx, topborder+146-14, 121+startx, topborder+157-14);				
			startx++;
		}
		
		flashoffset++;
		cnt+=4;
	}
	while (cnt < length);
	printf(" ");	
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

	// ADC Reset Registers
	*((ULONG *)((ULONG)cfg_mem+0x0004)) = 0x00000020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0008)) = 0x00fe0022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x000c)) = 0x00000024;	// MAIN_SPI_WTRIG	
	
	// ADC Power-Down
	*((ULONG *)((ULONG)cfg_mem+0x0010)) = 0x00700020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0014)) = 0x00750022;	// MAIN_SPI_WDATA = regval
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

	// Enable ADC I2S Master Mode
	*((ULONG *)((ULONG)cfg_mem+0x008c)) = 0x00200020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x0090)) = 0x00900022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x0094)) = 0x00000024;	// MAIN_SPI_WTRIG

	// Set DSP1 CLOCK
	*((ULONG *)((ULONG)cfg_mem+0x0098)) = 0x00210020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x009c)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00a0)) = 0x00000024;	// MAIN_SPI_WTRIG
	
	// Set DSP2 CLOCK
	*((ULONG *)((ULONG)cfg_mem+0x00a4)) = 0x00220020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00a8)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00ac)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// Set ADC CLOCK
	*((ULONG *)((ULONG)cfg_mem+0x00b0)) = 0x00230020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00b4)) = 0x00070022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00b8)) = 0x00000024;	// MAIN_SPI_WTRIG

	// Set BCLK = CLK/4 (192kHz sampling rate)
	*((ULONG *)((ULONG)cfg_mem+0x00bc)) = 0x00260020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00c0)) = 0x00030022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00c4)) = 0x00000024;	// MAIN_SPI_WTRIG
	
	// Disable PLL
	*((ULONG *)((ULONG)cfg_mem+0x00c8)) = 0x00280020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00cc)) = 0x00000022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00d0)) = 0x00000024;	// MAIN_SPI_WTRIG	

	// ADC Power-Up
	*((ULONG *)((ULONG)cfg_mem+0x00d4)) = 0x00700020;	// MAIN_SPI_ADDRESS = regnum
	*((ULONG *)((ULONG)cfg_mem+0x00d8)) = 0x00700022;	// MAIN_SPI_WDATA = regval
	*((ULONG *)((ULONG)cfg_mem+0x00dc)) = 0x00000024;	// MAIN_SPI_WTRIG	

/* End of Stream */
	*((ULONG *)((ULONG)cfg_mem+0x00e0)) = 0xffffffff;
}


void DrawLine(struct RastPort *rp, int x1, int y1, int x2, int y2)
{
	Move(rp, x1, y1);
	Draw(rp, x2, y2);
}

void wPrintF(int line, char * text, BOOL erase,UWORD topborder,struct TextFont *font,struct Window *mywin)
{

	struct IntuiText  myIText;
	struct TextAttr   myTextAttr;

	long myTextPen = 2;
	long myBackgroundPen = 3;	
	
	if (kick13 == TRUE)
	{
		myTextPen = 1;
		myBackgroundPen = 0;			
	}
	else
	{
		myTextPen = 2;
		myBackgroundPen = 3;			
	}
	
	if (erase == TRUE)
	{
		SetAPen(mywin->RPort, myBackgroundPen);
		RectFill(mywin->RPort, 120, topborder+26-14, 120+253, topborder+135-14);
	}
	
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
    myIText.NextText    = NULL;
	  
	myIText.IText       = text;
	PrintIText(mywin->RPort,&myIText,124,topborder+30-14+(line<<3));
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

	AddGadget(myWin, &b->gad, 0);
}

void drawBorders(struct RastPort *rp,UWORD topborder,struct TextFont *font)
{
	struct Border	shineBorder;
	struct Border	shadowBorder;
  
	struct IntuiText	myIText;
	struct TextAttr		myTextAttr;

	long myShadowPen;
	long myShinePen;

	long myTextPen = 1;
	long myBackgroundPen = 0;
	
	int startx,starty;
	
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

	startx	= 12;
	starty	= 10;

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
    shadowBorder.Count      = shineBorder.Count     = 5;
    shadowBorder.XY         = shineBorder.XY        = myBorderData;
	  
	DrawBorder(rp, &shadowBorder, startx, topborder+starty);

	starty	+= 52;
    shadowBorder.XY	= shineBorder.XY	= myBorderData2;	
	DrawBorder(rp, &shadowBorder, startx, topborder+starty);	

	starty	= 10;
	startx	+= 106;
    shadowBorder.XY	= shineBorder.XY	= myBorderData3;	
	DrawBorder(rp, &shadowBorder, startx, topborder+starty);

	starty	+= 120;
	startx	+= 0;
    shadowBorder.XY         = shineBorder.XY        = myBorderData4;	
	DrawBorder(rp, &shadowBorder, startx, topborder+starty);		

	myTextAttr.ta_Name  = font->tf_Message.mn_Node.ln_Name;
    myTextAttr.ta_YSize = font->tf_YSize;
    myTextAttr.ta_Style = font->tf_Style;
    myTextAttr.ta_Flags	= font->tf_Flags;
	  
	myIText.FrontPen	= myTextPen;
	myIText.BackPen		= myBackgroundPen;
    myIText.DrawMode	= JAM2;
    myIText.LeftEdge	= 0;
    myIText.TopEdge		= 0;
    myIText.ITextFont	= &myTextAttr;
    myIText.NextText	= NULL;
	  
	startx	= 28;
	starty	= 8;
	myIText.IText	= "FPGA Core";
	PrintIText(rp,&myIText,startx,topborder+ starty);
	  
	startx	-= 8;
	starty	+= 52;
	myIText.IText	= "FPGA Config";
	PrintIText(rp,&myIText,startx,topborder+ starty);			  
}

void createAllGadgets(void)
{
	int startx, starty;
	
	startx = 32;
	starty = topborder+20;
	
	createButton(&flashButton, BUTTON_FLASH,"Flash", startx, starty, 64);		
	AddGadget(myWin, &flashButton.gad, 0);	
	drawButton(&flashButton);
	
	starty+= 16;
	createButton(&infoButton, BUTTON_INFO,"Info", startx, starty, 64);		
	AddGadget(myWin, &infoButton.gad, 0);	
	drawButton(&infoButton);	
	
	starty+= 36;
	createButton(&initButton, BUTTON_INIT,"Init", startx, starty, 64);		
	AddGadget(myWin, &initButton.gad, 0);	
	drawButton(&initButton);
	
	starty+= 16;
	createButton(&loadButton, BUTTON_LOAD,"Load", startx, starty, 64);		
	AddGadget(myWin, &loadButton.gad, 0);	
	drawButton(&loadButton);
	
	starty+= 16;
	createButton(&saveButton, BUTTON_SAVE,"Save", startx, starty, 64);		
	AddGadget(myWin, &saveButton.gad, 0);	
	drawButton(&saveButton);	

	startx-= 8;
	starty+= 28;
	createButton(&quitButton, BUTTON_QUIT,"Quit", startx, starty, 64);		
	AddGadget(myWin, &quitButton.gad, 0);	
	drawButton(&quitButton);
}

UWORD handleGadgetEvent(struct Gadget *gad)
{
	switch (gad->GadgetID)
		{
		case BUTTON_FLASH:
			return CMD_FLASH;
			break;
		case BUTTON_INFO:
			return CMD_INFO;
			break;
		case BUTTON_INIT:
			return CMD_INIT;
			break;
		case BUTTON_SAVE:
			return CMD_SAVE;
			break;
		case BUTTON_LOAD:
			return CMD_LOAD;
			break;		
		case BUTTON_QUIT:
			return CMD_QUIT;
			break;
		default:
			return 0;
			break;
		}
}

void processWindowEvents(struct Window *mywin)
{
	ULONG	mainMsg;
	BOOL	terminated = FALSE;
	struct 	IntuiMessage *winMsg;
	UWORD command;

	UBYTE	board_product_id;
	UWORD	board_manufacturer_id;
	ULONG	board_serial_id;
	APTR	board_base;
	
	char	filename[1024];
	char	fullpath[1024];
	char	txtbuf[256];
	
	struct 	ConfigDev *myCD;
	struct 	FileInfoBlock* fib;
	struct 	FileRequester *fr;
	BPTR	lock;
	

	
#ifndef DEBUG
	BOOL	board_found = FALSE;
#else
	BOOL	board_found = TRUE;
#endif	

	UWORD	fpga_date_minute;
	UWORD	fpga_date_hour;
	UWORD	fpga_date_day;
	UWORD	fpga_date_month;
	UWORD	fpga_date_year;
	
	ULONG	fpga_id_high;
	ULONG	fpga_id_low;
	
	BPTR 	filehandle;
    long 	filesize;

	APTR	cfg_mem;
	APTR	core_mem;
	
	char *	pPosition;

	wPrintF(0,"AmiGUS Flash Tool V0.5", TRUE,topborder,font,mywin);
	wPrintF(1,"(C)2025 by Oliver Achten", FALSE,topborder,font,mywin);
	
	/* Find AmiGus Card */
	
	myCD = NULL;
	while(myCD = FindConfigDev(myCD,-1L,-1L)) /* search for all ConfigDevs */	
	{
		board_manufacturer_id = myCD->cd_Rom.er_Manufacturer;
		board_product_id = myCD->cd_Rom.er_Product;
		board_serial_id = myCD->cd_Rom.er_SerialNumber;
		board_base = myCD->cd_BoardAddr;
		if (board_manufacturer_id == AMIGUS_MANUFACTURER_ID && board_product_id == AMIGUS_MAIN_PRODUCT_ID)
		{
			board_found = TRUE;

			fpga_date_minute = (UWORD)(board_serial_id & (ULONG)0x3f);
			fpga_date_hour = (UWORD)((board_serial_id & (ULONG)0x7c0)>>6);
			fpga_date_day = (UWORD)((board_serial_id & (ULONG)0xf800)>>11);
			fpga_date_month = (UWORD)((board_serial_id & (ULONG)0xf0000)>>16);
			fpga_date_year = (UWORD)((board_serial_id & (ULONG)0xfff00000)>>20);
			
			fpga_id_high = ReadReg32(board_base,FPGA_ID_HIGH);
			fpga_id_low = ReadReg32(board_base,FPGA_ID_LOW);			
			break;
		}
	}
	
	if (board_found == FALSE)
	{
		printf("ERROR: AmiGUS not found!\n");
		return;
	}
	
	if (cfg_mem = AllocMem(FLASH_CONFIG_SIZE,MEMF_ANY))
    {
		if (core_mem = AllocMem(FLASH_CORE_SIZE,MEMF_ANY))
		{
		}
		else
		{
			FreeMem(cfg_mem,FLASH_CONFIG_SIZE);
			return;
		}
	}
	else 
	{
		return;
	}		

	while (!terminated)
	{
		command = 0;
		
		mainMsg = Wait(1L << mywin->UserPort->mp_SigBit);
		if ((1L << mywin->UserPort->mp_SigBit) & mainMsg)
		{
			winMsg = (struct IntuiMessage *)GetMsg(mywin->UserPort);
			switch (winMsg->Class)
			{
				case IDCMP_GADGETDOWN:
				case IDCMP_MOUSEMOVE:
				case IDCMP_GADGETUP:
					command = handleGadgetEvent((struct Gadget *)winMsg->IAddress);
					break;				
				case CLOSEWINDOW:
					terminated = TRUE;
					break;
				case REFRESHWINDOW:
					BeginRefresh(mywin);
					EndRefresh(mywin, TRUE);
					drawBorders(mywin->RPort,topborder,font);
					break;
			}
		}
		switch (command)
		{
			case CMD_FLASH:
				if (kick13 == FALSE)
				{
					if (fr = (struct FileRequester *)
						AllocAslRequest(ASL_FileRequest, frtags_flash))
					{		
				
						if (AslRequest(fr, 0L))
						{
							sprintf(filename,"%s", fr->rf_Dir);
							AddPart(filename,fr->rf_File,sizeof(filename));
						}
						else
						{
							printf("No file selected!\n");
							break;
						}
					}
					FreeAslRequest(fr);			

					filesize = 0;				
					fib = AllocDosObject(DOS_FIB, NULL);
					if (!fib)
					{
						break;
					}	

					if (filehandle = Open(filename,MODE_OLDFILE))
					{
						if (!ExamineFH(filehandle,fib))
						{
							FreeDosObject(DOS_FIB,fib);			
							Close(filehandle);
							wPrintF(0,"ERROR: could not open file", TRUE,topborder,font,mywin);
							break;
						}
						filesize = fib->fib_Size;
						FreeDosObject(DOS_FIB,fib);
						
						if (filesize == FLASH_CORE_SIZE)
						{				
							if (Read(filehandle, core_mem, filesize)==-1)	// Preload all memory buffers
							{
								FreeDosObject(DOS_FIB,fib);			
								Close(filehandle);
								wPrintF(0,"ERROR: could not read file", TRUE,topborder,font,mywin);
								break;
							}
							else
							{
								SetAPen(mywin->RPort, 0);
								RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
								
								wPrintF(0,"File loaded successfully!", TRUE,topborder,font,mywin);
								wPrintF(1,"Flashing FPGA core...", FALSE,topborder,font,mywin);	
								
								EraseCoreFlash(board_base);
								ProgramCoreFlash(board_base,core_mem,topborder,mywin);
								
								wPrintF(1,"Flashing FPGA core... done!", FALSE,topborder,font,mywin);
								wPrintF(3,"Power-cycle your machine", FALSE,topborder,font,mywin);	
							}
						}
						else
						{
							wPrintF(0,"ERROR: wrong file type", TRUE,topborder,font,mywin);
							break;
						}
					}
					else
					{
						wPrintF(0,"ERROR: could not open file", TRUE,topborder,font,mywin);
					}
				}
				else
				{
					if (filereq = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ, NULL)) 
					{
						filename[0] = 0;
						if (rtFileRequest(filereq, &filename[0], "Select FPGA flash file",
								TAG_END))
						{								
							pPosition = strchr(filereq->Dir,':');
							if (pPosition == NULL)
								if (filereq->Dir[0]==0)
									sprintf(fullpath,"%s%s",filereq->Dir,filename);
								else
									sprintf(fullpath,"%s\\%s",filereq->Dir,filename);
							else
								if (pPosition[1]==0)
									sprintf(fullpath,"%s%s",filereq->Dir,filename);
								else
									sprintf(fullpath,"%s\\%s",filereq->Dir,filename);
	
						}
						else
						{
							rtFreeRequest((APTR)filereq);
							printf("No file selected!\n");
							break;
						}
						rtFreeRequest((APTR)filereq);
					}
					else
					{
						break;
					}	
					printf("Path: %s",fullpath);
					printf("\n");
					
					filesize = 0;
					
					if (lock = Lock(fullpath, ACCESS_READ))
					{
						if (((fib = (struct FileInfoBlock*)AllocMem((ULONG)sizeof(struct FileInfoBlock),(ULONG)MEMF_ANY))))
						{
							if (Examine(lock, fib))
							{
								filesize = fib->fib_Size;
								printf("size:%d",filesize);
								FreeMem(fib,sizeof(struct FileInfoBlock));
								
							}
							else
							{			
								printf("ERROR: access file/n");
							}

						}
						UnLock(lock);
					}
					else
					{
						printf("ERROR: access file/n");
					}					

					if (filesize == FLASH_CORE_SIZE)
					{
						if (filehandle = Open(fullpath,MODE_OLDFILE))
						{
							if (Read(filehandle, core_mem, filesize)==-1)	// Preload all memory buffers
							{
								Close(filehandle);
								wPrintF(0,"ERROR: could not read file", TRUE,topborder,font,mywin);
								break;
							}
							else
							{
								SetAPen(mywin->RPort, 0);
								RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
								
								wPrintF(0,"File loaded successfully!", TRUE,topborder,font,mywin);
								wPrintF(1,"Flashing FPGA core...", FALSE,topborder,font,mywin);	
									
								EraseCoreFlash(board_base);
								ProgramCoreFlash(board_base,core_mem,topborder,mywin);
									
								wPrintF(1,"Flashing FPGA core... done!", FALSE,topborder,font,mywin);
								wPrintF(3,"Power-cycle your machine", FALSE,topborder,font,mywin);
								Close(filehandle);
							}
						}
						else
						{
							wPrintF(0,"ERROR: could not open file", TRUE,topborder,font,mywin);
						}
					}
					else
					{
						wPrintF(0,"ERROR: wrong file type", TRUE,topborder,font,mywin);
					}	
				}				
				break;
			case CMD_INFO:
				wPrintF(0,"FPGA core date:", TRUE,topborder,font,mywin);
				sprintf(txtbuf,"  %4d-%02d-%02d, %02d:%02d",fpga_date_year,fpga_date_month,fpga_date_day,fpga_date_hour,fpga_date_minute);
				wPrintF(1,txtbuf, FALSE,topborder,font,mywin);
				
				wPrintF(3,"FPGA ID:", FALSE,topborder,font,mywin);
				sprintf(txtbuf,"  0x%lx%lx",fpga_id_high,fpga_id_low);
				wPrintF(4,txtbuf, FALSE,topborder,font,mywin);
				
				wPrintF(6,"FPGA config status:", FALSE,topborder,font,mywin);
				if ((ReadReg16(board_base,FLASH_CONFIG_STATUS)&0x8000) == 0x8000)
				{
					wPrintF(7,"   Card config is NOT OKAY!", FALSE,topborder,font,mywin);
					wPrintF(8,"   >> Recommend to INIT config", FALSE,topborder,font,mywin);
				}
				else
					wPrintF(7,"   Card config is OKAY!", FALSE,topborder,font,mywin);	
				break;
			case CMD_INIT:
				wPrintF(0,"Reset FPGA config data... done!", TRUE,topborder,font,mywin);
				SetAPen(mywin->RPort, 0);
				RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
				initCfgMem(cfg_mem);
				EraseConfigFlash(board_base);
				ProgramConfigFlash(board_base,cfg_mem);
				SetAPen(mywin->RPort, 3);
				RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);
				break;	
			case CMD_LOAD:
				if (kick13 == FALSE)
				{
					if (fr = (struct FileRequester *)
						AllocAslRequest(ASL_FileRequest, frtags_load))
					{		
				
						if (AslRequest(fr, 0L))
						{
							sprintf(filename,"%s", fr->rf_Dir);
							AddPart(filename,fr->rf_File,sizeof(filename));
						}
						else
						{
							break;
						}
					}
					FreeAslRequest(fr);			

					filesize = 0;				
					fib = AllocDosObject(DOS_FIB, NULL);
					if (!fib)
					{
						break;
					}	

					if (filehandle = Open(filename,MODE_OLDFILE))
					{
						if (!ExamineFH(filehandle,fib))
						{
							FreeDosObject(DOS_FIB,fib);			
							Close(filehandle);
							wPrintF(0,"ERROR: could not open file", TRUE,topborder,font,mywin);
							break;
						}
						filesize = fib->fib_Size;
						FreeDosObject(DOS_FIB,fib);
						
						if (filesize == FLASH_CONFIG_SIZE)
						{				
							if (Read(filehandle, cfg_mem, filesize)==-1)	// Preload all memory buffers
							{
								FreeDosObject(DOS_FIB,fib);			
								Close(filehandle);
								wPrintF(0,"ERROR: could not read file", TRUE,topborder,font,mywin);
								break;
							}
							else
							{
								SetAPen(mywin->RPort, 0);
								RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
								
								wPrintF(0,"File loaded successfully!", TRUE,topborder,font,mywin);
								wPrintF(1,"Flashing FPGA config...", FALSE,topborder,font,mywin);							
								
								EraseConfigFlash(board_base);
								ProgramConfigFlash(board_base,cfg_mem);
								
								SetAPen(mywin->RPort, 3);
								RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
								
								wPrintF(1,"Flashing FPGA config... done!", FALSE,topborder,font,mywin);
								wPrintF(3,"Please reset your machine", FALSE,topborder,font,mywin);							
							}
						}
						else
						{
							wPrintF(0,"ERROR: wrong file type", TRUE,topborder,font,mywin);
							break;
						}
					}
					else
					{
						wPrintF(0,"ERROR: could not open file", TRUE,topborder,font,mywin);
					}
				}
				else
				{
					if (filereq = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ, NULL)) 
					{
						filename[0] = 0;
						if (rtFileRequest(filereq, &filename[0], "Load FPGA config file",
								TAG_END))
						{	
							sprintf(fullpath,"%s",filereq->Dir);
							
							pPosition = strchr(filereq->Dir,':');
							if (pPosition == NULL)
								if (filereq->Dir[0]==0)
									sprintf(fullpath,"%s%s",filereq->Dir,filename);
								else
									sprintf(fullpath,"%s\\%s",filereq->Dir,filename);
							else
								if (pPosition[1]==0)
									sprintf(fullpath,"%s%s",filereq->Dir,filename);
								else
									sprintf(fullpath,"%s\\%s",filereq->Dir,filename);
						}
						else
						{
							rtFreeRequest((APTR)filereq);
							printf("No file selected!\n");
							break;
						}
						rtFreeRequest((APTR)filereq);
					}
					else
					{
						break;
					}	
					printf("Path: %s",fullpath);
					printf("\n");
					
					filesize = 0;
					
					if (lock = Lock(fullpath, ACCESS_READ))
					{
						if (((fib = (struct FileInfoBlock*)AllocMem((ULONG)sizeof(struct FileInfoBlock),(ULONG)MEMF_ANY))))
						{
							if (Examine(lock, fib))
							{
								filesize = fib->fib_Size;
								printf("size:%d",filesize);
								FreeMem(fib,sizeof(struct FileInfoBlock));
							}
							else
							{			
								printf("ERROR: access file/n");
							}

						}
						UnLock(lock);
					}
					else
					{
						printf("ERROR: access file/n");
					}					

					if (filesize == FLASH_CONFIG_SIZE)
					{
						if (filehandle = Open(fullpath,MODE_OLDFILE))
						{
							if (Read(filehandle, core_mem, filesize)==-1)	// Preload all memory buffers
							{
								Close(filehandle);
								wPrintF(0,"ERROR: could not read file", TRUE,topborder,font,mywin);
								break;
							}
							else
							{
								SetAPen(mywin->RPort, 0);
								RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
									
								wPrintF(0,"File loaded successfully!", TRUE,topborder,font,mywin);
								wPrintF(1,"Flashing FPGA config...", FALSE,topborder,font,mywin);							
									
								EraseConfigFlash(board_base);
								ProgramConfigFlash(board_base,cfg_mem);
									
								SetAPen(mywin->RPort, 3);
								RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
									
								wPrintF(1,"Flashing FPGA config... done!", FALSE,topborder,font,mywin);
								wPrintF(3,"Please reset your machine", FALSE,topborder,font,mywin);
								Close(filehandle);
							}
						}
						else
						{
							wPrintF(0,"ERROR: could not open file", TRUE,topborder,font,mywin);
						}
					}
					else
					{
						wPrintF(0,"ERROR: wrong file type", TRUE,topborder,font,mywin);
					}						
				}
				break;
			case CMD_SAVE:
				if (kick13 == FALSE)
				{
					if (fr = (struct FileRequester *)
						AllocAslRequest(ASL_FileRequest, frtags_save))
					{		
				
						if (AslRequest(fr, 0L))
						{
							sprintf(filename,"%s", fr->rf_Dir);
							AddPart(filename,fr->rf_File,sizeof(filename));
						}
						else
						{
							wPrintF(0,"Config save aborted", TRUE,topborder,font,mywin);
							break;
						}
					}
					FreeAslRequest(fr);					
					ReadConfigFlash(board_base,cfg_mem);				
			
					filesize = FLASH_CONFIG_SIZE;				

					if (filehandle = Open(filename,MODE_NEWFILE))
					{
						if (Write(filehandle, cfg_mem, filesize)==-1)	// Preload all memory buffers
						{	
							Close(filehandle);
							wPrintF(0,"ERROR: could not write file", TRUE,topborder,font,mywin);
							break;
						}
						else
						{
							SetAPen(mywin->RPort, 0);
							RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
							
							wPrintF(0,"Config file written", TRUE,topborder,font,mywin);					
						}
						Close(filehandle);
					}
					else
					{
						wPrintF(0,"ERROR: file not written", TRUE,topborder,font,mywin);
						break;
					}
				}
				else
				{
					if (filereq = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ, NULL)) 
					{
						filename[0] = 0;
						if (rtFileRequest(filereq, &filename[0], "Save FPGA config file",
								TAG_END))
						{	
							sprintf(fullpath,"%s",filereq->Dir);
							
							pPosition = strchr(filereq->Dir,':');
							if (pPosition == NULL)
								if (filereq->Dir[0]==0)
									sprintf(fullpath,"%s%s",filereq->Dir,filename);
								else
									sprintf(fullpath,"%s\\%s",filereq->Dir,filename);
							else
								if (pPosition[1]==0)
									sprintf(fullpath,"%s%s",filereq->Dir,filename);
								else
									sprintf(fullpath,"%s\\%s",filereq->Dir,filename);
						}
						else
						{
							rtFreeRequest((APTR)filereq);
							wPrintF(0,"Config save aborted", TRUE,topborder,font,mywin);
							break;
						}
						rtFreeRequest((APTR)filereq);
					}
					else
					{
						break;
					}	
					printf("Path: %s",fullpath);
					printf("\n");	

					filesize = FLASH_CONFIG_SIZE;				

					if (filehandle = Open(fullpath,MODE_NEWFILE))
					{
						if (Write(filehandle, cfg_mem, filesize)==-1)	// Preload all memory buffers
						{	
							Close(filehandle);
							wPrintF(0,"ERROR: could not write file", TRUE,topborder,font,mywin);
							break;
						}
						else
						{
							SetAPen(mywin->RPort, 0);
							RectFill(mywin->RPort, 120, topborder+146-14, 120+253, topborder+157-14);	
							
							wPrintF(0,"Config file written", TRUE,topborder,font,mywin);					
						}
						Close(filehandle);
					}
					else
					{
						wPrintF(0,"ERROR: file not written", TRUE,topborder,font,mywin);
						break;
					}
					
				}					
				break;
			case CMD_QUIT:
				terminated = TRUE;
				break;
		}					
	}
	FreeMem(cfg_mem,FLASH_CONFIG_SIZE);
	FreeMem(core_mem,FLASH_CORE_SIZE);
}



void drawWindow(void)
{
	struct Screen   *mysc;
	void            *vi;

	
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
					topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);
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
		topborder = 14;
		if (NULL == (ReqToolsBase = (struct ReqToolsBase *)OpenLibrary("reqtools.library", 38)))
		{
			printf( "Can not open reqtools.library");
			return;
		}
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

		createAllGadgets();
		drawBorders(myWin->RPort,topborder,font);
		
		processWindowEvents(myWin);
		
		if (kick13 == FALSE)
		{
			CloseLibrary(AslBase);
		}
		else
		{
			CloseLibrary((struct Library *)ReqToolsBase);
		}
		CloseWindow(myWin);
	}
}


void main(void)
{	
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
