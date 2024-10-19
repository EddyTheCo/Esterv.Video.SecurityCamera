#include <opencv2/bgsegm.hpp>
#include <opencv2/opencv.hpp>

int main()
{
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open the camera" << std::endl;
        return -1;
    }

    cv::Mat frame, fgMask;

    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    std::cout << "Frame size: " << frame_width << "x" << frame_height << std::endl;

    cv::Ptr<cv::BackgroundSubtractor> bgSubtractor = cv::createBackgroundSubtractorMOG2();

    int index = 0;
    int fourcc = cv::VideoWriter::fourcc('M', 'P', '4', 'V');
    cv::VideoWriter writer;

    auto startTime = std::chrono::steady_clock::now();
    bool is_recording{false};
    while (true) {
        cap >> frame;
        bgSubtractor->apply(frame, fgMask);

	const int nonZeroPixels = cv::countNonZero(fgMask);
	const int totalPixels = frame.rows * frame.cols;

	const double changePercentage = (nonZeroPixels * 100.0) / totalPixels;

    const std::string output_name = "output" + std::to_string(index) + ".mp4";

    if (changePercentage > 10) {
        std::cout << "Significant change detected: " << changePercentage << "% of pixels changed." << std::endl;
        startTime = std::chrono::steady_clock::now();
        if (!is_recording) {
            writer.open(output_name, fourcc, 30.0, cv::Size(frame_width, frame_height));
            if (!writer.isOpened()) {
                std::cerr << "Error: Could not open the output video file" << std::endl;
                return -1;
            }
        }
        is_recording = true;
    }
    auto currentTime = std::chrono::steady_clock::now();

    if (is_recording) {
        std::cout << "writing frame to :" << output_name << std::endl;
        writer.write(frame);
        double elapsedSeconds
            = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        std::cout << "elapsedSeconds :" << elapsedSeconds << std::endl;
        if (elapsedSeconds >= 10) {
            is_recording = false;
            std::cout << "writer.release();" << std::endl;
            writer.release();
            index++;
        }
    }

    //cv::imshow("Foreground Mask", fgMask);

    //cv::imshow("Recording...", frame);

    if (cv::waitKey(30) == 27) {
        break;
    }
    }

    return 0;
}
