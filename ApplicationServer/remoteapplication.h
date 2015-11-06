#ifndef REMOTEAPPLICATION_H
#define REMOTEAPPLICATION_H

#include <QObject>
#include <QHostAddress>
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

signals:
    void imageUpdate();

protected slots:
    void readSocket();

private:
    QHostAddress m_myIPv4;
    quint16 m_port;
    class QUdpSocket* m_udpSocket;
    QRect m_geometry;
    QImage m_image;

};

#endif // REMOTEAPPLICATION_H
