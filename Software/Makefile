#
# This file is part of the AmiGUS Software Release.
#
# AmiGUS Software Release is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, version 3 of the License only.
#
# AmiGUS Software Release is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with AmiGUS Software Release.
#
# If not, see <http://www.gnu.org/licenses/>.
#

BDIR               = build

TARGET_DIR         = $(BDIR)/AmiGUS
TARGET_DIR13       = $(BDIR)/AmiGUS13
ADF_DIR1           = $(BDIR)/AmiGUS-ADF1
ADF_DIR2           = $(BDIR)/AmiGUS-ADF2
ADF_DIR3           = $(BDIR)/AmiGUS-ADF3

TOOL_DIR           = $(TARGET_DIR)/Tools
AHI_DIR            = $(TARGET_DIR)/AHI4
AHIFILE_DIR        = $(AHI_DIR)/AHI
AUDIOMODES_DIR     = $(AHI_DIR)/AudioModes
MHI_DIR            = $(TARGET_DIR)/MHI
FW_DIR             = $(TARGET_DIR)/Firmware
MANUAL_DIR         = $(TARGET_DIR)/Manual

TARGET_LHA         = $(BDIR)/AmiGUS-`date +%Y%m%d`.lha
TARGET_LHA13       = $(BDIR)/AmiGUS13-`date +%Y%m%d`.lha
ADF_IMAGE1         = $(BDIR)/AmiGUS-Tools-`date +%Y%m%d`.adf
ADF_IMAGE2         = $(BDIR)/AmiGUS-Manual-`date +%Y%m%d`.adf
ADF_IMAGE3         = $(BDIR)/AmiGUS-OS1.3-`date +%Y%m%d`.adf

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

###############################################################################

release: all-variants package

RELEASE_API_URL = https://api.github.com/repos/necronomfive/AmiGUS-pub/releases
UPLOAD_API_URL = https://uploads.github.com/repos/necronomfive/AmiGUS-pub/releases

release-publish: release
	@if [ -z "$(RELEASE_TAG)" ] ; then echo "Please call \"make RELEASE_TAG='rc-17' release-publish\"" ; exit 1 ; fi
	git tag "$(RELEASE_TAG)"
	git push --tags
	curl -L --netrc -X POST                       \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		$(RELEASE_API_URL)                          \
		-d '{ "tag_name":"$(RELEASE_TAG)", "target_commitish":"main", "name":"$(RELEASE_TAG)", "body":"Description of the release", "draft":false, "prerelease":true, "generate_release_notes":false }'
	curl -L --netrc                               \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		$(RELEASE_API_URL)/tags/$(RELEASE_TAG) | jq -r ".id" > $(BDIR)/TEMP_RELEASE_ID
	@echo "\n\n\n$(RELEASE_TAG) found as $$(cat $(BDIR)/TEMP_RELEASE_ID)"
	@echo $(TARGET_LHA)   > $(BDIR)/TEMP_LHA_FILE_NAME
	@echo $(TARGET_LHA13) > $(BDIR)/TEMP_LHA13_FILE_NAME
	@echo $(ADF_IMAGE1)   > $(BDIR)/TEMP_IMAGE1_FILE_NAME
	@echo $(ADF_IMAGE2)   > $(BDIR)/TEMP_IMAGE2_FILE_NAME
	@echo $(ADF_IMAGE3)   > $(BDIR)/TEMP_IMAGE3_FILE_NAME
	@echo "Files to release: \n\t\t\t$$(cat $(BDIR)/TEMP_LHA_FILE_NAME)"
	@echo "\t\t\t$$(cat $(BDIR)/TEMP_LHA13_FILE_NAME)"
	@echo "\t\t\t$$(cat $(BDIR)/TEMP_IMAGE1_FILE_NAME)"
	@echo "\t\t\t$$(cat $(BDIR)/TEMP_IMAGE2_FILE_NAME)"
	@echo "\t\t\t$$(cat $(BDIR)/TEMP_IMAGE3_FILE_NAME)"
	curl -L --netrc                               \
		-X POST                                     \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		-H "Content-Type: application/octet-stream" \
		"$(UPLOAD_API_URL)/$$(cat $(BDIR)/TEMP_RELEASE_ID)/assets?name=$$(basename $$(cat $(BDIR)/TEMP_LHA_FILE_NAME))" \
		--data-binary "@$$(cat $(BDIR)/TEMP_LHA_FILE_NAME)"
	curl -L --netrc                               \
		-X POST                                     \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		-H "Content-Type: application/octet-stream" \
		"$(UPLOAD_API_URL)/$$(cat $(BDIR)/TEMP_RELEASE_ID)/assets?name=$$(basename $$(cat $(BDIR)/TEMP_LHA13_FILE_NAME))" \
		--data-binary "@$$(cat $(BDIR)/TEMP_LHA13_FILE_NAME)"
	curl -L --netrc                               \
		-X POST                                     \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		-H "Content-Type: application/octet-stream" \
		"$(UPLOAD_API_URL)/$$(cat $(BDIR)/TEMP_RELEASE_ID)/assets?name=$$(basename $$(cat $(BDIR)/TEMP_IMAGE1_FILE_NAME))" \
		--data-binary "@$$(cat $(BDIR)/TEMP_IMAGE1_FILE_NAME)"
	curl -L --netrc                               \
		-X POST                                     \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		-H "Content-Type: application/octet-stream" \
		"$(UPLOAD_API_URL)/$$(cat $(BDIR)/TEMP_RELEASE_ID)/assets?name=$$(basename $$(cat $(BDIR)/TEMP_IMAGE2_FILE_NAME))" \
		--data-binary "@$$(cat $(BDIR)/TEMP_IMAGE2_FILE_NAME)"
	curl -L --netrc                               \
		-X POST                                     \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		-H "Content-Type: application/octet-stream" \
		"$(UPLOAD_API_URL)/$$(cat $(BDIR)/TEMP_RELEASE_ID)/assets?name=$$(basename $$(cat $(BDIR)/TEMP_IMAGE3_FILE_NAME))" \
		--data-binary "@$$(cat $(BDIR)/TEMP_IMAGE3_FILE_NAME)"

release-unpublish:
	@if [ -z "$(RELEASE_TAG)" ] ; then echo "Please call \"make RELEASE_TAG='rc-17' release-unpublish\"" ; exit 1 ; fi
	curl -L --netrc                               \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		$(RELEASE_API_URL)/tags/$(RELEASE_TAG) | jq -r ".id" > $(BDIR)/TEMP_RELEASE_ID
	echo "$(RELEASE_TAG) found as $$(cat $(BDIR)/TEMP_RELEASE_ID)"
	curl -L --netrc                               \
		-X DELETE                                   \
		-H "Accept: application/vnd.github+json"    \
		-H "X-GitHub-Api-Version: 2022-11-28"       \
		$(RELEASE_API_URL)/$$(cat $(BDIR)/TEMP_RELEASE_ID)
	git tag -d $(RELEASE_TAG)
	git push origin :$(RELEASE_TAG)

###############################################################################

package:
	-umount $(ADF_DIR1)
	-umount $(ADF_DIR2)
	-umount $(ADF_DIR3)
	rm -rf $(TARGET_DIR) $(TARGET_LHA) \
	       $(TARGET_DIR13) $(TARGET_LHA13) \
	       $(ADF_DIR1) $(ADF_IMAGE1) \
	       $(ADF_DIR2) $(ADF_IMAGE2) \
	       $(ADF_DIR3) $(ADF_IMAGE3)
	mkdir -p $(ADF_DIR1) $(ADF_DIR2) $(ADF_DIR3) \
	         $(TARGET_DIR) $(TARGET_DIR13)

	###########################################################################
	# Tools adf
	###########################################################################

	adf_floppy_create $(ADF_IMAGE1) dd
	adf_format -l AmiGUS-Tools -t 1 -f $(ADF_IMAGE1)
	fuseadf $(ADF_IMAGE1) $(ADF_DIR1)/

	mkdir -p $(ADF_DIR1)/Tools           \
	         $(ADF_DIR1)/AHI4            \
	         $(ADF_DIR1)/AHI4/AHI        \
			 $(ADF_DIR1)/AHI4/AudioModes \
			 $(ADF_DIR1)/MHI             \
			 $(ADF_DIR1)/Firmware 

	cp Installer/Tools.info                     $(ADF_DIR1)/
	cp Installer/AHI4.info                      $(ADF_DIR1)/
	cp Installer/MHI.info                       $(ADF_DIR1)/
	cp Installer/Firmware.info                  $(ADF_DIR1)/

	cp Tools/FlashFPGA/FlashGUI                 $(ADF_DIR1)/Tools
	cp Tools/FlashFPGA/FlashGUI.info            $(ADF_DIR1)/Tools
	cp Tools/Mixer/Mixer                        $(ADF_DIR1)/Tools
	cp Tools/Mixer/Mixer.info                   $(ADF_DIR1)/Tools
	cp Tools/PlayMIDI/PlayMidi                  $(ADF_DIR1)/Tools
	cp Tools/PlayMIDI/PlayMidi.info             $(ADF_DIR1)/Tools
	cp Tools/PlayMP3/PlayMP3                    $(ADF_DIR1)/Tools
	cp Tools/PlayMP3/PlayMP3.info               $(ADF_DIR1)/Tools
	cp Tools/PlayWAV/PlayWav                    $(ADF_DIR1)/Tools
	cp Tools/PlayWAV/PlayWav.info               $(ADF_DIR1)/Tools
	cp Tools/Record/Record                      $(ADF_DIR1)/Tools
	cp Tools/Record/Record.info                 $(ADF_DIR1)/Tools
	cp Drivers/AHI4/target/get_mem_log          $(ADF_DIR1)/AHI4/GetAhiMemLog
	cp Installer/program.info                   $(ADF_DIR1)/AHI4/GetAhiMemLog.info
	cp Drivers/AHI4/target/*.audio*             $(ADF_DIR1)/AHI4/AHI
	cp Drivers/AHI4/target/AMIGUS               $(ADF_DIR1)/AHI4/AudioModes
	cp Drivers/MHI/target/get_mem_log           $(ADF_DIR1)/MHI/GetMhiMemLog
	cp Installer/program.info                   $(ADF_DIR1)/MHI/GetMhiMemLog.info
	cp Drivers/MHI/target/mhi_lib_check         $(ADF_DIR1)/MHI/MHILibCheck
	cp Installer/program.info                   $(ADF_DIR1)/MHI/MHILibCheck.info
	cp Drivers/MHI/target/MHIplay               $(ADF_DIR1)/MHI/MHIplay
	cp Installer/program.info                   $(ADF_DIR1)/MHI/MHIplay.info
	cp Drivers/MHI/target/*.library*            $(ADF_DIR1)/MHI

	cp Drivers/MHI/COPYING                      $(ADF_DIR1)/GPL.txt
	cp Installer/GPL.txt.info                   $(ADF_DIR1)/GPL.txt.info
	cp Drivers/MHI/COPYING.LESSER               $(ADF_DIR1)/LGPL.txt
	cp Installer/LGPL.txt.info                  $(ADF_DIR1)/LGPL.txt.info
	cp Installer/CC-BY-NC-ND.txt                $(ADF_DIR1)/CC-BY-NC-ND.txt
	cp Installer/CC-BY-NC-ND.txt.info           $(ADF_DIR1)/CC-BY-NC-ND.txt.info
	cp Installer/ReadMeFirst.txt                $(ADF_DIR1)/ReadMeFirst.txt
	cp Installer/ReadMeFirst.txt.info           $(ADF_DIR1)/ReadMeFirst.txt.info

	cp Installer/Disk.info                      $(ADF_DIR1)/

	cp "Installer/AmiGUS installer"             $(ADF_DIR1)/
	cp Installer/English.info                   $(ADF_DIR1)/
	cp Installer/Deutsch.info                   $(ADF_DIR1)/

	LATEST_FW=`find ../FPGA/Releases/Amiga/ | grep "\.upd" | sort | tail -n 1` ;\
	BASENAME_FW=`basename $$LATEST_FW`                          ;\
	cp $$LATEST_FW                              $(ADF_DIR1)/Firmware       ;\
	cp Installer/fw.info                        $(ADF_DIR1)/Firmware/$$BASENAME_FW.info

	###########################################################################
	# Manual adf
	###########################################################################

	adf_floppy_create $(ADF_IMAGE2) dd
	adf_format -l AmiGUS-Manual -t 0 -f $(ADF_IMAGE2)
	fuseadf $(ADF_IMAGE2) $(ADF_DIR2)/

	cp Installer/Disk.info                      $(ADF_DIR2)/
	cp ../Documentation/AmiGUS/AmiGUS.guide     $(ADF_DIR2)/AmiGUSv39.guide
	sed -e "s#MultiView#Sys:Utilities/WDisplay#" \
	    -e "s#ilbm/#AmiGUS-Manual:ilbm/#"        \
	    ../Documentation/AmiGUS/AmiGUS.guide  > $(ADF_DIR2)/AmiGUSv38.guide
	cp -R ../Documentation/AmiGUS/ilbm          $(ADF_DIR2)
	cp Installer/AmiGUS.guide                   $(ADF_DIR2)
	cp Installer/AmiGUS.guide.info              $(ADF_DIR2)
	cp Installer/AmiGUS.guide.infoV38           $(ADF_DIR2)

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

	###########################################################################
	# LHA
	###########################################################################

	cp Installer/drawer.info                    $(TARGET_DIR).info
	mkdir -p $(TARGET_DIR)/Manual

	cp -R $(ADF_DIR1)/*                         $(TARGET_DIR)
	cp -R $(ADF_DIR2)/*                         $(TARGET_DIR)/Manual

	rm $(TARGET_DIR)/Disk.info
	rm $(TARGET_DIR)/Manual/Disk.info

	cd $$(dirname $(TARGET_LHA)) ; jlha a $$(basename $(TARGET_LHA)) `basename $(TARGET_DIR)` `basename $(TARGET_DIR)`.info

	###########################################################################
	# LHA 1.3
	###########################################################################

	cp Installer/drawer.info                    $(TARGET_DIR13).info
	mkdir -p $(TARGET_DIR13)/Manual

	cp -R $(ADF_DIR3)/*                         $(TARGET_DIR13)
	cp -R $(ADF_DIR2)/*                         $(TARGET_DIR13)/Manual

	rm $(TARGET_DIR13)/Disk.info
	rm $(TARGET_DIR13)/Manual/Disk.info

	cd $$(dirname $(TARGET_LHA13)) ; jlha a $$(basename $(TARGET_LHA13)) `basename $(TARGET_DIR13)` `basename $(TARGET_DIR13)`.info

	###########################################################################
	# cleanup
	###########################################################################

	umount $(ADF_DIR1)/
	umount $(ADF_DIR2)/
	umount $(ADF_DIR3)/

	-cp $(ADF_IMAGE1) /media/chritoph/HXC/
	-cp $(ADF_IMAGE2) /media/chritoph/HXC/
	-cp $(ADF_IMAGE3) /media/chritoph/HXC/

	###########################################################################
	# DONE
	###########################################################################

###############################################################################

all-variants: tools drivers support

tools: prepare-tools $(TOOLS/ALL)

support: $(DRIVERS/SUPPORT)
	echo $(DRIVERS/SUPPORT)

drivers: $(DRIVERS/ALL) 

prepare-tools:
	mkdir -p /tmp/tools-installation
	wget -P  /tmp/tools-installation https://aminet.net/util/libs/ReqToolsDev.lha
	cd       /tmp/tools-installation ; jlha x ReqToolsDev.lha
	wget -P  /tmp/tools-installation https://aminet.net/mus/play/hippoplayer.lha
	cd       /tmp/tools-installation ; jlha x hippoplayer.lha
	mkdir -p dependencies/libs
	cp -R    /tmp/tools-installation/ReqTools/include                   dependencies/
	cp -R    /tmp/tools-installation/HippoSupport/Libs/reqtools.library dependencies/libs/
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
