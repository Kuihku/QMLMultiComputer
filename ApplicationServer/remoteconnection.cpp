#include "remoteconnection.h"
#include "remoteconnectioninfo.h"
#include "message.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QMetaObject>

#include <QDebug>


RemoteConnection::RemoteConnection(Remote::Direction remoteDirection, QByteArray ip, QObject *parent) :
    QObject(parent),
    m_remoteDirection(remoteDirection),
    m_remoteSocket(new QTcpSocket(this)),
    m_ip(0)
{
    bool formatOk(false);
    m_ip = ip.toLong(&formatOk);
    QHostAddress remoteAddress(m_ip);
    if (formatOk && !remoteAddress.isNull()) {
        connect(m_remoteSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(m_remoteSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(m_remoteSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        m_remoteSocket->connectToHost(remoteAddress, REMOTE_PORT);
    }
    else {
        m_ip = 0;
        qWarning() << "RemoteConnection::RemoteConnection - error ip:" << ip << "- address:" << remoteAddress;
    }
}

RemoteConnection::RemoteConnection(QTcpSocket* socket, QObject *parent) :
    QObject(parent),
    m_remoteDirection(Remote::Undefined),
    m_remoteSocket(socket),
    m_ip(socket->peerAddress().toIPv4Address())
{
}

Remote::Direction RemoteConnection::remoteDirection() const
{
    return m_remoteDirection;
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
    qDebug() << "LocalConnection::socketReadyRead - bytes available:" << m_remoteSocket->bytesAvailable();
    Message* m(Message::read(m_remoteSocket));
    if (m) {
        switch (m->type()) {
            case MessageType::Undefined : qWarning("LocalConnection::socketReadyRead - message type: Undefined"); break;
            case MessageType::RemoteDirection : {
                RemoteDirectionMessage* rdm(dynamic_cast<RemoteDirectionMessage*>(m));
                handleRemoteDirection((Remote::Direction)rdm->remoteDirection());
                break;
            }
            default : {
                qDebug() << "LocalConnection::socketReadyRead - message:" << *m;
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
