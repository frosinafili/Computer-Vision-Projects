COMP 4800 – Assignment 2 YOLOv3 Object Detection

This program uses OpenCV and YOLOv3 to detect objects in an image.
The image file name, confidence threshold, and NMS threshold are passed
as command line arguments.

Detected objects are shown one at a time. The bounding box and label
change color for each object. The class label is displayed in the
top-right corner of the image for better clarity. 

Press any key to move to the next detected object.
Press ESC to exit the program.


Compiling (WSL / Linux):
g++ a2.cpp -o a2 `pkg-config --cflags --libs opencv4`

Running:
./a2 image.jpg 0.5 0.4
./a2 newyork.jpg 0.5 0.4

Required files in the same folder:
- a2.cpp
- yolov3.cfg
- yolov3.weights
- coco.names.txt

Frosina Filiposka 
[SID]


