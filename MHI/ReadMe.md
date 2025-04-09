Requirements:
-------------

* Point MHIDeveloper: at Include from https://aminet.net/driver/audio/mhi_dev.lha
* make from https://aminet.net/dev/c/make-4.4.1.lha in path
* AmigaOS NDK from https://aminet.net/dev/misc/NDK3.2.lha
* vbcc/bin from http://phoenix.owl.de/vbcc/2022-03-23/vbcc_bin_amigaos68k.lha
* vbcc/target from http://phoenix.owl.de/vbcc/2022-05-22/vbcc_target_m68k-amigaos.lha

for ADF creation:
* git clone https://github.com/adflib/ADFlib.git
** cd ADFlib
** ./autogen.sh
** ./configure.sh
** make
** make install
* sudo apt-get install check
* sudo apt-get install libfuse-dev
* git clone https://gitlab.com/t-m/fuseadf.git
** cd fuseadf
** ./autogen.sh
** ./configure.sh
** make
** make install

Process: 
mkdir adf
# bzip2 -c -d src/empty.adf.bz2 > target/AmiGUS.adf
adf_floppy_create target/AmiGUS.adf dd
adf_format -l AmiGUS -t 0 -f target/AmiGUS.adf
fuseadf target/AmiGUS.adf adf/
cp target/mhiamigus.library adf/
umount adf/
cp target/AmiGUS.adf ............
