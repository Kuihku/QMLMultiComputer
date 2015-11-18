#ifndef REMOTEAPPLICATION_H
#define REMOTEAPPLICATION_H

#include <QObject>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QRect>
#include <QImage>

class RemoteApplication : public QObject
{
    Q_OBJECT
public:
    explicit RemoteApplication(QHostAddress IPv4, quint16 port, QObject *parent = Q_NULLPTR);
    class QUdpSocket* udpSocket() const;
    void updateGeometry(QRect geometry);
    quint16 port() const;
    void sendImage(const QImage& image);
    QRect geometry() const;
    void paintImage(class QPainter *painter);

signals:
    void imageUpdate(QRect);

protected slots:
    void readSocket();
    void socketError(QAbstractSocket::SocketError error);
    void udbBytesWritten(qint64 bytes);

private:
    QHostAddress m_IPv4;
    quint16 m_port;
    class QUdpSocket* m_udpSocket;
    QRect m_geometry;
    QImage m_image;

};

#endif // REMOTEAPPLICATION_H
