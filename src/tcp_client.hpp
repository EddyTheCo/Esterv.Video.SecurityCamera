#pragma once


#include <cstdint>
#include <string>
#include <QObject>
#include<QTcpSocket>
#include <qquickimageprovider.h>

class Client : public QObject {
    Q_OBJECT

    QTcpSocket *socket_;
    enum class ReadingState { Size, FrameData, DescriptionData };
    ReadingState state_{ReadingState::Size};
    QByteArray received_data_;
    quint32 frame_size_;

    void analyzeData();
    void gotFrame();


    std::string server_ip_;
    uint32_t server_port_;

public:

    Client(QObject *parent = nullptr);
    void sendCommand(uint8_t command);
};

class WasmImageProvider : public QQuickImageProvider
{
public:
    WasmImageProvider():QQuickImageProvider(QQuickImageProvider::Image)
    {
        restart();
    }
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    static void restart(void);
    static QImage img;
};
