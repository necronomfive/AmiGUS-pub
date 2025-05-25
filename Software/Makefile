TARGET_DIR         = /tmp/AmiGUS
ADF_DIR1           = /tmp/AmiGUS-ADF1
ADF_DIR2           = /tmp/AmiGUS-ADF2
TOOL_DIR           = $(TARGET_DIR)/Tools
AHI_DIR            = $(TARGET_DIR)/AHI4
AHIFILE_DIR        = $(AHI_DIR)/AHI
AUDIOMODES_DIR     = $(AHI_DIR)/AudioModes
MHI_DIR            = $(TARGET_DIR)/MHI
FW_DIR             = $(TARGET_DIR)/Firmware
MANUAL_DIR         = $(TARGET_DIR)/Manual

TARGET_LHA         = /tmp/AmiGUS-`date +%Y%m%d`.lha
ADF_IMAGE1         = /tmp/AmiGUS-Tools-`date +%Y%m%d`.adf
ADF_IMAGE2         = /tmp/AmiGUS-Manual-`date +%Y%m%d`.adf

DRIVER_MAKES = $(shell find Drivers -mindepth 1 -maxdepth 2 -type f -name Makefile)
DRIVERS = $(dir $(DRIVER_MAKES))
DRIVERS/ALL = $(foreach DRIVER,$(DRIVERS),$(DRIVER)/all)
DRIVERS/CLEAN = $(foreach DRIVER,$(DRIVERS),$(DRIVER)clean)
DRIVERS/DIST-CLEAN = $(foreach DRIVER,$(DRIVERS),$(DRIVER)/dist-clean)
DRIVERS/SUPPORT = $(foreach DRIVER,$(DRIVERS),$(DRIVER)support)

CPUS = 000 020 040
LOGGINGS = NO_LOG SER_LOG FILE_LOG MEM_LOG

TOOLS = $(shell find Tools -mindepth 1 -maxdepth 1 -type d)
TOOLS/ALL = $(foreach TOOL,$(TOOLS),$(TOOL)/all)
TOOLS/CLEAN = $(foreach TOOL,$(TOOLS),$(TOOL)/clean)

.PHONY: $(DRIVERS/ALL) $(DRIVERS/SUPPORT) $(DRIVERS/CLEAN) $(DRIVERS/DIST-CLEAN) \
        $(TOOLS/ALL) $(TOOLS/CLEAN) \
		dist-clean clean all

#echo:
#	echo $(DRIVERS/SUPPORT)

release: all
	-umount $(ADF_DIR1)
	rm -rf $(TARGET_DIR) $(TARGET_LHA) $(ADF_DIR1) $(ADF_IMAGE1) $(ADF_DIR2) $(ADF_IMAGE2)
	mkdir -p $(ADF_DIR1) $(ADF_DIR2) $(TOOL_DIR) $(AUDIOMODES_DIR) $(AHIFILE_DIR) $(MHI_DIR) $(FW_DIR)

	cp Installer/drawer.info                    $(TARGET_DIR).info
	cp Installer/Tools.info                     $(TOOL_DIR).info
	cp Installer/AHI4.info                      $(AHI_DIR).info
	cp Installer/MHI.info                       $(MHI_DIR).info
	cp Installer/Firmware.info                  $(FW_DIR).info

	cp Tools/FlashFPGA/FlashGUI                 $(TOOL_DIR)
	cp Tools/FlashFPGA/FlashGUI.info            $(TOOL_DIR)
	cp Tools/Mixer/Mixer                        $(TOOL_DIR)
	cp Tools/Mixer/Mixer.info                   $(TOOL_DIR)
	cp Tools/PlayMIDI/PlayMidi                  $(TOOL_DIR)
	cp Tools/PlayMIDI/PlayMidi.info             $(TOOL_DIR)
	cp Tools/PlayMP3/PlayMP3                    $(TOOL_DIR)
	cp Tools/PlayMP3/PlayMP3.info               $(TOOL_DIR)
	cp Tools/PlayWAV/PlayWav                    $(TOOL_DIR)
	cp Tools/PlayWAV/PlayWav.info               $(TOOL_DIR)
	cp Tools/Record/Record                      $(TOOL_DIR)
	cp Tools/Record/Record.info                 $(TOOL_DIR)
	cp Drivers/AHI4/target/*.audio*             $(AHIFILE_DIR)
	cp Drivers/AHI4/target/AMIGUS               $(AUDIOMODES_DIR)
	cp Drivers/MHI/target/*.library*            $(MHI_DIR)
	# TODO: AHI support tools
	# TODO: MHI support tools

	LATEST_FW=`find ../FPGA/Releases/Amiga/ | sort | tail -n 1` ;\
	BASENAME_FW=`basename $$LATEST_FW`                          ;\
	cp $$LATEST_FW                              $(FW_DIR)       ;\
	cp Installer/fw.info                        $(FW_DIR)/$$BASENAME_FW.info

	cp Drivers/MHI/COPYING                      $(TARGET_DIR)/GPL.txt
	cp Installer/GPL.txt.info                   $(TARGET_DIR)/GPL.txt.info
	cp Drivers/MHI/COPYING.LESSER               $(TARGET_DIR)/LGPL.txt
	cp Installer/LGPL.txt.info                  $(TARGET_DIR)/LGPL.txt.info
	cp Installer/CC-BY-NC-ND.txt                $(TARGET_DIR)/CC-BY-NC-ND.txt
	cp Installer/CC-BY-NC-ND.txt.info           $(TARGET_DIR)/CC-BY-NC-ND.txt.info
	cp Installer/ReadMeFirst.txt                $(TARGET_DIR)/ReadMeFirst.txt
	cp Installer/ReadMeFirst.txt.info           $(TARGET_DIR)/ReadMeFirst.txt.info

	cp Installer/Disk.info                      $(TARGET_DIR)/

	cp "Installer/AmiGUS installer"             $(TARGET_DIR)/
	cp Installer/English.info                   $(TARGET_DIR)/
	cp Installer/Deutsch.info                   $(TARGET_DIR)/

	adf_floppy_create $(ADF_IMAGE1) dd
	adf_format -l AmiGUS-Tools -t 0 -f $(ADF_IMAGE1)
	fuseadf $(ADF_IMAGE1) $(ADF_DIR1)/
	cp -R $(TARGET_DIR)/* $(ADF_DIR1)/
	umount $(ADF_DIR1)/
	-cp $(ADF_IMAGE1) /media/chritoph/HXC/

	mkdir -p $(MANUAL_DIR)
	cp ../Documentation/AmiGUS/AmiGUS.guide     $(MANUAL_DIR)
	cp -R ../Documentation/AmiGUS/ilbm          $(MANUAL_DIR)
	cp Installer/AmiGUS.guide.info              $(MANUAL_DIR)

	adf_floppy_create $(ADF_IMAGE2) dd
	adf_format -l AmiGUS-Manual -t 0 -f $(ADF_IMAGE2)
	fuseadf $(ADF_IMAGE2) $(ADF_DIR2)/
	cp -R $(MANUAL_DIR)/* $(ADF_DIR2)/
	umount $(ADF_DIR2)/
	-cp $(ADF_IMAGE2) /media/chritoph/HXC/

	cd `dirname $(TARGET_DIR)` ; jlha a $(TARGET_LHA) `basename $(TARGET_DIR)` `basename $(TARGET_DIR)`.info

all: tools drivers support

tools: $(TOOLS/ALL)

support: $(DRIVERS/SUPPORT)
	echo $(DRIVERS/SUPPORT)

drivers: $(DRIVERS/ALL) 

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
	cp -R    /tmp/mhi-installation/Include/proto                Drivers/MHI/include
	cp -R    /tmp/mhi-installation/Include/pragmas              Drivers/MHI/include
	cp -R    /tmp/mhi-installation/Include/clib                 Drivers/MHI/include
	touch prepare-mhi

clean: $(DRIVERS/CLEAN) $(TOOLS/CLEAN)
	rm -rf $(ADF_IMAGE1) $(ADF_DIR1) $(ADF_IMAGE2) $(ADF_DIR2) $(TARGET_DIR) $(TARGET_LHA)

dist-clean: $(DRIVERS/DIST-CLEAN) clean
	rm -rf Drivers/*/include
	rm -rf /tmp/mhi-installation prepare-mhi
	rm -rf /tmp/ahi4-installation prepare-ahi

$(DRIVERS/CLEAN) $(DRIVERS/SUPPORT) $(TOOLS/ALL) $(TOOLS/CLEAN):
	echo -C $(@D) $(@F)
	$(MAKE) -C $(@D) $(@F)

$(DRIVERS/ALL): prepare-ahi prepare-mhi
	for CPU in $(CPUS)                                              ;\
	do                                                               \
		for LOGGING in $(LOGGINGS)                                  ;\
		do                                                           \
			$(MAKE) -C $(@D) $(@F) LIB_CPU=$$CPU LIB_LOG=$$LOGGING  ;\
		done                                                        ;\
	done
