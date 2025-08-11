## License

LGPL 3 - see COPYING + COPYING.LESSER.

Contributions very welcome!

## Usage 
### Requirements:
* Software
  * AmigaOS 1.3 or higher
  * AmiGUS board - :D - yeah, really, does not make sense without it
  * 68000 7 MHz or higher
  * May need some FastRAM - I never tested it without, if you do, let me know the result please :-)
  * HardDisk - >10MB. While you may be able to squeeze Workbench, HippoPlayer or MHIplay and the library to 880kB floppies, encoded media (MP3s, FLACs, ...) is huge!
### Installation:
Some easy manual steps for the time being, an installer for everything AmiGUS is in the main repositoy.
* Download a binary archive from the releases section.
* Transfer the archive to your Amiga.
* Extract the archive somewhere somehow. If you have never seen an lha file before please find yourself a manual about it.
* Copy `mhiamigus.library` to `Libs:MHI` (AmigaOS 2.0 and later) or `Libs:` (AmigaOS 1.3)
* Open the settings of an application with MHI support and select the `mhiamigus.library` from your installation path.
### Tested Sofware:
* HippoPlayer
  * 2.45 from https://aminet.net/mus/play/hippoplayer.lha
  * upgraded to 2.62+ (recommended - ! - has AmiGUS wavetable support as well) from https://aminet.net/mus/play/hippoplayerupdate.lha
* AmigaAMP
  * 3.33 from from https://aminet.net/mus/play/AmigaAMP3-68k.lha

## Building
### Requirements:
* MHI DevKit, from https://aminet.net/driver/audio/mhi_dev.lha
* make 4.4.1 from https://aminet.net/dev/c/make-4.4.1.lha
* AmigaOS NDK from https://aminet.net/dev/misc/NDK3.2.lha
* vbcc 0.9h patch 2, newer untested
  * vbcc/bin from http://phoenix.owl.de/vbcc/2022-03-23/vbcc_bin_amigaos68k.lha
  * vbcc/target from http://phoenix.owl.de/vbcc/2022-05-22/vbcc_target_m68k-amigaos.lha
* OPTIONAL: SASC 6.58
  * SAS/C 6.50 or later
  * Applied patch to 6.55 from https://aminet.net/dev/c/sc655pch.lha
  * Applied patch to 6.56 from https://aminet.net/dev/c/sc656pch.lha
  * Manual patch to 6.56 from https://aminet.net/dev/c/sc656man.lha
  * Applied patch to 6.57 from https://aminet.net/dev/c/sc657pch.lha
  * Applied patch to 6.58 from https://aminet.net/dev/c/sc658pch.lha
  * Applied Y2K patch from https://aminet.net/dev/c/SASC_Y2k_patch.lha
* OPTIONAL: vamos for building on Linux / MacOS from https://github.com/cnvogelg/amitools

### Setup:
* All of the requirements above installed somewhere ...
* ... and `path ... ADD`ed accordingly to make it callable ...
* ... and finally set `MHIDevelop:` to the `Include` folder from the extracted MHI DevKit downloaded as per above ...
* ... and run `make INCLUDES`.

### Steps:

There is some dark magic in the Makefile to enable putting random .asm or .c files into the src folder to add them to the binary.
Beware of it!

* `make INCLUDES` prepares the environment
* `make` builds MHI driver
* `make all` same as above
* `make install` same as above, and tries installing it to some local machine folders as well as some network paths. Feel free to adapt, but no Pull Requests breaking it on my machine accepted. :-P Sorry.
* `make clean` cleans build and target dir as well as support file artefacts
* `make clean-intermediate` cleans the build dir only
* `make dist-clean` prepares the source folder for distribution, drops includes as well.
* `make ECHO` helps debugging make's internal state
* `make test` to build / run the automated tests
* `make support` to build the support applications

Additional switches: 
* `make USEVBCC=1` switches from SAS/C to VBCC
* `make LIB_CPU=000` switches to creating 68000 code
  * `.......=020` switches to creating 68020 code
  * `.......=030` switches to creating 68030 code
  * `.......=040` switches to creating 68040 code
  * `.......=060` switches to creating 68060 code (I guess that works with VBCC only, if at all)
* `make LIB_LOG=NO_LOG` turns log generation completely off, release mode
  * `.......=SER_LOG` debug logging, via serial port / [Sashimi](https://aminet.net/dev/debug/Sashimi.lha) compatible, 
  * `.......=FILE_LOG` verbose logging, as file somewhere, defaults to RAM:
  * `.......=MEM_LOG` including interrupt logging, somewhere into RAM at some address. Use the Get[Mhi]MemLog tool to find it.

### Packaging

Install disk creation and packaging rely on the scripts in AmiGUS main repository, [here](https://github.com/necronomfive/AmiGUS-pub/tree/main/Software).