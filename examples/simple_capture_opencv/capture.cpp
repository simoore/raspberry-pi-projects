#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

int main(int argc, char **argv) {

    const cv::String windowTitle = "Capture Example";
    const int deviceId = 0;     // 0 = open default camera

    cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);
    cv::VideoCapture capture;
    capture.open(deviceId, cv::CAP_ANY);

    if (!capture.isOpened()) {
        std::cout << "Camera not open." << std::endl;
        return -1;
    }

    while (true) {
        cv::Mat frame;
        capture.read(frame);

        if (frame.empty()) {
            std::cout << "Frame is empty." << std::endl;
            return -1;
        }

        cv::imshow(windowTitle, frame);

        if (cv::waitKey(33) == 27) {
            break;
        }
    }
    return 0;
}
