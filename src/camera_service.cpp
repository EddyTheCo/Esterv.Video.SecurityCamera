#include"camera_service.hpp"
#include <opencv2/opencv.hpp>
#include"tcp_server.hpp"
#include<thread>

CameraService::CameraService()
{
    std::thread([this](){
        startMotionDetection();
    }).detach();
}
void CameraService::streamRecording(uint32_t command, uint32_t id)
{

}
void CameraService::startMotionDetection()
{
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open the camera" << std::endl;
    }
    else
    {
        cv::Mat frame,gray_frame, fgMask;

        int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
        int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

        std::cout << "Frame size: " << frame_width << "x" << frame_height << std::endl;

        cv::Ptr<cv::BackgroundSubtractor> bgSubtractor = cv::createBackgroundSubtractorMOG2();

        int fourcc = cv::VideoWriter::fourcc('M', 'P', '4', 'V');
        cv::VideoWriter writer;

        auto startTime = std::chrono::steady_clock::now();
        bool is_recording{false};
        while (true) {
            cap >> frame;
            cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
            bgSubtractor->apply(frame, fgMask);

            const int nonZeroPixels = cv::countNonZero(fgMask);
            const int totalPixels = frame.rows * frame.cols;

            const double changePercentage = (nonZeroPixels * 100.0) / totalPixels;

            const std::string output_name = "output" + std::to_string(recording_index) + ".mp4";

            if (changePercentage > 10) {
                std::cout << "Significant change detected: " << changePercentage << "% of pixels changed." << std::endl;
                startTime = std::chrono::steady_clock::now();
                if (!is_recording) {
                    writer.open(output_name, fourcc, 30.0, cv::Size(frame_width, frame_height),false);
                    if (!writer.isOpened()) {
                        std::cerr << "Error: Could not open the output video file" << std::endl;
                    }
                }
                is_recording = true;
            }
            auto currentTime = std::chrono::steady_clock::now();

            if (is_recording) {
                std::cout << "writing frame to :" << output_name << std::endl;

                std::time_t now = std::time(nullptr);  // Get current time in seconds
                char timestamp[100];
                std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

                cv::putText(gray_frame, timestamp, cv::Point(10, frame.rows - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5,
                            cv::Scalar(255, 255, 255), 1);

                writer.write(gray_frame);
                double elapsedSeconds
                    = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
                std::cout << "elapsedSeconds :" << elapsedSeconds << std::endl;
                if (elapsedSeconds >= 10) {
                    is_recording = false;
                    std::cout << "writer.release();" << std::endl;
                    writer.release();
                    recording_index++;
                }
            }
            std::vector<uint8_t> buffer;
            cv::imencode(".jpg", gray_frame, buffer);
            Session::sendFrame(buffer);
        }

    }


}
