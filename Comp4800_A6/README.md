A6 - Comp4800 

Frosina Filiposka 
[SID]

This program extracts a specific frame from a video file using FFmpeg libraries.

1. Opens the video file

2. Finds the video stream

3. Decodes frames one by one

4. Stops at the requested frame number

5. Converts that frame to RGB format

Saves:
1. One color image (PPM)
2. Four grayscale images (PGM) using different RGB weight combinations

Compilation
cc a6.c -lavformat -lavcodec -lavutil -lswscale -o tmp

How to Run
./tmp <video_file> <frame_number>

Example:
./tmp balumbalum.mp4 100

This extracts frame 100.

Output Files
After running, the program creates:

image0.ppm → color image
image1.pgm → grayscale (r+g+b)/3
optional: image2.pgm → 0.6r + 0.2g + 0.2b
optional: image3.pgm → 0.2r + 0.6g + 0.2b
optional: image4.pgm → 0.2r + 0.2g + 0.6b

Notes
The program uses sws_scale() to convert the frame from YUV to RGB.
All allocated memory is properly freed.
Images are saved in Netpbm format (PPM/PGM).