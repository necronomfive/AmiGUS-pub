# Welcome to AmiGUS!
<img src=\Media\Pics\Title.png width="1000">

**AmiGUS** is a modern high quality sound card for the Amiga. It was designed to provide the right combination of accelerated hardware and software features to bring the full multimedia experience to all Amiga users. 

By supporting established frameworks such as **AHI** and **MHI**, we ensure that the card is compatible to a wide range of already existing applications. In addition, the card's true power lies in its highly sophisticated wavetable engine, which provides a powerful feature upgrade to PAULA, supporting 32 true hardware voices using 32 MB of sample memory.

The good news is that, in order to use these features, you don't need a fast CPU. **AmiGUS** can be used on **ALL** Amigas with a free Zorro Slot, since all of the computational heavy-lifting is performed by the hardware acclerated functions on the card.

And since we are at it right now, you can own an **AmiGUS** too, either by buying it from one of our **supported resellers**, or you just download the card's design files provided here and **build one on your own!**

### _Let the Amiga sound revolution begin now!_

## AmiGUS Feature List
* **Zorro II** card for **Amiga 2000 / 3000 /4000** computers, or compatible bus boards.
* **Standard RCA** stereo output, **192kHz @24-bit**.
* **TOSLINK** optical output, supporting **96kHz @20-bit**.
* **3x analogue audio inputs** (PAULA, CD-ROM, EXTERNAL), sampled at **192kHz @24-bit**.
* **High-quality digital mixer** which operates at full **192kHz @24-bit** resolution.
* **AHI stream interface**, supporting **8-/16- & 24-bit** modes for playback and recording.
* **MP3/OGG/FLAC/WMA** hardware decoder **(VLSI VS1063)**, enhanced by a powerful DMA stream buffer.
* **32 channel wavetable engine** - featuring <ins>per voice</ins>:
  * **8-/16-bit** sample support, **192kHz @24-bit** mixing rate.
  * Sample start, stop and loop pointer for effective **one-shot or continuous playback modes**.
  * **32-bit** sample phase accumulators for **extra fine grained pitch definition**.
  * Optional **sample interpolation** for smooth sounds.
  * Channel independent **left / right stereo panning**.
  * Hardware **ADSR evelope generator**.
* **32 MB on-board sample memory** - ready to be used for **Multichannel Sample Tracking** or **MIDI**.
* **Flash-based mixer settings** which are automatically loaded on start-up.
* **Upgradable FPGA Core** via **JTAG**, or **Amiga-based flash tool**.
* **Open Hardware** - Gerbers and FPGA Bitstream <ins>freely available</ins>.
  
### _Minimum Amiga Configuration_
* Amiga 2000
* 68000 @7.09MHz
* 1 MB Chip / 4 MB Fast
* OS 1.3
### _Recommended Amiga Configuration_
* Amiga 2000 / 3000 / 4000
* 68030 @25Mhz
* 2 MB Chip / 16 MB Fast
* OS 3.9 or 3.2.2
  
## AmiGUS Parts & Function Overview
<img src=\Media\Pics\PCB_Outline.png width="1000">
The above diagram shows all relevant ports and components of the AmiGUS card.

### _List of Components_
|Part|Function| Documentation|
|-|-|-|
|**Altera 10M08 FPGA**| Main processing unit, receives and generates digital audio streams in 192kHz |[Register Map](https://github.com/necronomfive/AmiGUS/raw/main/Documentation/AmiGUS/AmiGUS_Register_Map.xlsx)|
|**32MB SDRAM**| Memory for sample storage, clocked at 122.2MHz  | [Datasheet](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/42_45S83200J_16160J-462626.pdf)  |
|**VS1063 Decoder**| Decoder chip from VLSI, supports MP3,OGG and FLAC  | [Datasheet](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/vs1063ds.pdf) [HW Guide](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/vs1063hg.pdf) |
|**PCM1812 ADC**| Analogue to digital coverter, captures audtio from all external inputs @192kHz, 24-bit| [Datasheet](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/pcm1862.pdf) |
|**PCM1794 DAC**| Digital to analogue converter, outputs final mixing result @192kHz 24-bit  | [Datasheet](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/pcm1794a.pdf) |
|**ZORRO II Bus**| AmiGUS supports Zorro II, non-DMA I/O accesses.| [Zorro Spec](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/zorro3.pdf) |

### _List of ports_
|Port|Function|
|-|-|
|**TOSLINK**| Optical output to sound system, supports up to 96kHz @20-bit. |
|**Analogue Output**| Stereo output via RCA jacks at 192kHz @24-bit. |
|**JTAG Port**| FPGA programming interface, Altera USB blaster compatible. |
|**Expansion Port**| Digital output of final mix, can be input to other cards. |
|**PAULA Input**| Internal connector for mixing PAULA sound to AmiGUS output.|
|**CDROM Input**| Internal connector for connecting a CDROM drive to the AmiGUS.|
|**EXTERNAL Input**| External line-in jack to mix or capture any sound source.|

## How to Build your AmiGUS
Here you find all materials to build your own personal AmiGUS card. Most soldering parts have been chosen so that you can build the card, even if you have novice soldering skills. If you are unsure, we recommend to either contact a person who is trustworthy and has the required soldering skills, or buy the card from one of our resellers.
### _PCB Data_
| File  | Description |
|-|-|
| x| Gerber and drill files |
|x | BOM for all components and Mouser references (if available) |
|[Download](https://github.com/necronomfive/AmiGUS/blob/main/PCB/Schematics/Z2_Audio_AmiGUS_V13.pdf) | AmiGUS card schematics. |

In terms of PCB manufacturing, both PCBWAY and JLPCB were able to produce functional boards during our prototyping phase. 
It is recommended that you specify either chamfering or beveling for the slot edge connector in order to prevent mechanical damage to the Zorro slots.

### _Slot Bracket Data_
| File  | Description |
|-|-|
|[Download](https://github.com/necronomfive/AmiGUS/blob/main/Documentation/Datasheets/AmiGUS_Bracket.pdf) | AmiGUS slot bracket holes. |

## AmiGUS Software
### _FPGA Core Update_
The Amiga file can be flashed using the AmiGUS Flash & Configuration tool.
| Altera File  | Amiga File | Date|
|-|-|-|
|<a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/FPGA/Releases/Quartus/250131_AmiGUS.pof">Download</a>| x | Jan 30th, 2025|

### _Base Software_
| File  | Description | Version | Date |
|-|-|-|-|
| <a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/Software/FlashFPGA/Release/FlashFPGA_250130.lha">Download</a> | AmiGUS Flash & Configuration Tool |v0.2| Jan 30th, 2025|
| <a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/Software/Mixer/Release/Mixer_250130.lha">Download</a> | AmiGUS Mixer |V0.42| Jan 30th, 2025|
| <a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/Software/PlayWAV/Release/PlayWav_250130.lha">Download</a> | AmiGUS WAV Player (does not use AHI) |V0.45| Jan 30th, 2025|
| <a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/Software/PlayMP3/Release/PlayMP3_250130.lha">Download</a> | AmiGUS MP3 Player (does not use AHI) |V0.32| Jan 30th, 2025|
| <a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/Software/Record/Release/Record_250130.lha">Download</a> | AmiGUS Raw Recording Tool |V0.1| Jan 30th, 2025|
| <a id="raw-url" href="https://github.com/necronomfive/AmiGUS/raw/main/Software/PlayMIDI/Release/PlayMIDI_250130.lha">Download</a> | AmiGUS MIDI Instrument Play |V0.1| Jan 30th, 2025|
### Driver Packages
| File  | Description | Version | Date |
|-|-|-|-|
| x| AHI Driver Package |x| Jan 30th, 2025|
|x | MHI Driver Package |x| Jan 30th, 2025|

## Audio Samples
These demo files have been generated by direct digital stream capture from the AmiGUS, and then downconverted to 44.1kHz MP3.

| File  | Description |
| ------------- | ------------- |
| [Example 1](https://github.com/necronomfive/AmiGUS/raw/main/Media/MP3s/AmiGUS_MOD_NoInterpolation.mp3?raw=true)  | Amiga MOD standard playback on AmiGUS |
| [Example 2](https://github.com/necronomfive/AmiGUS/raw/main/Media/MP3s/AmiGUS_MOD_WithInterpolation.mp3?raw=true)  | Amiga MOD playback using interpolation on AmiGUS |
| [Example 3](https://github.com/necronomfive/AmiGUS/raw/main/Media/MP3s/AmiGUS_S3M_WithInterpolation.mp3?raw=true)  | S3M playback using interpolation on AmiGUS |
| [Example 4](https://github.com/necronomfive/AmiGUS/raw/main/Media/MP3s/AmiGUS_Sample.mp3?raw=true)  | MIDI instrument played using all 32 channels |

## Supported Resellers
## Links
Here you will find links to other projects which support AmiGUS.
| Link  | Description |
| ------------- | ------------- |
| [amigaos-AmiGUS](https://github.com/christoph-fassbach/amigaos-AmiGUS) | AHI driver project for AmiGUS.|
| [HippoPlayer](https://github.com/koobo/HippoPlayer) | Music player which has preliminary AmiGUS support. |

## Licensing
### _AmiGUS Printed Circuit Board_
[![CC BY-NC-ND 4.0][cc-by-nc-nd-shield]][cc-by-nc-nd]\
AmiGUS PCB\
Copyright (C) 2025 by Oliver Achten

This work is licensed under a
[Creative Commons Attribution-NonCommercial-NoDerivs 4.0 International License][cc-by-nc-nd].

[![CC BY-NC-ND 4.0][cc-by-nc-nd-image]][cc-by-nc-nd]

[cc-by-nc-nd]: http://creativecommons.org/licenses/by-nc-nd/4.0/
[cc-by-nc-nd-image]: https://licensebuttons.net/l/by-nc-nd/4.0/88x31.png
[cc-by-nc-nd-shield]: https://img.shields.io/badge/License-CC%20BY--NC--ND%204.0-lightgrey.svg

### _AmiGUS Base Software_

AmiGUS Base Software\
Copyright (C) 2025 by Oliver Achten

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

### _AmiGUS AHI Driver_

AmiGUS AHI Driver\
Copyright (C) 2025 by Christoph Faßbach

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Credits / Thanks
* **Oliver Achten** - product idea, concept, PCB design, FPGA logic, docs, base software
* **Christoph Faßbach** - AHI & MHI driver framework
* **Torsten Hees** - AmiGUS prototype production coordination & support
* **Kari-Pekka Koljonen** - lots of help for HippoPlayer support
