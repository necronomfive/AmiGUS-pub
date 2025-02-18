/*
======================================================================
AmiGUS Record Utility
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <libraries/dos.h>
#include <libraries/configvars.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/expansion.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>

#include <hardware/intbits.h>

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

#define MAIN_REC_SMPL_FMT	0x80
#define MAIN_REC_SMPL_RATE	0x82
#define MAIN_REC_VOL_LEFT	0x84
#define MAIN_REC_VOL_RIGHT	0x86
#define MAIN_REC_FIFO_RES	0x88
#define MAIN_REC_FIFO_WTMK	0x8a
#define MAIN_REC_FIFO_DATA	0x8c
#define MAIN_REC_FIFO_USE	0x90


/* AmiGUS Main Interrupt Flags */

#define INT_FLG_FIFO_EMPTY		0x1
#define INT_FLG_FIFO_FULL		0x2
#define INT_FLG_FIFO_WTMK		0x4
#define INT_FLG_SPI_FIN			0x8
#define INT_FLG_REC_FIFO_EMPTY	0x10
#define INT_FLG_REC_FIFO_FULL	0x20
#define INT_FLG_REC_FIFO_WTMK	0x40
#define INT_FLG_MASK_SET		0x8000

/* AmiGUS Main Sample Formats */

#define SMPL_FMT_MONO_8BIT			0x0
#define SMPL_FMT_STEREO_8BIT		0x1
#define SMPL_FMT_MONO_16BIT			0x2
#define SMPL_FMT_STEREO_16BIT		0x3
#define SMPL_FMT_MONO_8BIT_SWP		0x4
#define SMPL_FMT_STEREO_8BIT_SWP	0x5
#define SMPL_FMT_MONO_16BIT_SWP		0x6
#define SMPL_FMT_STEREO_16BIT_SWP	0x7
#define	SMPL_FMT_LITTLE_ENDIAN		0x8

/* AmiGUS Main Sample Rates */

#define SMPL_RATE_8000		0x0
#define SMPL_RATE_11025		0x1
#define SMPL_RATE_16000		0x2
#define SMPL_RATE_22050		0x3
#define SMPL_RATE_24000		0x4
#define SMPL_RATE_32000		0x5
#define SMPL_RATE_44100		0x6
#define SMPL_RATE_48000		0x7
#define SMPL_RATE_96000		0x8

#define REC_FMT_8BIT		0x0
#define REC_FMT_16BIT		0x1
#define REC_FMT_STEREO		0x0
#define REC_FMT_MONOL		0x2
#define REC_FMT_MONOR		0x4
#define REC_FMT_MONOMIX		0x6
#define REC_FMT_BIG_END		0x0
#define REC_FMT_LITTLE_END	0x8
#define REC_FMT_SIGNED		0x0
#define REC_FMT_UNSIGNED	0x10

#define REC_SMPL_RATE_8000	0x0
#define REC_SMPL_RATE_11025	0x1
#define REC_SMPL_RATE_16000	0x2
#define REC_SMPL_RATE_22050	0x3
#define REC_SMPL_RATE_24000	0x4
#define REC_SMPL_RATE_32000	0x5
#define REC_SMPL_RATE_44100	0x6
#define REC_SMPL_RATE_48000	0x7
#define REC_SMPL_RATE_96000	0x8

#define REC_SRC_MIXER		0x0
#define	REC_SRC_ADC			0x10
#define	REC_SRC_MP3			0x20
#define	REC_SRC_HAGEN		0x30
#define REC_SRC_AHI			0x40

/* Memory Buffers */

#define	FIFO_SIZE			2048			/* FIFO has 2048 * 16-bit words */
#define FIFO_WTMK			1024			/* FIFO watermark */

#define	BUF_SIZE			(FIFO_WTMK<<1)
#define BUF_NUM				32
#define	MEM_SIZE			BUF_SIZE*BUF_NUM

struct IntData {

	APTR 	board_base;
	APTR	buffer;
	ULONG	buf_size;
	ULONG 	buf_count;	
	ULONG	buf_segments;
	ULONG	buf_half_segments;
		
	struct 	Task *rd_Task;
	ULONG 	rd_Signal;
	
	ULONG	tmp_buffer;		// used by int routine, do not modify!
};

/* Assembler function definitions */

extern void intServer();

/* Register Access Funtions */

void WriteReg16(APTR base, ULONG offset, UWORD val)
{
	*((UWORD *)((ULONG)base+offset)) = val;	
}

void WriteReg32(APTR base, ULONG offset, ULONG val)
{
	*((ULONG *)((ULONG)base+offset)) = val;	
}

void initAmiGUS(APTR board_base)
{
	WriteReg16(board_base,MAIN_REC_SMPL_RATE,0x0);						// Disable playback		
	WriteReg16(board_base,MAIN_REC_FIFO_RES,0x0);						// Reset FIFO state
	WriteReg16(board_base,MAIN_REC_FIFO_WTMK,FIFO_WTMK);				// Set FIFO watermark
	WriteReg16(board_base,MAIN_INTC,INT_FLG_REC_FIFO_EMPTY|INT_FLG_REC_FIFO_FULL|INT_FLG_REC_FIFO_WTMK);	// Disable INTS		
	WriteReg16(board_base,MAIN_INTE,INT_FLG_REC_FIFO_EMPTY|INT_FLG_REC_FIFO_FULL|INT_FLG_REC_FIFO_WTMK);	// Disable INTS
	WriteReg16(board_base,MAIN_INTE,INT_FLG_MASK_SET|INT_FLG_REC_FIFO_WTMK);						// Enable Watermark INT	
	WriteReg16(board_base,MAIN_REC_VOL_LEFT,0xffff);
	WriteReg16(board_base,MAIN_REC_VOL_RIGHT,0xffff);	
}

void shutdownAmiGUS(APTR board_base)
{
	WriteReg16(board_base,MAIN_REC_SMPL_RATE,0x0);						// Disable playback	
	WriteReg16(board_base,MAIN_INTC,INT_FLG_REC_FIFO_EMPTY|INT_FLG_REC_FIFO_FULL|INT_FLG_REC_FIFO_WTMK);	// Disable INTS	
	WriteReg16(board_base,MAIN_INTE,INT_FLG_REC_FIFO_EMPTY|INT_FLG_REC_FIFO_FULL|INT_FLG_REC_FIFO_WTMK);	// Disable INTS
	WriteReg16(board_base,MAIN_REC_FIFO_RES,0x0);						// Reset FIFO state
}

/* Main Program */

int main(int argc,char **argv)
{
 	struct Library	*SysBase = NULL;
	struct Library	*ExpansionBase = NULL;
	struct Library	*DOSBase = NULL; 

	struct Interrupt *fifoint;
 
	struct ConfigDev *myCD;
    struct FileInfoBlock* fib;
	
	struct IntData *intdata;

	UWORD	i;
	UWORD	smpl_fmt;
	UWORD	smpl_ctrl;
		
	UBYTE	board_product_id;
	UWORD	board_manufacturer_id;
	ULONG	board_serial_id;	
	
	UWORD	fpga_date_minute;
	UWORD	fpga_date_hour;
	UWORD	fpga_date_day;
	UWORD	fpga_date_month;
	UWORD	fpga_date_year;	
	
	APTR	board_base,memory,write_ptr;
	BPTR 	filehandle;
	BYTE	signr;
		
	BOOL	board_found = FALSE;
	BOOL	terminated = FALSE;
	
	ULONG 	Temp;
	ULONG 	WaitMask;
	
	SetTaskPri(FindTask(NULL),30);
	
    printf("\n==========================\n   AmiGUS Recorder V0.2\n (C)2025 by Oliver Achten \n==========================\n\n");
	
	
	smpl_fmt = REC_FMT_16BIT|REC_FMT_LITTLE_END;		// Set format		
	smpl_ctrl = REC_SMPL_RATE_44100;					// Start recording
	
	if (argc < 2)
	{
		printf("ERROR: wrong number of parameters\n\n");
		return 0;
	}
	else if (argc > 2)
	{
		for (i = 2; i < argc; i++)
		{
			if (strcmp(argv[i],"-8bit")==0)
			{
				smpl_fmt &=0xfffe;
				smpl_fmt |= REC_FMT_8BIT;
			}
			else if (strcmp(argv[i],"-16bit")==0)
			{
				smpl_fmt &=0xfffe;
				smpl_fmt |= REC_FMT_16BIT;				
			}
			else if (strcmp(argv[i],"-mono")==0)
			{
				smpl_fmt &=0xfff9;
				smpl_fmt |= REC_FMT_MONOMIX;
			}	
			else if (strcmp(argv[i],"-monol")==0)
			{
				smpl_fmt &=0xfff9;
				smpl_fmt |= REC_FMT_MONOL;				
			}	
			else if (strcmp(argv[i],"-monor")==0)
			{
				smpl_fmt &=0xfff9;
				smpl_fmt |= REC_FMT_MONOR;				
			}				
			else if (strcmp(argv[i],"-stereo")==0)
			{
				smpl_fmt &=0xfff9;
				smpl_fmt |= REC_FMT_STEREO;					
			}
			else if (strcmp(argv[i],"-big")==0)
			{
				smpl_fmt &=0xfff7;
				smpl_fmt |= REC_FMT_BIG_END;					
			}			
			else if (strcmp(argv[i],"-little")==0)
			{
				smpl_fmt &=0xfff7;
				smpl_fmt |= REC_FMT_LITTLE_END;					
			}
			else if (strcmp(argv[i],"-signed")==0)
			{
				smpl_fmt &=0xffef;
				smpl_fmt |= REC_FMT_SIGNED;							
			}			
			else if (strcmp(argv[i],"-unsigned")==0)
			{
				smpl_fmt &=0xffef;
				smpl_fmt |= REC_FMT_UNSIGNED;				
			}				
			else if (strcmp(argv[i],"-src_mixer")==0)
			{
				smpl_ctrl &= 0xff8f;
				smpl_ctrl |= REC_SRC_MIXER;				
			}
			else if (strcmp(argv[i],"-src_adc")==0)
			{
				smpl_ctrl &= 0xff8f;
				smpl_ctrl |= REC_SRC_ADC;	
				
			}	
			else if (strcmp(argv[i],"-src_mp3")==0)
			{
				smpl_ctrl &= 0xff8f;
				smpl_ctrl |= REC_SRC_MP3;					
			}	
			else if (strcmp(argv[i],"-src_wave")==0)
			{
				smpl_ctrl &= 0xff8f;
				smpl_ctrl |= REC_SRC_HAGEN;					
			}
			else if (strcmp(argv[i],"-src_ahi")==0)
			{
				smpl_ctrl &= 0xff8f;
				smpl_ctrl |= REC_SRC_AHI;					
			}			
			else if (strcmp(argv[i],"-8000")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_8000;					
			}
			else if (strcmp(argv[i],"-11025")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_11025;				
			}
			else if (strcmp(argv[i],"-16000")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_16000;				
			}	
			else if (strcmp(argv[i],"-22050")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_22050;				
			}			
			else if (strcmp(argv[i],"-24000")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_24000;				
			}
			else if (strcmp(argv[i],"-32000")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_32000;				
			}
			else if (strcmp(argv[i],"-44100")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_44100;				
			}
			else if (strcmp(argv[i],"-48000")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_48000;				
			}
			else if (strcmp(argv[i],"-96000")==0)
			{
				smpl_ctrl &= 0xfff0;
				smpl_ctrl |= REC_SMPL_RATE_96000;				
			}
			else
			{
				printf("ERROR: unknown parameter: %s\n\n",argv[i]);
				return 0;				
			}
		}	
	}
	
	/* ================ Open libraries ================ */

	if((SysBase=OpenLibrary("exec.library",36L))==NULL)
	{
		printf("ERROR: this tool needs Kickstart 2.0 or higher\n\n");
		return 0;
	}
	
	if((ExpansionBase=OpenLibrary("expansion.library",0L))==NULL)
	{
        printf("ERROR: can't open 'expansion.library'\n");
        return 0;
	}
	
	if((DOSBase=OpenLibrary("dos.library",0L))==NULL)
    {
		CloseLibrary(DOSBase);		
        printf("ERROR: can't open 'dos.library'");
        return 0;
    }
	
	/* ================ Find AmiGUS card ================ */
	
	myCD = NULL;
    while(myCD=FindConfigDev(myCD,-1L,-1L)) /* search for all ConfigDevs */	
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
			break;
		}
	}
	
	CloseLibrary(ExpansionBase);
	
	if (board_found == TRUE)
	{
		printf("AmiGUS found at $%lx\n",board_base);
		printf("FPGA Date: %4d-%02d-%02d, %02d:%02d\n\n",fpga_date_year,fpga_date_month,fpga_date_day,fpga_date_hour,fpga_date_minute);			
	
	}
	else
	{
		printf("ERROR: no AmiGUS board found!\n");	
		return 0;
	}
	
	/* ================ Install Interrupt Server ================ */

	if ((signr = AllocSignal(-1)) == -1)          /* Allocate a signal bit for the   */
    {                                             /* interrupt handler to signal us. */
        printf("ERROR: can't allocate signal\n");
        return 0;			
	}
	
	if (intdata = AllocMem(sizeof(struct IntData), MEMF_PUBLIC|MEMF_CLEAR))
    {
		intdata->buf_size = (ULONG)((BUF_SIZE>>2)-1);			/* Size of data transferred per buffer (in DWORDs) */
		intdata->buf_count = 0;							/* Current buffer number (reset to 0) */
		intdata->buf_segments = (ULONG)BUF_NUM;				/* Number of buffer segments */
		intdata->buf_half_segments = (ULONG)(BUF_NUM>>1);		/* 1/2 nuber of buffer segments */
		
		intdata->board_base = board_base;
		intdata->rd_Task = FindTask(NULL);
		intdata->rd_Signal = 1L << signr;	
	}
	else 
	{
        printf("ERROR: can't allocate memory for interrupt node\n");
        return 0;		
	}
    
	WaitMask = SIGBREAKF_CTRL_C |
				intdata->rd_Signal;
	
    if (fifoint = AllocMem(sizeof(struct Interrupt), 
                         MEMF_PUBLIC|MEMF_CLEAR))
    {
		fifoint->is_Node.ln_Type = NT_INTERRUPT;
        fifoint->is_Node.ln_Pri = 100;
        fifoint->is_Node.ln_Name = "INT_AMIGUS_REC";
        fifoint->is_Data = (APTR)intdata;
        fifoint->is_Code = intServer;
		
		AddIntServer(INTB_PORTS, fifoint);

	}
	else 
	{
        printf("ERROR: can't allocate memory for interrupt node\n");
		FreeMem(intdata,sizeof(struct IntData));
		FreeSignal(signr);
		
        return 0;		
	}
	

	
	/* ================ Open & Stream WAV File ================ */
	    
	if (memory = AllocMem(MEM_SIZE,MEMF_ANY)){}
    else
    {
        printf("ERROR: couldn't allocate buffer memory\n");
		CloseLibrary(DOSBase);
		FreeMem(intdata,sizeof(struct IntData));
		FreeMem(fifoint, sizeof(struct Interrupt));	
		FreeSignal(signr);
		
        return 0;
    }
	
    fib = AllocDosObject(DOS_FIB, NULL);
    if (!fib)
    {
        printf("ERROR: can't open file resources\n");
		CloseLibrary(DOSBase);
		FreeMem(intdata,sizeof(struct IntData));
		FreeMem(fifoint, sizeof(struct Interrupt));			
		FreeMem(memory,MEM_SIZE);
		FreeSignal(signr);
		
        return 0;
    }		
	
	initAmiGUS(board_base);

    if (filehandle = Open(argv[1],MODE_NEWFILE))
    {
        if (!ExamineFH(filehandle,fib))
        {
            printf("ERROR: can't open file resources");

			FreeDosObject(DOS_FIB,fib);
			
			Close(filehandle);
			CloseLibrary(DOSBase);
			shutdownAmiGUS(board_base);
			RemIntServer(INTB_PORTS, fifoint);							
			FreeMem(memory,MEM_SIZE);
			FreeMem(intdata,sizeof(struct IntData));
			FreeMem(fifoint, sizeof(struct Interrupt));	
			FreeSignal(signr);
			
            return 0;
        }
		
        FreeDosObject(DOS_FIB,fib);
		
		printf("Recording WAV file '%s'....",argv[1]);
		printf("\nPress CTRL-C to abort. \n");
		printf("   \n");		
			
		intdata->buffer = memory;
		WriteReg16(board_base,MAIN_REC_SMPL_FMT,smpl_fmt);		// Set format		
		WriteReg16(board_base,MAIN_REC_SMPL_RATE,smpl_ctrl|0x8000);			// Start recording

		while (terminated == FALSE)
		{	
			Temp = Wait(WaitMask);
					
					// Interrupt wait
					// Int server sends signal when one-half of buffer as been transferred
					
			if (SIGBREAKF_CTRL_C & Temp)								// User pressed CTRL-C -> Abort program
			{
				terminated = TRUE;
			}
			else if (intdata->rd_Signal & Temp)	
			{	
				if (intdata->buf_count >= (BUF_NUM>>1))					// Which half of the buffer is now used by int server?
				{
					write_ptr = memory;									// FIFO transfers second half, write first half
				}
				else
				{
					write_ptr = (APTR)((long)memory + (MEM_SIZE>>1));	// FIFO transfers first half, write second half
				}	
		
		
				if (Write(filehandle, write_ptr, MEM_SIZE>>1)==-1)
				{
					printf("ERROR: writing file\n");
							
					Close(filehandle);
					CloseLibrary(DOSBase);
					shutdownAmiGUS(board_base);
					RemIntServer(INTB_PORTS, fifoint);							
					FreeMem(memory,MEM_SIZE);
					FreeMem(intdata,sizeof(struct IntData));
					FreeMem(fifoint, sizeof(struct Interrupt));	
					FreeSignal(signr);
							
					return 0;
				}
				
			}	
		}		
		
		printf("stopped!\n");
		Close(filehandle);
	}
	else
	{
		printf("ERROR: couldn't open file '%s'\n",argv[1]);
	}
	
	CloseLibrary(DOSBase);
	CloseLibrary(SysBase);							
	shutdownAmiGUS(board_base);
	RemIntServer(INTB_PORTS, fifoint);							
	FreeMem(memory,MEM_SIZE);
	FreeMem(intdata,sizeof(struct IntData));
	FreeMem(fifoint, sizeof(struct Interrupt));		
	FreeSignal(signr);
	
	return 0;  
   }