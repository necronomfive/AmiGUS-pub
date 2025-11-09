# AmiGUS Assembly Guide

## Why All This?

* This is a collaborative project, so please join in ;)
* I want to explain the build process clearly
* I want to keep support efforts manageable - we all have day jobs, this is a hobby project, and neither I nor Oli can rescue every misassembled card
* Component costs are significant, so it's better to get it right the first time

## Step 1: Order PCBs

Use these Gerber files for ordering:
[/PCB/Gerbers/Version_13](https://github.com/necronomfive/AmiGUS-pub/tree/main/PCB/Gerbers/Version_13) and
[/PCB/Gerbers/Audiot](https://github.com/necronomfive/AmiGUS-pub/tree/main/PCB/Gerbers/Audiot)

Not intended as advertising, but here are my JLCPCB settings...

![settings JLCPCB 1](assets/images/AmiGUS-JLPCB-1.png)

![settings JLCPCB 2](assets/images/AmiGUS-JLPCB-2.png)

Could you do it differently?
Sure, but this method worked perfectly for me.

Timeline from my experience:

* PCBs ordered: September 9th
* AUDIOTs initially blocked (design rule violations)
* PCBs completed: September 19th
* Cheapest shipping option (FedEx via Paris) initiated same day, estimated 7-10 days
* Arrival: September 24th
* PCB fully assembled: September 27th
* September 29th: Discovered programming issues (faulty programmer cable connector)
* Testing completed: September 30th

## Step 2: Order Components

Example shopping carts for Reichelt and Mouser can be found here:

[/PCB/BOM/AmiGUS_Rev13-example](https://github.com/necronomfive/AmiGUS-pub/tree/main/PCB/BOM/AmiGUS_Rev13-example)

Alinea offers the slot bracket separately for a very reasonable €10 - [here](https://www.amiga-shop.net/Amiga-Ha.../Slotblende-fuer-AmiGUS-Soundkarte::1437.html)

If you're buying the bracket from Alinea, remove the Keystone bracket from your Mouser cart. If you're making your own... grab your Dremel and drill, and good luck!

## Step 3: The Assembly

Fresh from the manufacturer, your PCBs will look like this:

![AmiGUS PCB](assets/images/AmiGUS-PCBs.jpg)

The AUDIOT requires some preparation work:

![AUDIOT PCB](assets/images/Audiot-PCB.jpg)

You'll need to break off the upper section.
I secured the PCB in a vise like this:

![AUDIOT prep 1](assets/images/Audiot-Sanding.jpg)

Note: This version wasn't optimal - I've since refined the process. If you encounter issues, please let me know.

For best results, clamp it flush (using the card as a guide) and apply pressure. Then file the edge:

![AUDIOT prep 2](assets/images/Audiot-Sanding-II.jpg)

Clean up the edges:

![AUDIOT prep 3](assets/images/Audiot-Sanding-III.jpg)

Work on both sides:

![AUDIOT prep 4](assets/images/Audiot-Sanding-IV.jpg)

Final result:

![AUDIOT prep 5](assets/images/Audiot-Front.jpg)
![AUDIOT prep 6](assets/images/Audiot-Back.jpg)

General Notes

During assembly, use the [interactive BOM](https://htmlpreview.github.io/?https://github.com/necronomfive/AmiGUS-pub/blob/main/PCB/BOM/interactiveBOM.html) to track your progress.

My workbench setup includes:

![equipment](assets/images/Solder-Workshop.jpg)

* ELV LF-8800 soldering station with 1.6mm chisel tip, matching desoldering iron and soldering tweezers
* Stereo microscope
* Angled tweezers
* Felder 0.5mm, 3.5% FLUX Sn60PB40 solder
* Flux (amtech from eBay - extremely sticky, browns quickly when activated, spreads rapidly. Gets everywhere, which is annoying. Does the job well, possibly better than Reichelt's, but I'm undecided about buying it again)
* Desoldering braid for corrections
* Toothbrush, IPA, and cotton swabs for cleaning
* Kapton tape (Amazon's budget version works fine) for protecting sensitive areas
* Ultra-bright lamp (200W LED equivalent) - so bright it confuses my phone's white balance (hence the yellowish photos). Warning: daylight looks oddly dim after using this! :D
* Time - lots of it. Budget around 12 hours, though I might just be slow. :rolleyes:
* Experience shows that rushing leads to mistakes
* My approach: avoid mistakes rather than fixing them. Your mileage may vary.
	* AUDIOT preparation (5 units): 30 minutes
	* Session 1 (5 hours): ICs and electrolytic capacitors
	* Session 2 (3 hours): Small components
	* Session 3 (3.5 hours): Remaining small components, electrolytics, voltage regulators, connectors, headers
* Temperature settings for leaded solder:
	* For ground planes/heavy copper: 390°C
	* Alternative: Consider preheating PCB to ~100°C
	* Tweezers: 370°C
	* Desoldering iron (mainly for headers): 390°C (they're robust!)

## Step 3: Soldering the AmiGUS

### 1. All ICs except U16

I worked right to left, starting with:

The operational amplifiers (OPAs):

![Soldering 1](assets/images/OPA.jpg)

Skip U16 (DAC) for now, proceed to U13 (ADC):

![Soldering 2](assets/images/U13.jpg)

Next, U10 (codec):

![Soldering 3](assets/images/U10.jpg)

Install U11 (RAM) before U24 due to tight spacing and U11's finer pitch:

![Soldering 4](assets/images/U11.jpg)

Then U24 (logic gate):

![Soldering 5](assets/images/U24.jpg)

U14 follows naturally:

![Soldering 6](assets/images/U14.jpg)

Notice the Kapton tape here - protect those gold surfaces or you'll regret it. No tape = no sympathy! :P

Next comes U7 (FPGA):

![Soldering 7](assets/images/FPGA.jpg)

Install bus drivers right to left:

![Soldering 8](assets/images/Bus-Driver.jpg)

Clean thoroughly at this stage.

### 2. C37, C39, C44, C42

Before:

![Soldering 9](assets/images/before.jpg)

After:

![Soldering 10](assets/images/after.jpg)

### 3. U16

Technique: Tack one pin, apply flux, then...

![Soldering 11](assets/images/U16.jpg)

...complete the remaining pins:

![Solderingc12](assets/images/rest.jpg)

Now you can see why we installed the electrolytics first - protecting the ADC in this tight space is crucial.

### 4. C40, C111, C112

Space is tight here - electrolytics on one side, small components on the other:

![Soldering 13](assets/images/C40.jpg)

### 5. C97

Similar situation, with small components on the right:

![Soldering 14](assets/images/C97.jpg)

### 6. Crystal

Interesting component! First, tin the pads...

![Soldering 15](assets/images/crystal.jpg)

...position with desoldering tweezers...

![Soldering 16](assets/images/crystal-II.jpg)

...align carefully...

![Soldering 17](assets/images/crystal-III.jpg)

...done!

![Soldering 18](assets/images/crystal-IV.jpg)

I use this method because the pads are quite small. The tweezers let me heat all four points simultaneously for perfect placement.

### 7. Small Components

The tedious but essential part... My method: Tack one pad per position, place components, apply flux, then properly solder both sides under the microscope (blank side first, then the tacked side).

75x 100nF capacitors, mid-process:

![Soldering 19](assets/images/birdseeds.jpg)

After cleaning:

![Soldering 20](assets/images/cleaned.jpg)

Work through the interactive BOM systematically, checking off each component. Taking time here prevents errors.

![Soldering 21](assets/images/birdseeds-II.jpg)

Large ground planes can be challenging even at 390°C with the 1.6mm tip. Persistence required...

![Soldering 22](assets/images/birdseeds-III.jpg)

Clean frequently - flux residue spreads everywhere. Small components are particularly troublesome; even minimal flux application leaves residue.

### 8. U3 and U23

These SMD voltage regulators could have been installed earlier with the ICs.
Warning: Large copper areas require higher temperatures and possibly larger tips.

![Soldering 23](assets/images/U3.jpg)
![Soldering 24](assets/images/U3-II.jpg)
![Soldering 25](assets/images/U3-III.jpg)

### 9. Remaining Electrolytic Capacitors

![Soldering 26](assets/images/capacitors.jpg)

This illustrates my concerns about SMD electrolytics. Even with the smallest suitable polymer capacitors, soldering tip access is tight.
The large copper areas increase the risk of cold joints.

![Soldering 27](assets/images/capacitors-II.jpg)

Other locations are easier, though still with substantial copper areas. C98 and C99 (top right) are particularly close - through-hole would have been easier here.

![Soldering 28](assets/images/capacitors-III.jpg)

## 10. Through-hole Voltage Regulators U20, U21, U22

For perfectionist satisfaction, bend leads uniformly (trickier than it sounds due to manufacturer variations).

Pro tip: Mount with nylon screws first - never use metal screws as traces run underneath. Then solder the tab.

Important: If using a Reichelt 7909, verify you received the correct part. We've seen about 30% wrong regulators (7806/+6V instead). While no cards have been damaged yet, it's still problematic.

![Soldering 29](assets/images/U20.jpg)

High heat needed here. Shortened leads help. I use the desoldering iron for better heat transfer.

### 11. All Connectors

![Soldering 30](assets/images/jacks.jpg)

### 12. Pin Headers

Same approach: High heat, desoldering iron.

![Soldering 31](assets/images/pinheader.jpg)

AUDIOT follows the same process.

![Soldering 32](assets/images/pinheader-II.jpg)

Pin 1 at top right (notch up).
Be careful not to dislodge pins.

![Soldering 33](assets/images/pinheader-III.jpg)

Electrical assembly complete!

![Soldering 34](assets/images/ready.jpg)
![Soldering 35](assets/images/ready-II.jpg)

### 13. Slot Bracket Preparation

I opted for Alinea's pre-made bracket. DIY Dremel instructions welcome if anyone wants to contribute.

### 14. Mount Slot Bracket

Self-explanatory - you've got this!

## Step 4: Initial Setup

### 1. Testing

Protecting vintage hardware is crucial.
Let's check for short circuits.

![Checking 1](assets/images/regulators.png)

Linear regulators (top right of AmiGUS).

Check these points for unwanted connections:

1. GND to +12V
2. GND to -12V
3. GND to +5VA
4. GND to -5VA
5. GND to -9VA
6. +12V to -12V
7. +12V to +5VA
8. +12V to -5VA
9. +12V to -9VA
10. -12V to +5VA
11. -12V to -5VA
12. -12V to -9VA
13. +5VA to -5VA
14. +5VA to -9VA
15. -5VA to -9VA

Zorro connector: Small arrow below C94 marks Pin 1 (rear), Pin 2 (front).
Even pins ascending leftward (2,4,6,8,10...).

Key pins:
* GND: 2, 4, 100, 90, 88
* +5V: Pin 6
* +12V: Pin 10
* -12V: Pin 20

Expected readings:
* +5V to GND: ~2MΩ
* +12V to GND: ~50MΩ
* -12V to GND: ~60MΩ

Verify +/-12V at regulators, GND continuity,
+5V reaches TLV1117-33/TLV1117-18 (right pin).

![Checking 2](assets/images/regulators-II.png)

Additional checks:
Verify TLV1117 outputs against GND (left to center pin).
(@TurricanA1200's tip: Tiny solder bridges here can cause problems!)

Expected readings:
U3 - Pin 1 (GND) to:
- Pin 2 (3.3V): 400-460Ω
- Pin 3 (5V): ~50kΩ
U23 - Pin 1 (GND) to:
- Pin 2 (1.8V): ~360kΩ
- Pin 3 (5V): ~50kΩ

Proceed carefully!

### 2. Installation

Card can now be installed. Working power supply needed for programming.
Other functions optional at this stage.

Safe to insert - AmiGUS bus drivers are high-impedance, allowing normal system boot.

### 3. Programming

Altera Quartus software required. @botfixer tested current Quartus Prime Lite; I use 15.1 on Windows.

Initial screen:

![Firmware 1](assets/images/quartus.png)

Process:
1. File -> Open: Select XyZ_AmiGUS_FPGA.pof from [https://github.com/necronomfive/AmiGUS-pub/tree/main/FPGA/Releases/Quartus]()
2. Set Program/Configure options
3. Click Start

![Firmware 2](assets/images/quartus-II.png)

Note: Quartus ignores Verify settings.
Warning: A "100% Successful" message doesn't guarantee proper programming (I got this even with a failed cable).

![Firmware 3](assets/images/quartus-III.png)

Watch for system freeze (floppy drive stops clicking).
Solution: Cold restart ("Have you tried turning it off and on again?")

Hold both mouse buttons for Expansion Board Diagnostics...

![Board ID 1](assets/images/boot.jpg)

AmiGUS shows as devices 5, 6, 7:

![Board ID 2](assets/images/showconfig.png)

### 4. Software Setup

Download the latest [release](https://github.com/necronomfive/AmiGUS-pub/releases).
Note: Some users report LHA archive issues. Works with lha 2.15.
ADFs available as alternative.
For detailed installation options: [RTFM](https://github.com/necronomfive/AmiGUS-pub/blob/main/Documentation/AmiGUS/AmiGUS_User_Manual.pdf)

Start FlashFPGA, click "Info"...

![Firmware 4](assets/images/flash.png)

Expected warning - click "Init"...

![Firmware 5](assets/images/flash-II.png)

Blue progress bar appears. Non-functioning "Quit" button indicates issues.
Quick reset readies the card.

Testing with mixer:

![Test 1](assets/images/mixer.png)

"Levels" shows DAC/TOSLINK output.
Verify Paula connection: Check audio and level indicators
Same for LineIn.

Test with [SmartPlay](https://aminet.net/mus/play/SmartPlay.lha) (Paula MOD player).

Final setup:
1. Install [LHA](https://aminet.net/util/arc/lha.run)
2. Install [AHI 4.18](https://aminet.net/driver/audio/ahiusr_4.18.lha) (BGUI/[MUI](https://aminet.net/util/libs/mui38usr.lha))
3. Install [HippoPlayer](https://aminet.net/mus/play/hippoplayer.lha) + [update](https://aminet.net/mus/play/hippoplayerupdate.lha)
4. Prepare MOD/MP3 files

HippoPlayer usage:
"Add" button loads music.

Settings in Prefs ("Pr"):
MHI tab:

![Test 2](assets/images/prefs.png)

AmiGUS tab:

![Test 3](assets/images/prefs-II.png)

AHI tab:

![Test 4](assets/images/prefs-III.png)

Configuration rules:
* AmiGUS active = AHI/Paula inactive
* MHI on = mpega.library off

Usage guide:
* AHI testing: Disable native AmiGUS (2nd last tab), enable AHI (last tab), save/use
* MHI testing: Enable MHI (3rd last tab), save/use
* Native AmiGUS: Enable AmiGUS support (2nd last tab), AHI state irrelevant
* Paula audio: Both AHI and AmiGUS off

Operating modes:
MHI playback:

![Test 5](assets/images/MHI.png)

AGUS output:

![Test 6](assets/images/AGUS.png)

AHI output:

![Test 7](assets/images/AHI.png)

All modes functional?
Paula and LineIn working?

Great, congratulations, you did it!