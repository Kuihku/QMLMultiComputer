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
    explicit RemoteApplication(QHostAddress myIPv4, quint16 port, QObject *parent = 0);
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

private:
    QHostAddress m_myIPv4;
    quint16 m_port;
    class QUdpSocket* m_udpSocket;
    QRect m_geometry;
    QImage m_image;

};

#endif // REMOTEAPPLICATION_H
