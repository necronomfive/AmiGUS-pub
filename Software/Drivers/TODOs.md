Next:
- Switch to AmiGUS ints instead AHI timing
- Unify recording and playback buffer sizing 
  (as the buffer sizing can then be steered)
- Try snap upward 96kHz Mode
- Real 24bit recording
- Calibration of all audio sources' volumes with each other
- Calibrate volume AHI Mono vs Stereo
- Mixer UI elements and their responsiveness
- Get mpeg.device working as per https://aminet.net/package/mus/play/dmdev
- Make CAMD on https://aminet.net/package/mus/midi/camd / https://aminet.net/package/mus/play/projectomega / https://aminet.net/package/driver/other/uaemidi / https://aminet.net/package/mus/midi/camd40 / https://aminet.net/package/driver/other/mmp
- Start applying vendor patches to VS1063a firmware
- Fully split Recording + Playback parts in AHI to allow 1 client each

Done:
- Test 32/24bit recording as AHIST_S32S in AHIRecord -> Working
- Test DigiBooster panning as per https://www.a1k.org/forum/index.php?threads/90990/post-1826163 -> Working
- Deactivate AHI recording SRC!
- Prevent crashes when 2 AHI clients try allocating single AmiGUS
- Recording overflow handling similar to playback underrun handling
- Move to mode file: RecordingSampleTypeById, RecordingSampleAlignmentById, AmiGUSPlaybackSampleSizes -> moved all tables to lookup based on mode id
- Test Recording and release alpha15 / beta 15
- Get MHI implemented as per https://bitbucket.org/supernobby/mhimdev/src/main/ and https://aminet.net/package/driver/audio/mhimasplayer
- Implement MHI equalizer
- Fix stupid sounds blooping from one song to the next
- MHI Volume handline - test SCI_VOL 1063ds p.53
- Fix MHI Unit Tests
- AHI/MHI: move reservation of expansion device from driver loading to alloc/dealloc
- Move MHI interrupt.c::HandlePlayback() back to interrupt and create a buffer.c::getNextLong(), adapt tests
- Left/right swapped in stereo 8bit vs 16bit + 24bit
- Decide on debug + cpu variants
- Merge MHI base library back to AHI
- Merge MHI debug code back to AHI
- Test AHI, especially mem logging
- Build 64kHz mode into AHI
- Create Install disk script
- Write missing comments and ReadMe.mds