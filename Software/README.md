# This Folder - AmiGUS Software

## License and Legal

For all subfolders:
GPL 3 - unless noted otherwise - see LICENSE.md.

Contributions very welcome!

## Releases:

See [releases](https://github.com/necronomfive/AmiGUS-pub/releases) .

## Usage and installation:

See [documentation](https://github.com/necronomfive/AmiGUS-pub/tree/main/Documentation/AmiGUS).

## Building and packaging:

### Requirements:
* make 4.4.1 from https://aminet.net/dev/c/make-4.4.1.lha
* AmigaOS NDK from https://aminet.net/dev/misc/NDK3.2.lha
* vbcc 0.9h patch 2, newer untested
  * vbcc/bin from http://phoenix.owl.de/vbcc/2022-03-23/vbcc_bin_amigaos68k.lha
  * vbcc/target from http://phoenix.owl.de/vbcc/2022-05-22/vbcc_target_m68k-amigaos.lha
* SASC 6.58
  * SAS/C 6.50 or later
  * Applied patch to 6.55 from https://aminet.net/dev/c/sc655pch.lha
  * Applied patch to 6.56 from https://aminet.net/dev/c/sc656pch.lha
  * Manual patch to 6.56 from https://aminet.net/dev/c/sc656man.lha
  * Applied patch to 6.57 from https://aminet.net/dev/c/sc657pch.lha
  * Applied patch to 6.58 from https://aminet.net/dev/c/sc658pch.lha
  * Applied Y2K patch from https://aminet.net/dev/c/SASC_Y2k_patch.lha
* vamos for building on Linux / MacOS from https://github.com/cnvogelg/amitools - If you like to build all of that natively, please send a Pull Request of your modifications, we love contributions!

### Setup:
* All of the requirements above installed somewhere ...
* ... and `path ... ADD`ed accordingly to make it callable in vamos.

### Building:

* `make` same as `make release` - see below
* `make clean` deletes everything previously built
* `make dist-clean` as above, but additionally deletes all downloaded dependencies
* `make prepare-ahi` downloads and installs all dependencies for building the AmiGUS AHI drivers
* `make prepare-mhi` downloads and installs all dependencies for building the AmiGUS MHI drivers
* `make drivers` combines the two above, and builds all AmiGUS drivers in all variants
* `make prepare-tools` downloads and installs all dependencies for building the AmiGUS base tools
* `make tools` as above, and actually builds the AmiGUS base tools

### Packaging & releasing:

Releasing and packaging is semi-automated as well.
This requires a Linux build machine kind of, with vamos, curl, jq, and jlha installed and the correct credentials for api.github.com and uploads.github.com added to `~/.netrc`.

* `make all-variants` prepares, builds everything, iterating over all CPU and logging variants
* `make package` takes all variants and places them into nice LHA archives and an even nicer ADF files
* `make release` combines the two above
* `make RELEASE_TAG='rc-17' release-publish` builds a release, tags it accordingly and publishes it to [github releases page](https://github.com/necronomfive/AmiGUS-pub/releases) as pre-release. Please move over there to edit the release note and whatever else is needed.
Done.
* `make RELEASE_TAG='rc-17' release-unpublish` removes the tags and the release in case something was odd. No recovery, handle with care!

## Bugs?

Please report any issues found [here](https://github.com/necronomfive/AmiGUS-pub/issues) - happy for any input. :)
Enjoy!
