#
# This file is part of the AmiGUS.audio driver.
#
# AmiGUS.audio driver is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 3 of the License only.
#
# AmiGUS.audio driver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with AmiGUS.audio driver.
#
# If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(findstring /home,$(HOME)), /home)
  LINUX          = 1

  NDK            = $(VBCC)/NDK3.2/
  WILDCARD       = *

  MKDIR          = mkdir -p
  MKDIR_SUFFIX   =
  RM             = rm -rf
  RM_SUFFIX      =
  RUNNER         = vamos -c ../.vamosrc
  IGNORE_VAMOS   = -
  LHA            = jlha

  HOST_TOOLCHAIN = CROSS_TOOLCHAIN

  # Use .netrc to store passwords! 
  # ~/.netrc format:
  # machine <ip> login username password pass
  # Could do --user username:pass instead - but not cool in repo ;)
  INSTALL0       = curl --netrc --upload-file $(MODE_FILE_DST) ftp://192.168.0.4/cf0/Devs/AudioModes/
  INSTALL1       = curl --netrc --upload-file $(LIB_DST)       ftp://192.168.0.4/cf0/Devs/AHI/
  INSTALL2       =
else
  AMIGAOS       = 1
  
  NDK           = NDK_3.2:
  WILDCARD      = \#?

  MKDIR         = makedir
  MKDIR_SUFFIX  = ALL >NIL:
  RM            = delete
  RM_SUFFIX     = ALL >NIL:
  RUNNER        =
  IGNORE_VAMOS  =
  LHA           = lha

  HOST_TOOLCHAIN = NATIVE_TOOLCHAIN

  INSTALL0       = copy FROM $(MODE_FILE_DST)     	TO	Devs:AudioModes/	CLONE 
  INSTALL1       = copy FROM $(LIB_DST)	TO	Devs:AHI/		CLONE
  INSTALL2       = avail flush >NIL:
endif

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

LIB_CPU        ?= 000
LIB_LOG        ?= FILE_LOG

AHI_INCLUDE     = include
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

MODE_FILE_DST   = $(DDIR)AMIGUS
MODE_FILE_SRC   = $(SDIR)audiomodes.asm
MODE_FILE_ASM   = vasmm68k_mot
MODE_FILE_FLAGS = -quiet -phxass -I$(NDK_ASM_INCLUDE) -I$(AHI_INCLUDE)/ASM -Fbin

SRCS            = $(wildcard $(SDIR)$(WILDCARD).c)
BDIR_SRCS       = $(foreach SRC,$(SRCS),$(subst $(SDIR),$(BDIR),$(SRC)))
BDIR_OBJS       = $(BDIR_SRCS:%.c=%-$(LIB_CPU)-$(LIB_LOG).o)
LIB_OBJS        = $(BDIR)library-$(LIB_CPU)-$(LIB_LOG).o \
                  $(filter-out %/library-$(LIB_CPU)-$(LIB_LOG).o,$(BDIR_OBJS))
LIB_DST         = $(DDIR)AmiGUS.audio-$(LIB_CPU)-$(LIB_LOG)

# activate VBCC builds by adding `USEVBCC=1` to the make call!
ifdef USEVBCC
  ifdef AMIGAOS
    LDIR        = vlibos3:
  endif
  ifdef LINUX
    LDIR        = $(VBCC)/targets/m68k-amigaos/lib/
  endif
  
  CFLAGS        = -O2 -c -cpu=68$(LIB_CPU) -I$(AHI_INCLUDE)/C -I$(HDIR)       \
                  -D__NOLIBBASE__ -D$(HOST_TOOLCHAIN) -o
  LFLAGS        = -nostdlib -o
  LFLAGS1       =
  LFLAGST1      = $(LDIR)startup.o
  LFLAGST2      = $(LDIR)auto.lib $(LDIR)vc.lib
  # add -v below to see what vbcc does under the hood :)
  CC            = vc +aos68k
  LD            = vc +aos68k
else
  DEAC_CFLAGS   = RESETOPTIONS DEFINE=__NOLIBBASE__ DEFINE=$(HOST_TOOLCHAIN)  \
                  PARAMETERS=REGISTER DATA=FAR CODE=FAR                       \
                  NOSTACKCHECK NOCHECKABORT NOICONS                           \
                  IDIR=$(AHI_INCLUDE)/C IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)    \
                  OBJNAME

  CFLAGS        = RESETOPTIONS                                                \
                  DEF __NOLIBBASE__ DEF $(LIB_LOG) DEF $(HOST_TOOLCHAIN)      \
                  PARAMETERS=REGISTER DATA=FAR CODE=FAR                       \
                  NOSTACKCHECK NOCHECKABORT NOICONS                           \
                  IDIR=$(AHI_INCLUDE)/C IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)    \
                  STRINGMERGE UNSIGNEDCHAR CPU=68$(LIB_CPU)                   \
                  OPTIMIZE OPTIMIZERGLOBAL OPTIMIZERINLINE OPTIMIZERINLOCAL   \
                  OPTIMIZERLOOP OPTIMIZERPEEPHOLE OPTIMIZERSCHEDULE           \
                  OBJNAME

  LFLAGS        = NOICONS SMALLDATA SMALLCODE STRIPDEBUG \
                  TO 
  LFLAGS1       = FROM
  LFLAGST1      = LIB:c.o
  LFLAGST2      = LIBRARY LIB:sc.lib LIB:amiga.lib
  CC            = $(RUNNER) sc
  LD            = $(RUNNER) slink
endif

.PHONY: all install test support INCLUDES FOLDERS clean dist-clean dist

all: FOLDERS $(MODE_FILE_DST) $(LIB_DST)

ECHO:
	echo $(SRCS)

$(MODE_FILE_DST): $(MODE_FILE_SRC)
	$(MODE_FILE_ASM) $(MODE_FILE_FLAGS) $^ -o $@

$(LIB_DST): $(LIB_OBJS)
	$(LD) $(LFLAGS) $@ $(LFLAGS1) $^
	
$(BDIR)%-$(LIB_CPU)-$(LIB_LOG).o: $(SDIR)%.c $(HDIR)*
	$(CC) $(CFLAGS) $@ $<

install: all
	$(INSTALL0)
	$(INSTALL1)
	$(INSTALL2)

test: test_buffers test_copies test_support

test_buffers: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)buffers.o \
            $(BDIR)amigus_ahi_modes.o $(BDIR)copies.o \
            $(BDIR)debug.o $(BDIR)support.o \
            $(LFLAGST2)
	@echo " "
	$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

test_copies: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)copies.o \
            $(BDIR)amigus_ahi_modes.o \
            $(LFLAGST2)
	@echo " "
	$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

test_support: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)support.o \
            $(BDIR)debug.o \
            $(LFLAGST2)
	@echo " "
	$(IGNORE_VAMOS)$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

###############################################################################

support: FOLDERS
	echo "Hi"

###############################################################################

INCLUDES:
	make -f Makefile-Includes all 

FOLDERS:
	-@$(MKDIR) $(DDIR) $(MKDIR_SUFFIX)
	-@$(MKDIR) $(BDIR) $(MKDIR_SUFFIX)

clean:
	-$(RM) $(DDIR) $(BDIR) $(RM_SUFFIX)

dist-clean: clean
	make -f Makefile-Includes clean

dist: all
	$(LHA) a target/AHI4bin.lha target/$(WILDCARD)
