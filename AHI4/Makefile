#
# This file is part of the AmiGUS.audio driver.
#
# AmiGUS.audio driver is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 3 of the License only.
#
# AmiGUS.audio driver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU LesserGeneral Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
#

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

AHI_INCLUDE     = include
NDK             = NDK_3.2:
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

MODE_FILE_DST   = $(DDIR)AMIGUS
MODE_FILE_SRC   = $(SDIR)audiomodes.asm
MODE_FILE_ASM   = vasmm68k_mot
MODE_FILE_FLAGS = -quiet -phxass -I$(NDK_ASM_INCLUDE) -I$(AHI_INCLUDE)/Asm -Fbin

SRCS            = $(wildcard $(SDIR)#?.c)
BDIR_SRCS       = $(foreach SRC,$(SRCS),$(subst $(SDIR),$(BDIR),$(SRC)))
BDIR_OBJS       = $(BDIR_SRCS:%c=%o)
LIB_OBJS        = $(BDIR)libinit.o \
                  $(filter-out %/libinit.o,$(BDIR_OBJS))
LIB_DST         = $(DDIR)AmiGUS.audio

# activate VBCC builds by adding `VBCC=1` to the make call!
ifdef VBCC
  CFLAGS        = -O2 -c -I$(AHI_INCLUDE)/C -I$(HDIR) -D__NOLIBBASE__ -o
  LFLAGS        = -nostdlib -o
  LFLAGS1       =
  CC            = vc
  LD            = vc
else
  CFLAGS        = RESETOPTIONS DEFINE=__NOLIBBASE__                         \
                  PARAMETERS=REGISTER DATA=FARONLY CODE=FAR                 \
                  NOSTACKCHECK NOCHECKABORT NOICONS                         \
                  IDIR=$(AHI_INCLUDE)/C IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)  \
                  OBJNAME
                  
  xxx=            STRINGMERGE UNSIGNEDCHAR                                  \
                  OPTIMIZE OPTIMIZERGLOBAL OPTIMIZERINLINE OPTIMIZERINLOCAL \
                  OPTIMIZERLOOP OPTIMIZERPEEPHOLE OPTIMIZERSCHEDULE         \
                  OBJNAME

  LFLAGS        = NOICONS SMALLDATA SMALLCODE STRIPDEBUG \
                  TO 
  LFLAGS1       = FROM
  CC            = sc
  LD            = slink
endif

all: FOLDERS $(MODE_FILE_DST) $(LIB_DST)

$(MODE_FILE_DST): $(MODE_FILE_SRC)
	$(MODE_FILE_ASM) $(MODE_FILE_FLAGS) $^ -o $@

$(LIB_DST): $(LIB_OBJS)
	$(LD) $(LFLAGS) $@ $(LFLAGS1) $^
	
$(BDIR)%.o: $(SDIR)%.c $(HDIR)*
	$(CC) $(CFLAGS) $@ $<

install: all
	copy FROM $(MODE_FILE_DST)     	TO	Devs:AudioModes/	CLONE 
	copy FROM $(LIB_DST)	TO	Devs:AHI/		CLONE
	avail flush >NIL:
#	tools/FlushLibs
#	AddAudioModes devs:AudioModes/AMIGUS

INCLUDES:
	make -f Makefile-Includes all 

FOLDERS:
	-@makedir $(DDIR) ALL >NIL:
	-@makedir $(BDIR) ALL >NIL:

clean:
	-delete $(DDIR) $(BDIR) ALL >NIL:

dist-clean: clean
	make -f Makefile-Includes clean
