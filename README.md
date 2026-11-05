# Computer Vision Projects (COMP 4800)

**COMP 4800 — In-Class Demos and Assignments | Winter 2026**

Welcome to my portfolio for **COMP 4800: Computer Vision**. This repository contains eight assignments exploring digital image processing, object detection, computer graphics, clustering, video processing, multithreading, and audio generation.

The projects emphasize hands-on implementation of computer vision and multimedia concepts using **C, C++, OpenCV, FFmpeg, and platform-specific APIs**.

---

## Core Technologies & Environment

- **Languages:** C and C++
- **Computer Vision & Graphics:** OpenCV (image manipulation, drawing, visualization, and DNN inference)
- **Deep Learning:** YOLOv3 using OpenCV's DNN module and COCO class labels
- **Video Processing:** FFmpeg (`libavformat`, `libavcodec`, `libavutil`, `libswscale`)
- **Concurrency:** POSIX threads (`pthread`), mutexes, condition variables, and a circular buffer
- **Audio:** Windows Core Audio (WASAPI)
- **Platforms:** Linux/WSL for several assignments; Windows for Assignment 8

---

## Repository Roadmap

Each directory contains a separate assignment, including its source code and any accompanying sample inputs or documentation.

- **[Comp4800_A1](./Comp4800_A1):** **RGB Colour Reduction** — Manipulates individual red, green, and blue channels of digital images using pixel-level OpenCV operations and an interactive image viewer.
- **[Comp4800_A2](./Comp4800_A2):** **YOLOv3 Object Detection** — Detects and labels objects in images using a pretrained YOLOv3 model, confidence thresholds, and non-maximum suppression (NMS).
- **[Comp4800_A3](./Comp4800_A3):** **Interactive Disco Animation** — Animates a bouncing ball over an image background, switching colour-channel reduction effects when the ball hits the boundaries.
- **[Comp4800_A4](./Comp4800_A4):** **K-Means Clustering Visualization** — Implements K-means for 2D points and animates centroid movement, cluster assignments, and convergence using OpenCV.
- **[Comp4800_A5](./Comp4800_A5):** **Video Metadata Extraction** — Uses FFmpeg in C to display full media information or selected properties, including stream count, frame rate, and video duration.
- **[Comp4800_A6](./Comp4800_A6):** **Video Frame Extraction & Grayscale Conversion** — Decodes a specified video frame with FFmpeg and saves it as a colour PPM image and grayscale PGM images using different RGB weightings.
- **[Comp4800_A7](./Comp4800_A7):** **Multithreaded Video Playback** — Combines FFmpeg decoding, an OpenCV display window, and POSIX producer-consumer threads synchronized through a circular frame buffer.
- **[Comp4800_A8](./Comp4800_A8):** **Windows Audio Synthesis** — Uses the Windows Core Audio API to access an output device, report audio settings, and generate a pulsing synthesized tone.

---

## Quickstart: Running Projects Locally

### 1. Clone the Repository

```bash
git clone <your-repository-url>
cd Computer-Vision-Projects
```

Replace `<your-repository-url>` with the actual GitHub clone URL and adjust the directory name if needed.

### 2. Install the Required Dependencies

Dependencies vary by assignment. On a Debian/Ubuntu-based Linux environment, common development packages include:

```bash
sudo apt update
sudo apt install build-essential pkg-config libopencv-dev libavformat-dev libavcodec-dev libavutil-dev libswscale-dev
```

**Note:** Assignment 8 targets Windows and requires Windows Core Audio headers and libraries; it is not a Linux/WSL program.

### 3. Compile and Run an Assignment

For example, Assignment 1 uses OpenCV:

```bash
cd Comp4800_A1
g++ ColourReductions.cpp -o a1 $(pkg-config --cflags --libs opencv4)
./a1 test.jpg 50
```

For assignment-specific commands and arguments, check the relevant source file or the README inside its directory (where provided).

### 4. Additional Notes

- Some assignments require local image or video inputs and a graphical display environment. You can use your choice of these. 
- Assignment 2 uses YOLOv3 model configuration, COCO labels, and pretrained weights. The weights file is large; if you exclude it from Git version control, download it separately before running the program.
- Binary executables, generated media outputs, and large pretrained model files may be excluded from Git tracking to keep the repository lightweight.
