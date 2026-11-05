A5: Retrieving Info from format context

February 24, 2026 1 

• C and ffmpeg
• Two allowed header files only.
• Maximum 50 lines of code.
• Video file name should be an input.
• Provide a short video.
• Existing dump function from ffmpeg allowed for type 0.
• Output for type 1 should follow the sample. No additional output allowed.
• Individual work. 
• Points: 1 demo.
• Demo in class: Tu. Mar. 3


Running commands: 
Step 1 - Navigate to assignment folder "Comp4800_A5"
cd Comp4800_A5 

Step 2 - Check contents you should see "balumbalum.mp4" and "getinfo.c"
ls 

Step 3 - Compiling the program to get "tmp" file 
cc getinfo.c -lavformat -lavutil -o tmp 

If it compiles successfully, it will return to prompt silently. 

Step 4 - Run with no aguments (Usage Test)
./tmp

Expected output:

usage: argv[0] filename type(0:dump,1:selected)

Good. That shows argument validation works.

Step 5 - Run Type 0 (Full Dump)
./tmp myvideo.mp4 0

You should see a large FFmpeg metadata dump:

Input #0, mov,mp4,...
  Metadata:
  Duration: ...
  Stream #0:0: Video: ...
  Stream #0:1: Audio: ...

This proves:

avformat_open_input() works

av_dump_format() works

Step 6 - Run Type 1 (Selected Output)
./tmp myvideo.mp4 1

Expected format:

number of streams: 2
frame rate: 30.0
duration: 0 min 15 sec

Demo Flow In Class: 
cd Comp4800_A5
cc getinfo.c -lavformat -lavutil -o tmp
./tmp
./tmp myvideo.mp4 0
./tmp myvideo.mp4 1