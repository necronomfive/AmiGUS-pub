# Makefile for example library
## use with: make 
#
# assumes to have the compiler (m68k-amigaos-gcc or vc) in path
# also assumes that for vbcc, the "kick13" target is present
# in the installation
#
#
TARGET1  = mhiamigus.library 
TARGET2  = libtest

DEFINES  = -DLIBNAME=mhiamigus.library 
#see version.h
#DEFINES += -DLIBVERSION=1
#DEFINES += -DLIBREVISION=2
#DEFINES += -DLIBDATE=23.05.2019
#DEFINES += -DLIBEXTRA=FPU

# compiler_vcc = 1 will trigger VBCC, else GCC for Amiga builds
# note: GCC builds assume bebbo version, change -Fhunk to -Faout for gcc2.95
compiler_vcc = 1

RM	= rm -f

ifeq ($(compiler_vcc),1)
# VBCC (use explicit vlink line for LINK= if complaints about R_PC happen)
CC  = vc
# -Rshort -Bstatic
LINKEXE = vlink -bamigahunk -x -s -mrel -Cvbcc -Bstatic -nostdlib
# -sc
CFLAGS  = -Os -+ +kick13 -c99 -cpu=68000 -I. $(DEFINES)
VASM    = vasmm68k_mot -quiet -I$(PREFIX)
LDFLAGS =
else
CC      = m68k-amigaos-gcc
CFLAGS  = -I. -mcrt=nix13 -Wall -O2 -fomit-frame-pointer -msmall-code $(DEFINES)
CFLAGS  += -Wno-strict-aliasing -Wno-pointer-to-int-cast
#CFLAGS += -m68020 -m68881
LDFLAGS = -s -mcrt=nix13 -noixemul -nostartfiles
LINKEXE = $(CC)
endif

OBJS1	= libfuncs_impl.o mhi_all.o
OBJS	= libheader.o libinit.o $(OBJS1)
HEADERS = config.h

all: $(TARGET1) $(TARGET2)

install: all
	cp $(TARGET1) ~/Documents/FS-UAE/Shared/MHI/
	cp $(TARGET2) ~/Documents/FS-UAE/Shared/MHI/

$(TARGET1): $(OBJS)
	$(LINKEXE) $(LDFLAGS) -o $(TARGET1) $(OBJS)

$(TARGET2): libtest.o
	$(CC) $(CFLAGS) -o $@ libtest.o

clean:
	-$(RM) $(TARGET1) $(TARGET2) $(OBJS) libtest.o


# dependencies

