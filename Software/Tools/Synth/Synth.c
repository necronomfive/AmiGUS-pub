/*
======================================================================
AmiGUS Synth Utility
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
#include <exec/io.h>
#include <devices/serial.h>

#include <libraries/dos.h>
#include <libraries/configvars.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/expansion.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>
#include <clib/alib_protos.h>

#include <hardware/intbits.h>

#define	FALSE	0
#define TRUE	1
#define BOOL	int

/* AmiGUS Zorro IDs */

#define	AMIGUS_HAGEN_PRODUCT_ID	17
#define AMIGUS_MANUFACTURER_ID	2782

/* AmiGUS Main Register Definitions */
#define HAGEN_INTC0			0x00
#define HAGEN_INTC1			0x02
#define HAGEN_INTC2			0x04
#define HAGEN_INTC3			0x06
#define HAGEN_INTE0			0x08
#define HAGEN_INTE1			0x0a
#define HAGEN_INTE2			0x0c
#define HAGEN_INTE3			0x0e

#define HAGEN_WDATAH		0x10
#define HAGEN_WDATAL		0x12
#define HAGEN_WADDRH		0x14
#define HAGEN_WADDRL		0x16
#define HAGEN_WRESET		0x18

#define HAGEN_VOICE_BNK		0x1e
#define HAGEN_VOICE_CTRL	0x20
#define HAGEN_VOICE_PSTRTH	0x22
#define HAGEN_VOICE_PSTRTL	0x24
#define HAGEN_VOICE_PLOOPH	0x26
#define HAGEN_VOICE_PLOOPL	0x28
#define HAGEN_VOICE_PENDH	0x2a
#define HAGEN_VOICE_PENDL	0x2c
#define HAGEN_VOICE_RATEH	0x2e
#define HAGEN_VOICE_RATEL	0x30
#define HAGEN_VOICE_VOLUMEL	0x32
#define HAGEN_VOICE_VOLUMER 0x34

#define	HAGEN_VOICE_ENV_ATTACK	0x36
#define	HAGEN_VOICE_ENV_DECAY	0x38
#define	HAGEN_VOICE_ENV_SUSTAIN	0x3a
#define	HAGEN_VOICE_ENV_RELEASE	0x3c
#define	HAGEN_VOICE_ENV_RESET	0x3e

#define HAGEN_GLOBAL_VOLUMEL	0x40
#define HAGEN_GLOBAL_VOLUMER	0x42

#define	MIDI_NOTE_OFF			0x80
#define MIDI_NOTE_ON			0x90

/* Static arrays */

static ULONG midiFreq[128] = {	
								/* Samplerates based on C-4 = 22.050Hz*/
								 0x3ACCCC,0x3E4BE2,0x420032,0x45ECE5
								,0x4A1556,0x4E7D13,0x5327DF,0x5819B7
								,0x5D56D4,0x62E3B0,0x68C50A,0x6EFFE7
								,0x759999,0x7C97C5,0x840064,0x8BD9CB
								,0x942AAD,0x9CFA27,0xA64FBF,0xB0336E
								,0xBAADA9,0xC5C761,0xD18A14,0xDDFFCE
								,0xEB3333,0xF92F8B,0x10800C9,0x117B396
								,0x128555B,0x139F44F,0x14C9F7E,0x16066DD
								,0x1755B52,0x18B8EC3,0x1A31429,0x1BBFF9C
								,0x1D66666,0x1F25F16,0x2100192,0x22F672C
								,0x250AAB7,0x273E89E,0x2993EFD,0x2C0CDBB
								,0x2EAB6A4,0x3171D87,0x3462852,0x377FF38
								,0x3ACCCCC,0x3E4BE2D,0x4200325,0x45ECE59
								,0x4A1556E,0x4E7D13C,0x5327DFB,0x5819B77
								,0x5D56D49,0x62E3B0E,0x68C50A5,0x6EFFE70
								,0x7599999,0x7C97C5A,0x840064B,0x8BD9CB3
								,0x942AADD,0x9CFA279,0xA64FBF7,0xB0336EF
								,0xBAADA93,0xC5C761D,0xD18A14B,0xDDFFCE1
								,0xEB33333,0xF92F8B5,0x10800C97,0x117B3966
								,0x128555BB,0x139F44F3,0x14C9F7EE,0x16066DDF
								,0x1755B527,0x18B8EC3B,0x1A314296,0x1BBFF9C2
								,0x1D666666,0x1F25F16A,0x2100192E,0x22F672CC
								,0x250AAB77,0x273E89E6,0x2993EFDC,0x2C0CDBBF
								,0x2EAB6A4F,0x3171D876,0x3462852D,0x377FF385
								,0x3ACCCCCC,0x3E4BE2D5,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
								,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF,0x3FFFFFFF
							};
							
UWORD logVel[128] = {
						0x0
						,0x4
						,0x10
						,0x24
						,0x41
						,0x65
						,0x92
						,0xc7
						,0x104
						,0x149
						,0x196
						,0x1eb
						,0x249
						,0x2ae
						,0x31c
						,0x392
						,0x410
						,0x496
						,0x524
						,0x5ba
						,0x659
						,0x6ff
						,0x7ae
						,0x865
						,0x924
						,0x9eb
						,0xaba
						,0xb92
						,0xc71
						,0xd59
						,0xe48
						,0xf40
						,0x1040
						,0x1148
						,0x1259
						,0x1371
						,0x1491
						,0x15ba
						,0x16eb
						,0x1824
						,0x1965
						,0x1aae
						,0x1bff
						,0x1d58
						,0x1eba
						,0x2023
						,0x2195
						,0x230f
						,0x2491
						,0x261b
						,0x27ad
						,0x2948
						,0x2aea
						,0x2c95
						,0x2e48
						,0x3003
						,0x31c6
						,0x3391
						,0x3564
						,0x373f
						,0x3923
						,0x3b0f
						,0x3d02
						,0x3efe
						,0x4102
						,0x430e
						,0x4523
						,0x473f
						,0x4964
						,0x4b90
						,0x4dc5
						,0x5002
						,0x5247
						,0x5494
						,0x56e9
						,0x5947
						,0x5bac
						,0x5e1a
						,0x6090
						,0x630e
						,0x6594
						,0x6822
						,0x6ab8
						,0x6d57
						,0x6ffd
						,0x72ac
						,0x7563
						,0x7822
						,0x7ae9
						,0x7db8
						,0x808f
						,0x836f
						,0x8656
						,0x8946
						,0x8c3e
						,0x8f3e
						,0x9246
						,0x9556
						,0x986e
						,0x9b8f
						,0x9eb7
						,0xa1e8
						,0xa521
						,0xa862
						,0xabab
						,0xaefc
						,0xb255
						,0xb5b7
						,0xb920
						,0xbc92
						,0xc00c
						,0xc38e
						,0xc718
						,0xcaaa
						,0xce45
						,0xd1e7
						,0xd592
						,0xd944
						,0xdcff
						,0xe0c2
						,0xe48d
						,0xe860
						,0xec3c
						,0xf01f
						,0xf40b
						,0xf7ff
						,0xfbfb
						,0xffff
};

/* Assembler function definitions */

extern __regargs __asm initSample(register __a0 APTR *addr, register __a6 APTR *baseadd, register __d3 ULONG size);

/* Register Access Funtions */

void WriteReg16(APTR base, ULONG offset, UWORD val)
{
	*((UWORD *)((ULONG)base+offset)) = val;	
}

void WriteReg32(APTR base, ULONG offset, ULONG val)
{
	*((ULONG *)((ULONG)base+offset)) = val;	
}

UWORD ReadReg16(APTR base, ULONG offset)
{
	return *((UWORD *)((ULONG)base+offset));
}

void hagenPlayNote(APTR base, UWORD chNr, UWORD midNote, UWORD midVel)
{
	ULONG freq;
	UWORD fVel;
	WriteReg16(base,HAGEN_VOICE_BNK,chNr);	// Select voice
	WriteReg16(base,HAGEN_VOICE_CTRL,0x0);	// Stop note	
	WriteReg16(base,HAGEN_VOICE_ENV_RESET,0x0);	// Reset evelope
	
	freq = midiFreq[midNote];				// Convert midi note to frequency value
	
	WriteReg32(base,HAGEN_VOICE_RATEH,freq);	// Program frequency
	
	fVel = logVel[midVel];
	
	if ((chNr & 1) == 0)
	{
		WriteReg16(base,HAGEN_VOICE_VOLUMEL,(fVel>>2));
		WriteReg16(base,HAGEN_VOICE_VOLUMER,(fVel));
	}
	else
	{
		WriteReg16(base,HAGEN_VOICE_VOLUMEL,(fVel));
		WriteReg16(base,HAGEN_VOICE_VOLUMER,(fVel>>2));		
	}
	
	WriteReg32(base,HAGEN_VOICE_PSTRTH,0x0);	// Reset pointer
	WriteReg16(base,HAGEN_VOICE_CTRL,0xc02f);		// Trigger voice
}

void hagenReleaseNote(APTR base, UWORD chNr)
{
	ULONG freq;
	UWORD fVel;
	WriteReg16(base,HAGEN_VOICE_BNK,chNr);	// Select voice
	WriteReg16(base,HAGEN_VOICE_CTRL,0x802f);	// Release voice key
}


/* Main Program */

int main(int argc,char **argv)
{
 	struct Library	*SysBase = NULL;
	struct Library	*ExpansionBase = NULL;
	struct Library	*DOSBase = NULL; 

	struct ConfigDev *myCD;
    struct FileInfoBlock* fib;

		
	UBYTE	board_product_id;
	UWORD	board_manufacturer_id;
	ULONG	board_serial_id;
	
	UWORD	fpga_date_minute;
	UWORD	fpga_date_hour;
	UWORD	fpga_date_day;
	UWORD	fpga_date_month;
	UWORD	fpga_date_year;	
	
	UWORD	control_word;
	APTR	board_base,memory;
	BPTR 	filehandle;
	BOOL	terminated;
	
    long 	filesize,read_data;
	int 	i,val;
	
	struct MsgPort *SerialMP;       /* pointer to our message port */
	struct IOExtSer *SerialIO;      /* pointer to our IORequest */
	
	#define READ_BUFFER_SIZE 64
	char SerialReadBuffer[READ_BUFFER_SIZE]; /* Reserve SIZE bytes storage */
	
	ULONG Temp;
	ULONG WaitMask;
	ULONG midiDat;
	ULONG midiCnt;
	ULONG midiFreq;
	ULONG midiVel;
	ULONG midiMsg;
	UWORD playvoice;
	UWORD synthChns[32];
	
	BPTR lock;
	
	BOOL	board_found = FALSE;
	
    printf("\n=========================\nAmiGUS SYNTH Player V0.1\n(C) 2025 by Oliver Achten\n=========================\n\n");

	if (argc < 2)
	{
		printf("ERROR: wrong number of parameters\n\n");
		return 0;
	}
	else if (argc > 2)
	{
		control_word = (UWORD)strtol(argv[2], NULL, 16);
		printf("control_word = 0x%lx\n", control_word);
	}
	else
	{
//		control_word = 0x800d;
	}
	
	/* ================ Open libraries ================ */

	if((SysBase=OpenLibrary("exec.library",0L))==NULL)
	{
		printf("ERROR: something is really screwed here\n\n");
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
		if (board_manufacturer_id == AMIGUS_MANUFACTURER_ID && board_product_id == AMIGUS_HAGEN_PRODUCT_ID)
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
		printf("AmiGUS (Wave) found at $%lx\n",board_base);
		printf("FPGA Date: %4d-%02d-%02d, %02d:%02d\n\n",fpga_date_year,fpga_date_month,fpga_date_day,fpga_date_hour,fpga_date_minute);		
	}
	else
	{
		printf("ERROR: no AmiGUS board found!\n");	
		return 0;
	}
	
	
	val = -32768;

	WriteReg16(board_base,HAGEN_WRESET,0x0);
	WriteReg16(board_base,HAGEN_WADDRH,0x0);
	WriteReg16(board_base,HAGEN_WADDRL,0x0);

	for (i=0; i<42; i++)
	{
		WriteReg16(board_base,HAGEN_WDATAH,val);
		val+= 780;
		WriteReg16(board_base,HAGEN_WDATAL,val);
		val+= 780;		
	}


	/* Set-up all 32 voices */
	for (i=0; i<32; i++)
	{
		WriteReg16(board_base,HAGEN_VOICE_BNK,i);
		WriteReg16(board_base,HAGEN_VOICE_PSTRTH,0x0);
		WriteReg16(board_base,HAGEN_VOICE_PSTRTL,0x0);
		WriteReg16(board_base,HAGEN_VOICE_PLOOPH,0x0);
		WriteReg16(board_base,HAGEN_VOICE_PLOOPL,0x0);
		WriteReg32(board_base,HAGEN_VOICE_PENDH,84);
		WriteReg32(board_base,HAGEN_VOICE_RATEH,0x0);
		WriteReg16(board_base,HAGEN_VOICE_VOLUMEL,0x8000);
		WriteReg16(board_base,HAGEN_VOICE_VOLUMER,0x8000);
		
		WriteReg16(board_base,HAGEN_VOICE_ENV_ATTACK,0xf800);
		WriteReg16(board_base,HAGEN_VOICE_ENV_DECAY,0xf001);
		WriteReg16(board_base,HAGEN_VOICE_ENV_SUSTAIN,0x2000);
		WriteReg16(board_base,HAGEN_VOICE_ENV_RELEASE,0x0400);	
		WriteReg16(board_base,HAGEN_VOICE_ENV_RESET,0x0);				
	}
	// Set master volume
	WriteReg16(board_base,HAGEN_GLOBAL_VOLUMEL,0xffff);
	WriteReg16(board_base,HAGEN_GLOBAL_VOLUMER,0xffff);
	

	/* Create the message port */
	if (SerialMP=CreatePort("Serial",10))
		{
		/* Create the IORequest */
		if (SerialIO = (struct IOExtSer *)
						CreateExtIO(SerialMP,sizeof(struct IOExtSer)))
		{
			/* Open the serial device */
			if (OpenDevice(SERIALNAME,0,(struct IORequest *)SerialIO,0L))
				/* Inform user that it could not be opened */
				printf("Error: %s did not open\n",SERIALNAME);
			else
			{
				WaitMask = SIGBREAKF_CTRL_C|
							 1L << SerialMP->mp_SigBit;
							 
//                SerialIO->IOSer.io_Command  = SDCMD_QUERY;   /* device use, returns error or 0. */
//                if (DoIO((struct IORequest *)SerialIO))
//                    Printf("Query  failed. Error - %d\n", SerialIO->IOSer.io_Error);
//                else
//                    /* Print serial device status - see include file for meaning */
//                   /* Note that with DoIO, the Wait and GetMsg are done by Exec */
//                    printf("Serial device status: $%x\n\n", SerialIO->io_Status);							 

				SerialIO->IOSer.io_Command  = SDCMD_SETPARAMS;
				SerialIO->IOSer.io_Length   = 1;
				SerialIO->IOSer.io_Data     = (APTR)&SerialReadBuffer[0];
				SerialIO->io_Baud			= 31250;	// MIDI Baudrate
				SerialIO->io_ReadLen		= 8;	// MIDI Baudrate
				SerialIO->io_WriteLen  		= 8;     // 8 bit data
				SerialIO->io_StopBits  		= 1;     // 1 Stop bit
				SerialIO->io_SerFlags 		&= ~SERF_PARTY_ON; // No Parity
				SerialIO->io_SerFlags 		|= SERF_XDISABLED;	// No Flow Control	
				SerialIO->io_SerFlags 		|= SERF_RAD_BOOGIE; // Optimised protocol
             
				if (DoIO((struct IORequest *)SerialIO))
					printf("Set Params failed ");   /* Inform user of error */
				else
                {				
				
					printf("Receiving MIDI data on Channel #1\n");
					printf("Press CTRL-C to abort program\n");
					terminated = FALSE;
					midiCnt = 0;
					midiMsg = 0;
					playvoice = 0;

					for (i=0; i<32; i++)
					{
						synthChns[i]=0xffff;
					}

					// Main Loop
					
					while (terminated == FALSE)
					{
						SerialIO->IOSer.io_Command  = CMD_READ;
						SerialIO->IOSer.io_Length   = 1;
						SerialIO->IOSer.io_Data     = (APTR)&SerialReadBuffer[0];
						SendIO((struct IORequest *)SerialIO);
						
						Temp = Wait(WaitMask);

						if (SIGBREAKF_CTRL_C & Temp)
							terminated = TRUE;
								
						if (CheckIO((struct IORequest *)SerialIO) ) /* If request is complete... */
						{
							WaitIO((struct IORequest *)SerialIO);   /* clean up and remove reply */
							midiDat = (ULONG)SerialReadBuffer[0]&0xff;
							if (midiDat == MIDI_NOTE_OFF || midiDat == MIDI_NOTE_ON)
							{
								midiCnt = 0;
								midiMsg = midiDat;
							}
							else if (midiCnt != 3)
							{
								midiCnt++;
								if (midiCnt==1)
									midiFreq = midiDat;
								if (midiCnt==2)
								{
									midiVel = midiDat;
									if (midiMsg == MIDI_NOTE_ON)
									{
										if (midiVel != 0)
										{
											synthChns[playvoice] = (UWORD)midiFreq;
											hagenPlayNote(board_base, (UWORD)playvoice, (UWORD)midiFreq, (UWORD)midiVel);
											playvoice++;
												if (playvoice == 32)
													playvoice = 0;
										}
										else
										{
											for (i=0; i<32; i++)
											{
												if ((UWORD)midiFreq == synthChns[i]);
												{
													synthChns[i] = 0xffff;
													hagenReleaseNote(board_base, (UWORD)i);
												}
											}	
										}
									}
									else if (midiMsg == MIDI_NOTE_OFF)
									{
										for (i=0; i<32; i++)
										{
											if ((UWORD)midiFreq == synthChns[i]);
											{
												synthChns[i] = 0xffff;
												hagenReleaseNote(board_base, (UWORD)i);
											}
										}
									}							
								}
							}
						}						
					}

				}
				printf("Program aborted!\n");
				AbortIO((struct IORequest *)SerialIO);  /* Ask device to abort request, if pending */
				WaitIO((struct IORequest *)SerialIO);   /* Wait for abort, then clean up */
				CloseDevice((struct IORequest *)SerialIO);
			}
			/* Delete the IORequest */
			DeleteExtIO((struct IORequest *)SerialIO);
		}
		else
			/* Inform user that the IORequest could be created */
			printf("Error: Could create IORequest\n");

		/* Delete the message port */
		DeletePort(SerialMP);
		}
	else
		/* Inform user that the message port could not be created */
		printf("Error: Could not create message port\n");
	
	
	
	CloseLibrary(SysBase);	

	return 0;  
   }
