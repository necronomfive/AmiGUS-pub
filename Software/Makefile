TARGET_DIR         = /tmp/AmiGUS
ADF_DIR1           = /tmp/AmiGUS-ADF1
ADF_DIR2           = /tmp/AmiGUS-ADF2
ADF_DIR3           = /tmp/AmiGUS-ADF3
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
ADF_IMAGE3         = /tmp/AmiGUS-OS1.3-`date +%Y%m%d`.adf

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
	-umount $(ADF_DIR2)
	-umount $(ADF_DIR3)
	rm -rf $(TARGET_DIR) $(TARGET_LHA) \
	       $(ADF_DIR1) $(ADF_IMAGE1) \
	       $(ADF_DIR2) $(ADF_IMAGE2) \
	       $(ADF_DIR3) $(ADF_IMAGE3)
	mkdir -p $(ADF_DIR1) $(ADF_DIR2) $(ADF_DIR3) \
	         $(TOOL_DIR) \
			 $(AUDIOMODES_DIR) $(AHIFILE_DIR) \
			 $(MHI_DIR) \
			 $(FW_DIR)

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
	cp Drivers/AHI4/target/get_mem_log          $(AHI_DIR)/GetAhiMemLog
	cp Installer/program.info                   $(AHI_DIR)/GetAhiMemLog.info
	cp Drivers/AHI4/target/*.audio*             $(AHIFILE_DIR)
	cp Drivers/AHI4/target/AMIGUS               $(AUDIOMODES_DIR)
	cp Drivers/MHI/target/get_mem_log           $(MHI_DIR)/GetMhiMemLog
	cp Installer/program.info                   $(MHI_DIR)/GetMhiMemLog.info
	cp Drivers/MHI/target/mhi_lib_check         $(MHI_DIR)/MHILibCheck
	cp Installer/program.info                   $(MHI_DIR)/MHILibCheck.info
	cp Drivers/MHI/target/MHIplay               $(MHI_DIR)/MHIplay
	cp Installer/program.info                   $(MHI_DIR)/MHIplay.info
	cp Drivers/MHI/target/*.library*            $(MHI_DIR)

	LATEST_FW=`find ../FPGA/Releases/Amiga/ | grep "\.upd" | sort | tail -n 1` ;\
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

	###########################################################################
	# Tools adf
	###########################################################################

	adf_floppy_create $(ADF_IMAGE1) dd
	adf_format -l AmiGUS-Tools -t 1 -f $(ADF_IMAGE1)
	fuseadf $(ADF_IMAGE1) $(ADF_DIR1)/
	cp -R $(TARGET_DIR)/* $(ADF_DIR1)/
	umount $(ADF_DIR1)/
	-cp $(ADF_IMAGE1) /media/chritoph/HXC/

	mkdir -p $(MANUAL_DIR)
	cp Installer/Disk.info                      $(MANUAL_DIR)/
	cp ../Documentation/AmiGUS/AmiGUS.guide     $(MANUAL_DIR)
	cp -R ../Documentation/AmiGUS/ilbm          $(MANUAL_DIR)
	cp Installer/AmiGUS.guide.info              $(MANUAL_DIR)

	###########################################################################
	# Manual adf
	###########################################################################

	adf_floppy_create $(ADF_IMAGE2) dd
	adf_format -l AmiGUS-Manual -t 1 -f $(ADF_IMAGE2)
	fuseadf $(ADF_IMAGE2) $(ADF_DIR2)/
	cp -R $(MANUAL_DIR)/* $(ADF_DIR2)/
	umount $(ADF_DIR2)/
	-cp $(ADF_IMAGE2) /media/chritoph/HXC/

	###########################################################################
	# LHA
	###########################################################################

	cp Tools/Mixer/Mixer13                      $(TOOL_DIR)
	cp Tools/Mixer/Mixer13.info                 $(TOOL_DIR)
	cp Tools/FlashFPGA/FlashGUI13               $(TOOL_DIR)
	cp Tools/FlashFPGA/FlashGUI13.info          $(TOOL_DIR)
	cp -R dependencies/libs                     $(TARGET_DIR)/Libs

	cd `dirname $(TARGET_DIR)` ; jlha a $(TARGET_LHA) `basename $(TARGET_DIR)` `basename $(TARGET_DIR)`.info

	###########################################################################
	# OS1.3 adf
	###########################################################################

	adf_floppy_create $(ADF_IMAGE3) dd
	adf_format -l AmiGUS-OS1.3 -t 0 -f $(ADF_IMAGE3)
	fuseadf $(ADF_IMAGE3) $(ADF_DIR3)/

	mkdir -p $(ADF_DIR3)/Tools $(ADF_DIR3)/Firmware $(ADF_DIR3)/MHI

	cp Installer/13/Tools.info                  $(ADF_DIR3)/Tools.info
	cp Installer/13/MHI.info                    $(ADF_DIR3)/MHI.info
	cp Installer/13/Firmware.info               $(ADF_DIR3)/Firmware.info
	cp Installer/13/Libs.info                   $(ADF_DIR3)/Libs.info

	cp Tools/Mixer/Mixer13                      $(ADF_DIR3)/Tools
	cp Tools/Mixer/Mixer13.info                 $(ADF_DIR3)/Tools
	cp Tools/FlashFPGA/FlashGUI13               $(ADF_DIR3)/Tools
	cp Tools/FlashFPGA/FlashGUI13.info          $(ADF_DIR3)/Tools
	cp Tools/PlayMIDI/PlayMidi                  $(ADF_DIR3)/Tools
	cp Tools/PlayMIDI/PlayMidi.info             $(ADF_DIR3)/Tools
	cp Tools/PlayMP3/PlayMP3                    $(ADF_DIR3)/Tools
	cp Tools/PlayMP3/PlayMP3.info               $(ADF_DIR3)/Tools
	cp Tools/PlayWAV/PlayWav                    $(ADF_DIR3)/Tools
	cp Tools/PlayWAV/PlayWav.info               $(ADF_DIR3)/Tools
	cp Tools/Record/Record                      $(ADF_DIR3)/Tools
	cp Tools/Record/Record.info                 $(ADF_DIR3)/Tools
	cp -R dependencies/libs                     $(ADF_DIR3)/Libs
	cp Drivers/MHI/target/get_mem_log           $(ADF_DIR3)/MHI/GetMhiMemLog
	cp Installer/program.info                   $(ADF_DIR3)/MHI/GetMhiMemLog.info
	cp Drivers/MHI/target/mhi_lib_check         $(ADF_DIR3)/MHI/MHILibCheck
	cp Installer/program.info                   $(ADF_DIR3)/MHI/MHILibCheck.info
	cp Drivers/MHI/target/MHIplay               $(ADF_DIR3)/MHI/MHIplay
	cp Installer/program.info                   $(ADF_DIR3)/MHI/MHIplay.info
	cp Drivers/MHI/target/*.library*000*        $(ADF_DIR3)/MHI/
	cp Drivers/MHI/target/*.library*020*        $(ADF_DIR3)/MHI/

	cp Drivers/MHI/COPYING                      $(ADF_DIR3)/GPL.txt
	cp Installer/13/GPL.txt.info                $(ADF_DIR3)/GPL.txt.info
	cp Drivers/MHI/COPYING.LESSER               $(ADF_DIR3)/LGPL.txt
	cp Installer/13/LGPL.txt.info               $(ADF_DIR3)/LGPL.txt.info
	cp Installer/CC-BY-NC-ND.txt                $(ADF_DIR3)/CC-BY-NC-ND.txt
	cp Installer/13/CC-BY-NC-ND.txt.info        $(ADF_DIR3)/CC-BY-NC-ND.txt.info
	cp Installer/ReadMeFirst.txt                $(ADF_DIR3)/ReadMeFirst.txt
	cp Installer/13/ReadMeFirst.txt.info        $(ADF_DIR3)/ReadMeFirst.txt.info

	cp Installer/Disk.info                      $(ADF_DIR3)/

	cp Installer/13/English                     $(ADF_DIR3)/
	cp Installer/13/English.info                $(ADF_DIR3)/
	cp Installer/13/Deutsch                     $(ADF_DIR3)/
	cp Installer/13/Deutsch.info                $(ADF_DIR3)/

	LATEST_FW=`find ../FPGA/Releases/Amiga/ | grep "\.upd" | sort | tail -n 1` ;\
	BASENAME_FW=`basename $$LATEST_FW`                          ;\
	cp $$LATEST_FW                              $(ADF_DIR3)/Firmware       ;\
	cp Installer/fw.info                        $(ADF_DIR3)/Firmware/$$BASENAME_FW.info

	umount $(ADF_DIR3)/
	-cp $(ADF_IMAGE3) /media/chritoph/HXC/

all: tools drivers support

tools: prepare-tools $(TOOLS/ALL)

support: $(DRIVERS/SUPPORT)
	echo $(DRIVERS/SUPPORT)

drivers: $(DRIVERS/ALL) 

prepare-tools:
	mkdir -p /tmp/tools-installation
	wget -P  /tmp/tools-installation https://aminet.net/dev/misc/gt34_1.lha
	cd       /tmp/tools-installation ; jlha x gt34_1.lha
	wget -P  /tmp/tools-installation https://aminet.net/util/libs/ReqToolsDev.lha
	cd       /tmp/tools-installation ; jlha x ReqToolsDev.lha
	# wget -P  /tmp/tools-installation https://aminet.net/util/libs/ReqToolsUsr.lha
	# cd       /tmp/tools-installation ; jlha x ReqToolsUsr.lha
	wget -P  /tmp/tools-installation https://aminet.net/mus/play/hippoplayer.lha
	cd       /tmp/tools-installation ; jlha x hippoplayer.lha
	mkdir -p dependencies/libs
	cp -R    /tmp/tools-installation/ReqTools/include                   dependencies/
	#cp -R    /tmp/tools-installation/ReqTools/libs                      dependencies/
	cp -R    /tmp/tools-installation/HippoSupport/Libs/reqtools.library dependencies/libs/
	cp -R    /tmp/tools-installation/GadTools34/gadtools_34.library     dependencies/libs/gadtools.library
	touch prepare-tools

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
	rm -rf /tmp/gt34-installation prepare-tools

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
