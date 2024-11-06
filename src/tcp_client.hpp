#pragma once


#include <cstdint>
#include <string>
#include <QObject>
#include <QTcpSocket>
#include <qquickimageprovider.h>

class Client : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString address MEMBER address_ NOTIFY addressChanged)
    Q_PROPERTY(quint16 port MEMBER port_ NOTIFY portChanged)
    Q_PROPERTY(ConState  state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString frameSource READ getSource NOTIFY sourceChanged)
    QML_ELEMENT




    QTcpSocket *socket_;
    enum class ReadingState { Size, FrameData, DescriptionData };
    ReadingState read_state_{ReadingState::Size};
    QByteArray received_data_;
    quint32 frame_size_;
    QString frame_src_;
    QString address_;
    quint16 port_{0};

    void analyzeData();
    void gotFrame();
    void setFrameId();
    void tryConnect();
    void onConnected();
    void onDisconnected();


    std::string server_ip_;
    uint32_t server_port_;

public:
    enum ConState {
        Disconnected = 0,
        Connected
    };
    Q_ENUM(ConState)
    ConState state(void){return conn_state_;}
    Client(QObject *parent = nullptr);
    Q_INVOKABLE void sendCommand(uint8_t command);
    QString getSource(void) const { return frame_src_; }
signals:
    void stateChanged();
    void sourceChanged();
    void addressChanged();
    void portChanged();
private:
    ConState conn_state_{Disconnected};

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
