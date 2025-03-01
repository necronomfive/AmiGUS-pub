#
# This file is part of the mhiAmiGUS.library.
#
# mhiAmiGUS.library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 3 of the License only.
#
# mhiAmiGUS.library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU LesserGeneral Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(findstring /home,$(HOME)), /home)
  LINUX          = 1

  NDK            = $(VBCC)/NDK3.2/
  WILDCARD       = *

  MKDIR          = mkdir -p
  MKDIR_SUFFIX   =
  RM             = rm -rf
  RM_SUFFIX      =
  RUNNER         = vamos
  IGNORE_VAMOS   = -
  LHA            = jlha

  HOST_TOOLCHAIN = CROSS_TOOLCHAIN

  # Use .netrc to store passwords! 
  # ~/.netrc format:
  # machine <ip> login username password pass
  # Could do --user username:pass instead - but not cool in repo ;)
  INSTALL0       = curl --netrc --upload-file $(LIB_DST)       ftp://192.168.0.4/cf0/Libs/MHI/
  INSTALL1       =
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

  INSTALL0       = copy FROM $(LIB_DST)	TO	Libs:MHI/		CLONE
  INSTALL1       = avail flush >NIL:
endif

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

MHI_INCLUDE     = include
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

SRCS            = $(wildcard $(SDIR)$(WILDCARD).c)
BDIR_SRCS       = $(foreach SRC,$(SRCS),$(subst $(SDIR),$(BDIR),$(SRC)))
BDIR_OBJS       = $(BDIR_SRCS:%c=%o)
LIB_OBJS        = $(BDIR)library.o \
                  $(filter-out %/library.o,$(BDIR_OBJS))
LIB_DST         = $(DDIR)mhiAmiGUS.library

# activate VBCC builds by adding `USEVBCC=1` to the make call!
ifdef USEVBCC
  ifdef AMIGAOS
    LDIR        = vlibos3:
  endif
  ifdef LINUX
    LDIR        = $(VBCC)/targets/m68k-amigaos/lib/
  endif
  
  CFLAGS        = -O2 -c -I$(MHI_INCLUDE) -I$(HDIR)                          \
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
                  IDIR=$(MHI_INCLUDE) IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)    \
                  OBJNAME

  CFLAGS        = RESETOPTIONS DEFINE=__NOLIBBASE__ DEFINE=$(HOST_TOOLCHAIN)  \
                  PARAMETERS=REGISTER DATA=FAR CODE=FAR                       \
                  NOSTACKCHECK NOCHECKABORT NOICONS                           \
                  IDIR=$(MHI_INCLUDE) IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)    \
                  STRINGMERGE UNSIGNEDCHAR                                    \
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

.PHONY: all install test INCLUDES FOLDERS clean dist-clean dist

all: FOLDERS $(MODE_FILE_DST) $(LIB_DST)

ECHO:
	echo $(SRCS)

$(MODE_FILE_DST): $(MODE_FILE_SRC)
	$(MODE_FILE_ASM) $(MODE_FILE_FLAGS) $^ -o $@

$(LIB_DST): $(LIB_OBJS)
	$(LD) $(LFLAGS) $@ $(LFLAGS1) $^
	
$(BDIR)%.o: $(SDIR)%.c $(HDIR)*
	$(CC) $(CFLAGS) $@ $<

install: all
	$(INSTALL0)
	$(INSTALL1)

test: test_playback

test_playback: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)interrupt.o \
            $(BDIR)debug.o $(BDIR)support.o \
            $(LFLAGST2)
	@echo " "
	$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

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
	$(LHA) a target/MHIbin.lha target/$(WILDCARD)
