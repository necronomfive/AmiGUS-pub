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

release: all
	rm -rf /tmp/AmiGUS/
	mkdir -p /tmp/AmiGUS/
	cp Tools/FlashFPGA/FlashGUI /tmp/AmiGUS/
	cp Tools/FlashFPGA/FlashGUI.info /tmp/AmiGUS/
	cp Tools/Mixer/Mixer /tmp/AmiGUS/
	cp Tools/Mixer/Mixer.info /tmp/AmiGUS/
	cp Tools/PlayMIDI/PlayMidi /tmp/AmiGUS/
	cp Tools/PlayMIDI/PlayMidi.info /tmp/AmiGUS/
	cp Tools/PlayMP3/PlayMP3 /tmp/AmiGUS/
	cp Tools/PlayMP3/PlayMP3.info /tmp/AmiGUS/
	cp Tools/PlayWAV/PlayWav /tmp/AmiGUS/
	cp Tools/PlayWAV/PlayWav.info /tmp/AmiGUS/
	cp Tools/Record/Record /tmp/AmiGUS/
	cp Tools/Record/Record.info /tmp/AmiGUS/

all: tools
# drivers

tools: $(TOOLS/ALL)

drivers: $(DRIVERS/ALL)

clean: $(DRIVERS/CLEAN) $(TOOLS/CLEAN)

dist-clean: $(DRIVERS/DIST-CLEAN)

ECHO:
	echo $(DRIVERS/CLEAN)

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
