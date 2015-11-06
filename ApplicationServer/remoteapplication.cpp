#include "remoteapplication.h"

#include <QUdpSocket>
#include <QBuffer>

RemoteApplication::RemoteApplication(QHostAddress myIPv4, quint16 port, QObject *parent) :
    QObject(parent),
    m_myIPv4(myIPv4),
    m_port(port),
    m_udpSocket(new QUdpSocket(this))
{
    if (!m_udpSocket->bind(myIPv4, port)) {
        qWarning() << "RemoteApplication::RemoteApplication - udpSocket bind error:" << m_udpSocket->errorString();
    }
    else {
        connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    }
}

QUdpSocket* RemoteApplication::udpSocket() const
{
    return m_udpSocket;
}

void RemoteApplication::updateGeometry(QRect geometry)
{
    m_geometry = geometry;
}

quint16 RemoteApplication::port() const
{
    return m_udpSocket->localPort();
}

void RemoteApplication::sendImage(const QImage &image)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); // writes image into ba in PNG format
    m_udpSocket->writeDatagram(ba.data(), ba.size(), m_myIPv4, m_port);
}

void RemoteApplication::readSocket()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
        QImage image(QImage::fromData(datagram, "PNG"));
        if (!image.isNull()) {
            m_image = image;
        }
    }
    if (!m_image.isNull()) {
        emit imageUpdate();
    }
}
