#include "remoteapplication.h"

#include <QUdpSocket>
#include <QBuffer>
#include <QDataStream>
#include <QPainter>

RemoteApplication::RemoteApplication(QHostAddress IPv4, quint16 port, QObject *parent) :
    QObject(parent),
    m_IPv4(IPv4),
    m_port(port),
    m_udpSocket(new QUdpSocket(this))
{
    connect(m_udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(m_udpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(udbBytesWritten(qint64)));
}

void RemoteApplication::bind()
{
    if (m_udpSocket->bind(m_IPv4, m_port)) {
        qDebug() << "RemoteApplication::bind - udpSocket bind addr:" << m_IPv4 << "- port:" << m_port;
        connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    }
    else {
        qWarning() << "RemoteApplication::bind - udpSocket bind error:" << m_udpSocket->errorString();
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
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << image;
    int size(buffer.size());
    qDebug() << "RemoteApplication::sendImage - bytearray size:" << size;
    m_udpSocket->writeDatagram(buffer.data(), size, m_IPv4, m_port);
    buffer.close();
}

QRect RemoteApplication::geometry() const
{
    return m_geometry;
}

void RemoteApplication::paintImage(QPainter *painter)
{
    if (!m_image.isNull()) {
        painter->drawImage(m_geometry, m_image);
    }
}

void RemoteApplication::readSocket()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        qint64 size(m_udpSocket->pendingDatagramSize());
        datagram.resize(size);
        qDebug("RemoteApplication::readSocket - size: %d", (int)size);
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
        QBuffer buffer;
        buffer.setData(datagram);
        buffer.open(QBuffer::ReadOnly);
        QDataStream in(&buffer);
        QImage image;
        in >> image;
        buffer.close();
        if (!image.isNull()) {
            m_image = image;
            emit imageUpdate(m_geometry);
        }
        else {
            qWarning("RemoteApplication::readSocket - Error: image is null");
        }
    }
}

void RemoteApplication::socketError(QAbstractSocket::SocketError error)
{
    qWarning() << "RemoteApplication::socketError - code:" << error << "- error:" << m_udpSocket->errorString();
}

void RemoteApplication::udbBytesWritten(qint64 bytes)
{
    qDebug("RemoteApplication::udbBytesWritten - bytes: %d", (int)bytes);
}
