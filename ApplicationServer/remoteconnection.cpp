#include "remoteconnection.h"
#include "remoteconnectioninfo.h"
#include "remoteapplication.h"
#include "message.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QMetaObject>
#include <QUdpSocket>
#include <QMapIterator>
#include <QPainter>

#include <QDebug>

RemoteConnection::RemoteConnection(QHostAddress myIPv4, Remote::Direction remoteDirection, QByteArray ip, QObject *parent) :
    QObject(parent),
    m_nextUdpPort(49152),
    m_myIPv4(myIPv4),
    m_remoteDirection(remoteDirection),
    m_remoteSocket(new QTcpSocket(this))
{
    QHostAddress remoteAddress(QString::fromLatin1(ip));
    if (remoteAddress.isNull()) {
        qWarning() << "RemoteConnection::RemoteConnection - error ip:" << ip << "- address:" << remoteAddress;
    }
    else {
        connect(m_remoteSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(m_remoteSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(m_remoteSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        m_remoteSocket->connectToHost(remoteAddress, REMOTE_PORT);
    }
}

RemoteConnection::RemoteConnection(QHostAddress myIPv4, QTcpSocket* socket, QObject *parent) :
    QObject(parent),
    m_nextUdpPort(49152),
    m_myIPv4(myIPv4),
    m_remoteDirection(Remote::Undefined),
    m_remoteSocket(socket)
{
}

Remote::Direction RemoteConnection::remoteDirection() const
{
    return m_remoteDirection;
}

void RemoteConnection::updateGeometry(QString appUid, int x, int y, int width, int height)
{
    RemoteApplication* remoteApplication(m_remoteApplications.value(appUid, NULL));
    if (!remoteApplication) {
        remoteApplication = new RemoteApplication(m_myIPv4, m_nextUdpPort++, this);
        if (m_nextUdpPort > 65534) {
            m_nextUdpPort = 49153;
        }
        m_remoteApplications.insert(appUid, remoteApplication);
    }
    if (m_remoteSocket->isWritable()) {
        RemoteGeometryMessage gm(appUid, remoteApplication->port(), x, y, width, height);
        gm.write(m_remoteSocket);
    }
}

void RemoteConnection::sendImage(QString appUid, const QImage& image)
{
    RemoteApplication* remoteApplication(m_remoteApplications.value(appUid, NULL));
    if (remoteApplication) {
        remoteApplication->sendImage(image);
    }
}

void RemoteConnection::paintImages(QRegion region, QPainter *painter)
{
    QMapIterator<QString, RemoteApplication*> remoteApplicationIterator(m_remoteApplications);
    while (remoteApplicationIterator.hasNext()) {
        remoteApplicationIterator.next();
        RemoteApplication* remoteApplication(remoteApplicationIterator.value());
        if (region.contains(remoteApplication->geometry())) {
            remoteApplication->paintImage(painter);
        }
    }
}

void RemoteConnection::socketConnected()
{
    qDebug("RemoteConnection::socketConnected");
    RemoteDirectionMessage rdm(QString(), m_remoteDirection);
    rdm.write(m_remoteSocket);
}

void RemoteConnection::socketDisconnected()
{
    qDebug("RemoteConnection::socketDisconnected");
}

void RemoteConnection::readSocket()
{
    qDebug() << "RemoteConnection::readSocket - bytes available:" << m_remoteSocket->bytesAvailable();
    Message* m(Message::read(m_remoteSocket));
    if (m) {
        switch (m->type()) {
            case MessageType::Undefined : qWarning("RemoteConnection::readSocket - message type: Undefined"); break;
            case MessageType::RemoteDirection : {
                RemoteDirectionMessage* rdm(dynamic_cast<RemoteDirectionMessage*>(m));
                handleRemoteDirection((Remote::Direction)rdm->remoteDirection());
                break;
            }
            case MessageType::RemoteGeometry: {
                RemoteGeometryMessage* gm(dynamic_cast<RemoteGeometryMessage*>(m));
                handleGeometryUpdate(gm->appUid(), gm->port(), gm->geometry());
            break;
            }
            case MessageType::RemoteLaunch: {
                RemoteLaunchMessage* rlm(dynamic_cast<RemoteLaunchMessage*>(m));
                handleRemoteLaunchUpdate(rlm->appUid(), rlm->data());
            }
            // TODO: Remove later, For testing purposes only
            case MessageType::RemotePing: {
                qDebug("RemoteConnection::readSocket - ping received");
                m->write(m_remoteSocket);
            break;
            }
            default : {
                qDebug() << "RemoteConnection::readSocket - message:" << *m;
                break;
            }
        }
        delete m;
    }
}

void RemoteConnection::handleRemoteDirection(Remote::Direction remoteDicrection)
{
    if (remoteDicrection == Remote::NorthWest) {
        m_remoteDirection = Remote::SouthEast;
    }
    else if (remoteDicrection == Remote::North) {
        m_remoteDirection = Remote::South;
    }
    else if (remoteDicrection == Remote::NorthEast) {
        m_remoteDirection = Remote::SouthWest;
    }
    else if (remoteDicrection == Remote::West) {
        m_remoteDirection = Remote::East;
    }
    else if (remoteDicrection == Remote::East) {
        m_remoteDirection = Remote::West;
    }
    else if (remoteDicrection == Remote::SouthWest) {
        m_remoteDirection = Remote::NorthEast;
    }
    else if (remoteDicrection == Remote::South) {
        m_remoteDirection = Remote::North;
    }
    else if (remoteDicrection == Remote::SouthEast) {
        m_remoteDirection = Remote::NorthWest;
    }
    emit connectionReady();
}

void RemoteConnection::handleGeometryUpdate(QString appUid, quint16 port, QRect rect)
{
    RemoteApplication* remoteApplication(m_remoteApplications.value(appUid, NULL));
    if (!remoteApplication) {
        remoteApplication = new RemoteApplication(m_myIPv4, port, this);
        connect(remoteApplication, SIGNAL(imageUpdate(QRect)), this, SIGNAL(imageUpdate(QRect)));
        m_remoteApplications.insert(appUid, remoteApplication);
    }
    remoteApplication->updateGeometry(rect);
}

void RemoteConnection::handleRemoteLaunchUpdate(QString appUid, QString data)
{
    emit launchApplication(appUid, data);
}
