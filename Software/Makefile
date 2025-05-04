DRIVER_MAKES = $(shell find Drivers -mindepth 1 -maxdepth 2 -type f -name Makefile)
DRIVERS = $(dir $(DRIVER_MAKES))
DRIVERS/ALL = $(foreach DRIVER,$(DRIVERS),$(DRIVER)/all)
DRIVERS/CLEAN = $(foreach DRIVER,$(DRIVERS),$(DRIVER)clean)
DRIVERS/DIST-CLEAN = $(foreach DRIVER,$(DRIVERS),$(DRIVER)/dist-clean)

CPUS = 000 020 040
LOGGINGS = NO_LOG SER_LOG FILE_LOG MEM_LOG

TOOLS = $(shell find Tools -mindepth 1 -maxdepth 1 -type d)
TOOLS/ALL = $(foreach TOOL,$(TOOLS),$(TOOL)/all)
TOOLS/CLEAN = $(foreach TOOL,$(TOOLS),$(TOOL)/clean)

.PHONY: $(DRIVERS/ALL) $(DRIVERS/CLEAN) $(DRIVERS/DIST-CLEAN) \
        $(TOOLS/ALL) $(TOOLS/CLEAN) \
		clean all

ADF_IMAGE  = /tmp/AmiGUS.adf
ADF_DIR    = /tmp/AmiGUS-ADF
TARGET_DIR = /tmp/AmiGUS
TARGET_LHA = /tmp/AmiGUS-`date +%Y%m%d`.lha
TOOL_DIR   = $(TARGET_DIR)/Tools
AHI_DIR = $(TARGET_DIR)/AHI4
AUDIOMODES_DIR = $(AHI_DIR)/AudioModes
AHIFILE_DIR = $(AHI_DIR)/AHI
MHI_DIR = $(TARGET_DIR)/MHI

release: all
	-umount $(ADF_DIR)
	rm -rf $(TARGET_DIR) $(TARGET_LHA) $(ADF_DIR) $(ADF_IMAGE)
	mkdir -p $(ADF_DIR) $(TOOL_DIR) $(AUDIOMODES_DIR) $(AHIFILE_DIR) $(MHI_DIR)

	cp drawer.info                     $(TARGET_DIR).info
	cp drawer.info                     $(TOOL_DIR).info
	cp drawer.info                     $(AHI_DIR).info
	cp drawer.info                     $(MHI_DIR).info

	cp Tools/FlashFPGA/FlashGUI        $(TOOL_DIR)
	cp Tools/FlashFPGA/FlashGUI.info   $(TOOL_DIR)
	cp Tools/Mixer/Mixer               $(TOOL_DIR)
	cp Tools/Mixer/Mixer.info          $(TOOL_DIR)
	cp Tools/PlayMIDI/PlayMidi         $(TOOL_DIR)
	cp Tools/PlayMIDI/PlayMidi.info    $(TOOL_DIR)
	cp Tools/PlayMP3/PlayMP3           $(TOOL_DIR)
	cp Tools/PlayMP3/PlayMP3.info      $(TOOL_DIR)
	cp Tools/PlayWAV/PlayWav           $(TOOL_DIR)
	cp Tools/PlayWAV/PlayWav.info      $(TOOL_DIR)
	cp Tools/Record/Record             $(TOOL_DIR)
	cp Tools/Record/Record.info        $(TOOL_DIR)
	cp Drivers/AHI4/target/*.audio*    $(AHIFILE_DIR)
	cp Drivers/AHI4/target/AMIGUS      $(AUDIOMODES_DIR)
	cp Drivers/MHI/target/*            $(MHI_DIR)

	cd `dirname $(TARGET_DIR)` ; jlha a $(TARGET_LHA) `basename $(TARGET_DIR)`*

	adf_floppy_create $(ADF_IMAGE) dd
	adf_format -l AmiGUS -t 0 -f $(ADF_IMAGE)
	fuseadf $(ADF_IMAGE) $(ADF_DIR)/
	cp -R $(TARGET_DIR)/* $(ADF_DIR)/
	umount $(ADF_DIR)/
	-cp $(ADF_IMAGE) /media/chritoph/HXC/

all: tools drivers

tools: $(TOOLS/ALL)

drivers: prepare-ahi prepare-mhi $(DRIVERS/ALL)

prepare-ahi:
	mkdir -p /tmp/ahi4-installation Drivers/AHI4/include
	wget  -P /tmp/ahi4-installation https://www.lysator.liu.se/ahi/v4-site/files/ahi/system/ahidev.lha
	cd       /tmp/ahi4-installation ; jlha x ahidev.lha
	cp -R    /tmp/ahi4-installation/AHI/Developer/include/C     Drivers/AHI4/include
	cp -R    /tmp/ahi4-installation/AHI/Developer/include/Asm   Drivers/AHI4/include/ASM
	touch prepare-ahi

prepare-mhi:
	mkdir -p /tmp/mhi-installation Drivers/MHI/include
	wget  -P /tmp/mhi-installation https://aminet.net/driver/audio/mhi_dev.lha
	cd       /tmp/mhi-installation ; jlha x mhi_dev.lha
	cp -R    /tmp/mhi-installation/Include/libraries            Drivers/MHI/include
	touch prepare-mhi

clean: $(DRIVERS/CLEAN) $(TOOLS/CLEAN)

dist-clean: $(DRIVERS/DIST-CLEAN)

$(DRIVERS/CLEAN) $(TOOLS/ALL) $(TOOLS/CLEAN):
	$(MAKE) -C $(@D) $(@F)

$(DRIVERS/ALL):
	for CPU in $(CPUS)                                              ;\
	do                                                               \
		for LOGGING in $(LOGGINGS)                                  ;\
		do                                                           \
			$(MAKE) -C $(@D) $(@F) LIB_CPU=$$CPU LIB_LOG=$$LOGGING  ;\
		done                                                        ;\
	done
