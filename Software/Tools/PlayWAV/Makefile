#
# This file is part of the AmiGUS Flash Utility.
#
# AmiGUS Flash Utility is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License only.
#
# AmiGUS Flash Utility is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with AmiGUS Flash Utility.  If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(findstring /home,$(HOME)), /home)
  LINUX         = 1

  WILDCARD      = *

  MKDIR         = mkdir -p
  MKDIR_SUFFIX  =
  RM            = rm -rf
  RM_SUFFIX     =
  CP            = cp
  RUNNER        = vamos -c ../.vamosrc
  IGNORE_VAMOS  = -
  LHA           = jlha
else
  AMIGAOS       = 1

  WILDCARD      = \#?

  MKDIR         = makedir
  MKDIR_SUFFIX  = ALL >NIL:
  RM            = delete
  RM_SUFFIX     = ALL >NIL:
  CP            = copy
  RUNNER        =
  IGNORE_VAMOS  =
  LHA           = lha
endif

TARGET          = PlayWav
FOLDER          = PlayWAV

.PHONY: clean release release0 release01

all: $(TARGET)

clean:
	-$(RM) $(WILDCARD).o $(WILDCARD).lnk $(TARGET) $(RM_SUFFIX)

$(TARGET):
	$(RUNNER) sc $(TARGET).c asmroutines.a incdir=include: cpu=68000 link

release: $(TARGET) release0$(LINUX)

# Amiga Version:
release0:
	$(LHA) a Release/$(FOLDER)_XXX.lha /$(FOLDER)/ $(WILDCARD)

# Linux Version:
release01:
	cd ..                                                       ; \
	FILES=`find $(FOLDER) -maxdepth 1 -type f`                  ; \
	$(LHA) a \
		$(FOLDER)/Release/$(TARGET)_`date +%y%m%d`.lha \
		$$FILES
