#pragma once

#include <QObject>
#include <QWebSocket>
#include <cstdint>
#include <qquickimageprovider.h>

class Client : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl address MEMBER address_ NOTIFY addressChanged)
  Q_PROPERTY(ConState state READ state NOTIFY stateChanged)
  Q_PROPERTY(QString frameSource READ getSource NOTIFY sourceChanged)
  QML_ELEMENT

  QWebSocket *socket_;
  enum class ReadingState { Size, FrameData, DescriptionData };
  ReadingState read_state_{ReadingState::Size};
  QByteArray frame_array_;
  QByteArray frame_size_array_;
  quint32 frame_size_;
  QString frame_src_;

  void analyzeData(const QByteArray &message);
  void gotFrame();
  void setFrameId();

  void onConnected();
  void onDisconnected();

  QUrl address_;

public:
  enum ConState { Disconnected = 0, Connected };
  Q_ENUM(ConState)
  ConState state(void) { return conn_state_; }
  Client(QObject *parent = nullptr);
  Q_INVOKABLE void sendCommand(uint8_t command);
  Q_INVOKABLE void tryConnect();
  QString getSource(void) const { return frame_src_; }
signals:
  void stateChanged();
  void sourceChanged();
  void addressChanged();
  void portChanged();

private:
  ConState conn_state_{Disconnected};
};

class WasmImageProvider : public QQuickImageProvider {
public:
  WasmImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {
    restart();
  }
  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override;
  static void restart(void);
  static QImage img;
};
