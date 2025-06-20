Please note the following licenses:
-----------------------------------

AHI and MHI drivers are published under GNU Lesser General Public License.
See LGPL.txt and GPL.txt for details.

The AmiGUS base tools (FlashGUI, Mixer, PlayMidi, PlayMP3, PlayWav, Record)
are published under GNU General Public License.
See GPL.txt for details.

The AmiGUS firmware and hardware design files are published under 
Creative Commons Attribution-NonCommercial-NoDerivs 4.0
International License.
See CC-BY-NC-ND.txt for details.

All project sources are available at:
https://github.com/necronomfive/AmiGUS-pub/

Please find the latest releases at:
https://github.com/necronomfive/AmiGUS-pub/releases

-----------------------------------------------------------------------------

Distribution:
-------------

What is allowed? 
- Read the licenses - that is what they are meant for. ;-)

What is included?
- The distribution consists of 4 files so far:
  - AmiGUS-Tools-<YYYYmmDD>.adf
    Contains the AmiGUS Base Tools and all Drivers meant for OS2.0 and above,
    including an Installer script. Very likely, you want to use this. ;-)
  - AmiGUS-Manual-<YYYYmmDD>.adf
    Contains the AmiGUS Manual in AmigaGuide format, for direct use or to be
    installed using the Installer script from the tools disk.
    Works on OS2.1 and above, but looks best in OS3.2 and above.
  - AmiGUS-OS13-<YYYYmmDD>.adf
    This is the ONLY disk readable to OS1.3 machines - and it is meant for
    them. It contains special versions of the most important base tools and
    the MHI driver to enable using the AmiGUS even there.
    Comes with a Shell install script.
    DO -=NOT=- USE ON NEWER OS, PLEASE!!!
  - AmiGUS-<YYYYmmDD>.lha
    Contains all of the above - but only the installers for OS2.0 or above.

How do I install the hardware or software?
- For installation instructions, please check the manual,
  paper, pdf, docx, or AmigaGuide.
  Worst case, check out https://github.com/necronomfive/AmiGUS-pub/ ,
  please.

-----------------------------------------------------------------------------

Dependencies:
-------------

- In case you experience issues with a missing Installer or the installation
  does not work as expected on the Installer version in your OS2.0 or above,
  please try the latest Commodore / ESCOM / Amiga Technologies Installer from
  
  https://aminet.net/util/misc/Installer-43_3.lha

  Installer 43.3
  Copyright © 1995-96 Escom AG.

- For file requesters, AmiGUS base tools rely on 

  reqtools.library 38.390
  Copyright © 1991-1994 Nico François

- To view the AmigaGuide manual on AmigaOSs earlier 3.0, 
  you will need to install "AmigaGuide Development" version from

  https://aminet.net/text/hyper/aguide34.lha
  
  AmigaGuide_Dev 34.3
  Copyright © 1992-1993 Commodore-Amiga, Inc.

- For using AHI, you need 

  https://aminet.net/driver/audio/ahiusr_4.18.lha 

  (RECOMMENDED)
  AHI 4.18
  Copyright © 1994-1998 Martin Blom

  - OR -

  https://aminet.net/driver/audio/m68k-amigaos-ahiusr.lha

  AHI 6.0
  Copyright © 1994-2005 Martin Blom

  installed.

-----------------------------------------------------------------------------

Enjoy!
