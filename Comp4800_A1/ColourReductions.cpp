#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat reduceChannel(const cv::Mat& img, int channel, int reduction) {
    cv::Mat result = img.clone();

    for (int i = 0; i < result.rows; i++) {
        for (int j = 0; j < result.cols; j++) {
            cv::Vec3b& pixel = result.at<cv::Vec3b>(i, j);
            int value = pixel[channel] - reduction;
            if (value < 0)
                pixel[channel] = 0;
        else
                pixel[channel] = value;
        }
    }
    return result;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: ./a1 <image_file> <reduction_amount>" << std::endl;
        return -1;
    }

    std::string filename = argv[1];
    int reduction = atoi(argv[2]);

    cv::Mat original = cv::imread(filename);
    if (original.empty()) {
        std::cout << "Error: Cannot load image." << std::endl;
        return -1;
    }

    cv::Mat redReduction   = reduceChannel(original, 2, reduction);
    cv::Mat greenReduction = reduceChannel(original, 1, reduction);
    cv::Mat blueReduction  = reduceChannel(original, 0, reduction);

    std::vector<cv::Mat> images = {
        original,
        redReduction,
        greenReduction,
        blueReduction
    };

    cv::destroyAllWindows();
   
    cv::namedWindow("Assignment 1", cv::WINDOW_NORMAL);
    cv::resizeWindow("Assignment 1", 900, 600);
    cv::moveWindow("Assignment 1", 510, 240);
    
    int index = 0;

    cv::imshow("Assignment 1", images[index]);

    while (true) {
        int key = cv::waitKey(0);
        if (key == 27) {
            break;
        }
        index = (index + 1) % images.size();
        cv::imshow("Assignment 1", images[index]);
    }
    cv::destroyAllWindows();
    return 0;
}