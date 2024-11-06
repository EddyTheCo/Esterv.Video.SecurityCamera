#include <opencv2/opencv.hpp>
#include "tcp_client.hpp"
#include <QImage>


void Client::tryConnect()
{
    if(port_>0u&&!address_.isEmpty())
    {
        socket_->connectToHost(address_, port_);
    }
}
void Client::onConnected()
{
    conn_state_=Connected;
    emit stateChanged();
}
void Client::onDisconnected()
{
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
    connect(this, &Client::addressChanged, this, &Client::tryConnect);
    connect(this, &Client::portChanged, this, &Client::tryConnect);
}
void Client::sendCommand(uint8_t command)
{
    QByteArray data;
    data.push_back(command);
    socket_->write(data);
    socket_->flush();
}
void Client::analyzeData() {

        received_data_.append(socket_->readAll());
        switch (read_state_) {
        case ReadingState::Size:

            if(received_data_.size()>=4)
            {
                auto buffer = QDataStream(&received_data_, QIODevice::ReadOnly);
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

                received_data_.remove(0, 4);
            }
            break;
        case ReadingState::FrameData:

            if(received_data_.size()>=frame_size_)
            {
                gotFrame();
                read_state_ = ReadingState::Size;
                received_data_.remove(0, frame_size_);
            }
            break;
        }
}

void Client::gotFrame()
{
    std::vector<uint8_t> frame(received_data_.begin(), received_data_.begin()+frame_size_);
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
