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

  LIB_HOST       = cross

  # Use .netrc to store passwords! 
  # ~/.netrc format:
  # machine <ip> login username password pass
  # Could do --user username:pass instead - but not cool in repo ;)
  INSTALL0       = curl --netrc --upload-file $(MODE_FILE_DEST) ftp://192.168.0.4/cf0/Devs/AudioModes/
  INSTALL1       = curl --netrc --upload-file $(LIB_FILE_DEST)  ftp://192.168.0.4/cf0/Devs/AHI/$(LIB_NAME).audio
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

  LIB_HOST      = native

  INSTALL0       = copy FROM $(MODE_FILE_DEST) TO	Devs:AudioModes/	CLONE 
  INSTALL1       = copy FROM $(LIB_FILE_DEST)	 TO	Devs:AHI/$(LIB_NAME).audio		CLONE
  INSTALL2       = avail flush >NIL:
endif

DDIR   = target/
BDIR   = build/
SDIR   = src/
HDIR   = header/

LIB_CPU        ?= 000
LIB_LOG        ?= MEM_LOG
# NO_LOG
# SER_LOG
# FILE_LOG
# MEM_LOG
LIB_NAME        = AmiGUS
LIB_FILE        = $(LIB_NAME).audio
LIB_VERSION     = 4
LIB_REVISION    = 19
LIB_PREFIX      = _LIB_

LIB_LIBS        =
LIB_FILE_DEST   = $(DDIR)$(LIB_FILE)-$(LIB_CPU)-$(LIB_LOG)

###########################

AHI_INCLUDE     = include
NDK_ASM_INCLUDE = $(NDK)Include_I
NDK_C_INCLUDE   = $(NDK)Include_H

MODE_FILE_DEST  = $(DDIR)AMIGUS
MODE_FILE_SRC   = $(SDIR)audiomodes.asm
MODE_FILE_ASM   = vasmm68k_mot
MODE_FILE_FLAGS = -quiet -phxass -I$(NDK_ASM_INCLUDE) -I$(AHI_INCLUDE)/ASM -Fbin

C_SRCS          = $(wildcard $(SDIR)$(WILDCARD).c)
BDIR_C_SRCS     = $(foreach C_SRC,$(C_SRCS),$(subst $(SDIR),$(BDIR),$(C_SRC)))
BDIR_C_OBJS     = $(BDIR_C_SRCS:%.c=%-$(LIB_CPU)-$(LIB_LOG).o)

A_SRCS          = $(filter-out %$(MODE_FILE_SRC),$(wildcard $(SDIR)$(WILDCARD).asm))
BDIR_A_SRCS     = $(foreach A_SRC,$(A_SRCS),$(subst $(SDIR),$(BDIR),$(A_SRC)))
BDIR_A_OBJS     = $(BDIR_A_SRCS:%.asm=%.o)

LIB_OBJS        = $(BDIR)library-$(LIB_CPU)-$(LIB_LOG).o \
                  $(filter-out %/library-$(LIB_CPU)-$(LIB_LOG).o,$(BDIR_C_OBJS)) \
                  $(filter-out %/library-$(LIB_CPU)-$(LIB_LOG).o,$(BDIR_A_OBJS))

###########################

# activate VBCC builds by adding `USEVBCC=1` to the make call!
ifdef USEVBCC
  ifdef AMIGAOS
    LDIR        = vlibos3:
  endif
  ifdef LINUX
    LDIR        = $(VBCC)/targets/m68k-amigaos/lib/
  endif

  AFLAGS        = -I$(NDK_ASM_INCLUDE) -quiet -phxass -Fhunk -o
  CFLAGS        = -O2 -c -cpu=68$(LIB_CPU)                             \
                  -I$(AHI_INCLUDE)/C -I$(HDIR)                         \
                  -D__NOLIBBASE__ -D$(LIB_LOG)                         \
                  -DLIB_FILE=$(LIB_FILE) -DLIB_DATE=__AMIGADATE__      \
                  -DLIB_VERSION=$(LIB_VERSION)                         \
                  -DLIB_REVISION=$(LIB_REVISION) -DLIB_CPU=$(LIB_CPU)  \
                  -DLIB_COMPILER=VBCC -DLIB_HOST=$(LIB_HOST)           \
                  -o
  LFLAGS        = -bamigahunk -x -s -mrel -Cvbcc -Bstatic -nostdlib -o
  LFLAGS1       =
  LFLAGST1      = $(LDIR)startup.o
  LFLAGST2      = $(LDIR)auto.lib $(LDIR)vc.lib
  # add -v below to see what vbcc does under the hood :)
  CC            = vc +aos68k
  AS            = vasmm68k_mot
  LD            = vc +aos68k
else
  AFLAGS        = RESETOPTIONS DEFINE=__NOLIBBASE__ DEFINE=$(LIB_HOST)         \
                  IDIR=$(AHI_INCLUDE)/ASM IDIR=NDK_3.2:Include_I IDIR=$(HDIR)  \
                  OBJNAME

  CFLAGS        = RESETOPTIONS                                                 \
                  PARAMETERS=REGISTER DATA=FAR CODE=FAR                        \
                  NOSTACKCHECK NOCHECKABORT NOICONS                            \
                  STRINGMERGE UNSIGNEDCHAR CPU=68$(LIB_CPU)                    \
                  OPTIMIZE OPTIMIZERGLOBAL OPTIMIZERINLINE OPTIMIZERINLOCAL    \
                  OPTIMIZERLOOP OPTIMIZERPEEPHOLE OPTIMIZERSCHEDULE            \
                  IDIR=$(AHI_INCLUDE)/C IDIR=$(NDK_C_INCLUDE) IDIR=$(HDIR)     \
                  DEF __NOLIBBASE__ DEF $(LIB_LOG)                             \
                  DEF LIB_FILE=$(LIB_FILE) DEF LIB_DATE=__AMIGADATE__          \
                  DEF LIB_VERSION=$(LIB_VERSION)                               \
                  DEF LIB_REVISION=$(LIB_REVISION) DEF LIB_CPU=$(LIB_CPU)      \
                  DEF LIB_COMPILER=SAS/C DEF LIB_HOST=$(LIB_HOST)              \
                  OBJNAME

  LFLAGS        = NOICONS SMALLDATA SMALLCODE STRIPDEBUG \
                  TO 
  LFLAGS1       = FROM
  LFLAGST1      = LIB:c.o
  LFLAGST2      = LIBRARY LIB:sc.lib LIB:amiga.lib
  CC            = $(RUNNER) sc
  AS            = $(RUNNER) sc
  LD            = $(RUNNER) slink
  FD            = $(RUNNER) sc:c/fd2pragma
endif

##################################

.PHONY: all install test support INCLUDES FOLDERS clean dist-clean dist

all: FOLDERS $(MODE_FILE_DEST) $(LIB_FILE_DEST)

ECHO:
	@echo "C-sources: "$(C_SRCS)
	@echo "ASM-sources: "$(A_SRCS)
	@echo "USEVBCC: "$(USEVBCC)

install: all
	$(INSTALL0)
	$(INSTALL1)
	$(INSTALL2)

clean-intermediate:
	-$(RM) $(BDIR) $(RM_SUFFIX)

clean: clean-intermediate
	-$(RM) $(DDIR) adf $(RM_SUFFIX)
	-$(RM) support/$(WILDCARD).o

dist-clean: clean
	make -f Makefile-Includes clean

dist: all
	$(LHA) a target/AHI4bin.lha target/$(WILDCARD)

INCLUDES:
	make -f Makefile-Includes all 

FOLDERS:
	-@$(MKDIR) $(DDIR) $(MKDIR_SUFFIX)
	-@$(MKDIR) $(BDIR) $(MKDIR_SUFFIX)

$(MODE_FILE_DEST): $(MODE_FILE_SRC)
	$(MODE_FILE_ASM) $(MODE_FILE_FLAGS) $^ -o $@

$(LIB_FILE_DEST): $(LIB_OBJS)
	$(LD) $(LFLAGS) $@ $(LFLAGS1) $^

###############################################################################

$(BDIR)%-$(LIB_CPU)-$(LIB_LOG).o: $(SDIR)%.c $(HDIR)*
	$(CC) $(CFLAGS) $@ $<

$(BDIR)%.o: $(SDIR)%.asm $(HDIR)*
	$(AS) $(AFLAGS) $@ $<

###############################################################################

test: test_buffers test_copies test_support

test_buffers: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)buffers-$(LIB_CPU)-$(LIB_LOG).o \
            $(BDIR)amigus_ahi_modes-$(LIB_CPU)-$(LIB_LOG).o \
            $(BDIR)copies-$(LIB_CPU)-$(LIB_LOG).o \
            $(BDIR)debug-$(LIB_CPU)-$(LIB_LOG).o \
            $(BDIR)support-$(LIB_CPU)-$(LIB_LOG).o \
            $(LFLAGST2)
	@echo " "
	$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

test_copies: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)copies-$(LIB_CPU)-$(LIB_LOG).o \
            $(BDIR)amigus_ahi_modes-$(LIB_CPU)-$(LIB_LOG).o \
            $(LFLAGST2)
	@echo " "
	$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

test_support: FOLDERS $(LIB_OBJS)
	@echo " "
	$(CC) $(CFLAGS) test/$@.o test/$@.c
	$(LD) $(LFLAGS) test/$@ $(LFLAGS1) $(LFLAGST1) test/$@.o \
            $(BDIR)support-$(LIB_CPU)-$(LIB_LOG).o \
            $(BDIR)debug-$(LIB_CPU)-$(LIB_LOG).o \
            $(LFLAGST2)
	@echo " "
	$(IGNORE_VAMOS)$(RUNNER) test/$@
	-@$(RM) test/$@ test/$@.o $(RM_SUFFIX)

###############################################################################

support: FOLDERS get_mem_log

get_mem_log:
	$(CC) $(CFLAGS) support/$@.o support/$@.c
	$(LD) $(LFLAGS) target/$@ $(LFLAGS1) $(LFLAGST1) support/$@.o $(LFLAGST2)
	-cp target/$@ ~/Documents/FS-UAE/Shared/AHI/
	-curl --netrc --upload-file target/$@       ftp://192.168.0.4/cf0/Expansion/AmiGUS/GetAhiMemLog

###############################################################################

ADF: all support
	mkdir -p adf
	# bzip2 -c -d src/empty.adf.bz2 > target/AmiGUS.adf
	adf_floppy_create target/AmiGUS.adf dd
	adf_format -l AmiGUS -t 0 -f target/AmiGUS.adf
	fuseadf target/AmiGUS.adf adf/
	cp target/AmiGUS.audio adf/
	cp target/AMIGUS adf/
	cp target/get_mem_log adf/
	umount adf/
	-cp target/AmiGUS.adf /media/chritoph/HXC/
