Next:
- Try snap upward 96kHz Mode
- Move to mode file: RecordingSampleTypeById, RecordingSampleAlignmentById, AmiGUSPlaybackSampleSizes
- Recording overflow handling similar to playback underrun handling
- Real 24bit recording
- Left/right swapped in stereo 8bit vs 16bit + 24bit 
- Calibration of all audio sources' volumes with each other
- Calibrate volume AHI Mono vs Stereo
- Mixer UI elements and their responsiveness
- Get MHI implemented as per https://bitbucket.org/supernobby/mhimdev/src/main/
- Get mpeg.device working as per https://aminet.net/package/mus/play/dmdev
- Make CAMD on https://aminet.net/package/mus/midi/camd / https://aminet.net/package/mus/play/projectomega / https://aminet.net/package/driver/other/uaemidi / https://aminet.net/package/mus/midi/camd40 / https://aminet.net/package/driver/other/mmp
- Fully split Recording + Playback parts in AHI to allow 1 client each

Done:
- Test 32/24bit recording as AHIST_S32S in AHIRecord -> Working
- Test DigiBooster panning as per https://www.a1k.org/forum/index.php?threads/90990/post-1826163 -> Working
- Deactivate AHI recording SRC!
- Prevent crashes when 2 AHI clients try allocating single AmiGUS
