#include <opencv2/opencv.hpp>
#include "tcp_client.hpp"
#include <QImage>


void Client::analyzeData() {

        received_data_.append(socket_->readAll());
        switch (state_) {
        case ReadingState::Size:

            if(received_data_.size()>=4)
            {
                auto buffer = QDataStream(&received_data_, QIODevice::ReadOnly);
                buffer.setByteOrder(QDataStream::BigEndian);
                buffer >> frame_size_;
                if(frame_size_<2147483648)
                {
                    state_ = ReadingState::FrameData;
                }
                else
                {
                    state_ = ReadingState::DescriptionData;
                }

                received_data_.remove(0, 4);
            }
            break;
        case ReadingState::FrameData:

            if(received_data_.size()>=frame_size_)
            {
                gotFrame();
                state_ = ReadingState::Size;
                received_data_.remove(0, frame_size_);
            }
            break;
        }
}

void Client::gotFrame()
{
    std::vector<uint8_t> frame(received_data_.begin(), received_data_.begin()+frame_size_);
    const auto grayMat= cv::imdecode(frame, cv::IMREAD_GRAYSCALE);
    QImage grayImage(grayMat.data, grayMat.cols, grayMat.rows, grayMat.step, QImage::Format_Grayscale8);
}
void WasmImageProvider::restart(void)
{
    WasmImageProvider::img=QImage(QSize(200,150),QImage::Format_RGBA8888);
    WasmImageProvider::img.fill("black");
}
QImage WasmImageProvider::img=QImage();
QImage WasmImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    return img;
}
