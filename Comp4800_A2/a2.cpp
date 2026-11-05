#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>
#include <iostream>

// Removing duplicates using opencv defined function: cv.dnn::NMSBoxes
struct Detection {
    int classId;
    float confidence;
    cv::Rect box;
};

int main(int argc, char** argv) {

    // Check arguments - 0.5, 0.4 are good defaults
    if (argc != 4) {
        std::cout << "Usage: ./a2 <image> <confidence> <nms>" << std::endl;
        return -1;
    }

    std::string imageFile = argv[1];
    float confThreshold = std::stof(argv[2]);
    float nmsThreshold = std::stof(argv[3]);

    // Load image
    cv::Mat image = cv::imread(imageFile);
    if (image.empty()) {
        std::cout << "Error loading image." << std::endl;
        return -1;
    }

    // Load class names coco.names.txt
    std::vector<std::string> classNames;
    std::ifstream classFile("coco.names.txt");
    std::string line;
    while (std::getline(classFile, line))
        classNames.push_back(line);

    if (classNames.empty()) {
        std::cout << "Error: coco.names.txt not loaded." << std::endl;
        return -1;
    }

    // Load YOLOv3 network weights/.cfg file
    cv::dnn::Net net = cv::dnn::readNet("yolov3.weights", "yolov3.cfg");

    // Force CPU (WSL safe)
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    // Create Matrix blob
    cv::Mat blob = cv::dnn::blobFromImage(image, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);

    net.setInput(blob);

    // Forward pass
    std::vector<cv::Mat> outputs;
    std::vector<std::string> outputNames = net.getUnconnectedOutLayersNames();
    net.forward(outputs, outputNames);

    if (outputs.empty()) {
        std::cout << "YOLO forward failed." << std::endl;
        return -1;
    }

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (auto& out : outputs) {
        for (int i = 0; i < out.rows; i++) {

            float* data = (float*)out.ptr(i);

            float objectness = data[4];
            if (objectness < confThreshold)
                continue;

            cv::Mat scores = out.row(i).colRange(5, out.cols);

            cv::Point classIdPoint;
            double classConfidence;
            minMaxLoc(scores, 0, &classConfidence, 0, &classIdPoint);

            float confidence = objectness * classConfidence;

            if (confidence > confThreshold) {

                int centerX = (int)(data[0] * image.cols);
                int centerY = (int)(data[1] * image.rows);
                int width   = (int)(data[2] * image.cols);
                int height  = (int)(data[3] * image.rows);

                int left = centerX - width / 2;
                int top  = centerY - height / 2;

                left = std::max(0, left);
                top  = std::max(0, top);
                width  = std::min(width, image.cols - left);
                height = std::min(height, image.rows - top);

                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
                classIds.push_back(classIdPoint.x);
            }
        }
    }

    // Apply NMS
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    std::vector<Detection> detections;
    for (int i : indices) {
        Detection d;
        d.classId = classIds[i];
        d.confidence = confidences[i];
        d.box = boxes[i];
        detections.push_back(d);
    }

    if (detections.empty()) {
        std::cout << "No objects detected." << std::endl;
        return 0;
    }

    // Sort by bounding box area (descending)
    std::sort(detections.begin(), detections.end(), [](const Detection& a, const Detection& b) {
            return a.box.area() > b.box.area();
        });

    // Display one detected object at a time
    for (size_t i = 0; i < detections.size(); i++) {

        if (detections[i].classId >= classNames.size())
            continue;

        cv::Mat display = image.clone();

        cv::Scalar color(rand() % 256, rand() % 256, rand() % 256);

        cv::rectangle(display, detections[i].box, color, 2);

        std::string label = classNames[detections[i].classId] + " : " + cv::format("%.2f", detections[i].confidence);

        int baseline;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseline);

        //Label position to top-right corner and inside the image with some padding
        int padding = 10;
        cv::Point labelPos(display.cols - labelSize.width - padding, padding + labelSize.height);

        // White background box
        cv::rectangle(display,
            cv::Point(labelPos.x - 5, labelPos.y - labelSize.height - 5),
            cv::Point(labelPos.x + labelSize.width + 5, labelPos.y + baseline + 5),
            cv::Scalar(255, 255, 255),
            cv::FILLED
        );

        // Label text
        cv::putText(display, label, labelPos, cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);

        cv::imshow("YOLOv3 Object Detection", display);

        int key = cv::waitKey(0);

        if (key == 27) // ESC key to exit
            break;
    }

    cv::destroyAllWindows();
    return 0;
}
