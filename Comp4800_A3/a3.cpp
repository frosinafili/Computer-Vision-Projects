#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {

    // 1. Ask user for animation rate
    int delay;
    cout << "Enter animation delay in milliseconds (ms): ";
    cin >> delay;

    // Color reduction mode
    int colorMode = 0; // Initialize with no colour reduction 
    
    // 0 = normal; 1 = reduce red; 2 = reduce green; 3 = reduce blue    

    // 2. Load background image
    Mat background = imread("disco-floor.jpg");
    if (background.empty()) {
        cout << "Cannot load image, using blank canvas." << endl;
        background = Mat::zeros(500, 500, CV_8UC3);
    }

    namedWindow("Disco Animation", WINDOW_NORMAL);

    // Ball properties
    int radius = 25;
    Point center(100, 100);
    int dx = 5;
    int dy = 4;
    Scalar ballColor(0, 0, 255); // Red (BGR)

    while (true) {

        // Start with fresh background
        Mat frame = background.clone();

        // Applying fcolour recution method from A1
        if (colorMode == 1) { // reduce red
            for (int y = 0; y < frame.rows; y++)
                for (int x = 0; x < frame.cols; x++)
                    frame.at<Vec3b>(y, x)[2] /= 4;
        }
        else if (colorMode == 2) { // reduce green
            for (int y = 0; y < frame.rows; y++)
                for (int x = 0; x < frame.cols; x++)
                    frame.at<Vec3b>(y, x)[1] /= 4;
        }
        else if (colorMode == 3) { // reduce blue
            for (int y = 0; y < frame.rows; y++)
                for (int x = 0; x < frame.cols; x++)
                    frame.at<Vec3b>(y, x)[0] /= 4;
        }

        // Draw disco ball
        circle(frame, center, radius, ballColor, -1);

        imshow("Animation", frame);

        int key = waitKey(delay);
        if (key == 27) // ESC
            break;

        //MOVE BALL
        center.x += dx;
        center.y += dy;

        //BOUNCE + CHANGE COLOR MODE
        if (center.x - radius <= 0 || center.x + radius >= frame.cols) {
            dx = -dx;
            colorMode = (colorMode + 1) % 4;
        }

        if (center.y - radius <= 0 || center.y + radius >= frame.rows) {
            dy = -dy;
            colorMode = (colorMode + 1) % 4;
        }
    }

    destroyAllWindows();
    return 0;
}
