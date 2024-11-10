#include <opencv2/opencv.hpp>
#include "tcp_client.hpp"
#include <QImage>


void Client::tryConnect()
{
    if(port_>0u&&!address_.isEmpty())
    {
        socket_->disconnectFromHost();
        qDebug()<<"connectToHost";
        socket_->connectToHost(address_, port_);
    }
}
void Client::onConnected()
{
    qDebug()<<"Connected";
    conn_state_=Connected;
    emit stateChanged();
    sendCommand(1);
}
void Client::onDisconnected()
{
    qDebug()<<"Disconnected";
    conn_state_=Disconnected;
    emit stateChanged();
}
Client::Client(QObject *parent) : QObject(parent), socket_(new QTcpSocket(this)) {
    connect(socket_, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket_, &QTcpSocket::readyRead, this, &Client::analyzeData);
    connect(socket_, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(socket_, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError socketError) {
        qDebug() << "Connection failed with error:" << socketError;
    });
}
void Client::sendCommand(uint8_t command)
{
    QByteArray data;
    data.push_back(command);
    socket_->write(data);
    socket_->flush();
}
void Client::analyzeData() {

    std::size_t pos{0};
    const auto data=socket_->readAll();
    const auto length=data.size();
    while (pos < length) {

        switch (read_state_) {
        case ReadingState::Size:
            frame_size_array_.push_back(data.at(pos));
            if(frame_size_array_.size() == 4)
            {
                auto buffer = QDataStream(&frame_size_array_, QIODevice::ReadOnly);
                buffer.setByteOrder(QDataStream::BigEndian);
                buffer >> frame_size_;
                if(frame_size_<2147483648)
                {
                    read_state_ = ReadingState::FrameData;
                }
                else
                {
                    read_state_ = ReadingState::DescriptionData;
                }
                frame_size_array_.clear();
            }

            ++pos;
            break;
        case ReadingState::FrameData:
            const auto take_size =
                std::min(static_cast<quint32>(frame_size_ - frame_array_.size()), static_cast<quint32>(length - pos));
            std::move(data.begin() + pos, data.begin() + pos + take_size,
                      std::back_inserter(frame_array_));
            pos += take_size;
            if (frame_array_.size() == frame_size_) {
                gotFrame();
                read_state_ = ReadingState::Size;
                frame_array_.clear();
            }
            break;
        }
    }
}

void Client::gotFrame()
{
    std::vector<uint8_t> frame(frame_array_.begin(), frame_array_.end());
    const auto grayMat= cv::imdecode(frame, cv::IMREAD_GRAYSCALE);
    WasmImageProvider::img = QImage(grayMat.data, grayMat.cols, grayMat.rows, grayMat.step, QImage::Format_Grayscale8);
    setFrameId();
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
void Client::setFrameId() {
    static quint8 index = 0;
    frame_src_ = "camimage" + QString::number(index);
    emit sourceChanged();
    index++;
}
