#include "server.h"
#include "localconnection.h"
#include "remoteconnection.h"
#include "mainview.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QPainter>
#include <QMapIterator>
#include <QNetworkInterface>

#include <QCoreApplication>

#include <QDebug>

Server::Server(QObject *parent) :
    QObject(parent),
    m_view(new MainView(this)),
    m_localServer(new QLocalServer(this)),
    m_remoteServer(new QTcpServer(this))
{
    QHostAddress myIP(myIPv4());
    if (!myIP.isNull()) {
        connect(m_remoteServer, SIGNAL(newConnection()), this, SLOT(newRemoteConnection()));
        if (!m_remoteServer->listen(myIP, REMOTE_PORT)) {
            qWarning() << "Server::Server - m_remoteServer listen error:" << m_remoteServer->errorString();
        }
    }
    parseConfigFile();
    m_view->showFullScreen();

    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(newLocalConnection()));

    if (!m_localServer->listen("QML1")) {
        qWarning() << "Server::Server - localserver listening error:" << m_localServer->errorString();
    }

}

Server::~Server()
{
    delete m_view;
}

void Server::paintWindows(QRect rect, QRegion region, QPainter* painter)
{
//    qDebug() << "Server::paintWindows - rect:" << rect << "- region:" << region;
//    qDebug("Server::paintWindows - painter: %p", painter);
//    Q_UNUSED(rect)


    int connectionCount(m_localConnections.count());
    for (int i(0); i < connectionCount; i++) {
        LocalConnection* localConnection(m_localConnections.at(i));
        if (region.contains(localConnection->geometry())) {
//            qDebug() << "Server::paintWindows";
            localConnection->paintImage(painter);
        }
    }
}

void Server::newRemoteConnection()
{
    qDebug("Server::newLocalConnection");
    while (m_remoteServer->hasPendingConnections()) {
        QTcpSocket* socket(m_remoteServer->nextPendingConnection());
        RemoteConnection* remoteConnection(new RemoteConnection(socket, this));
        m_unconnectedRemoteConnections.append(remoteConnection);
        connect(remoteConnection, SIGNAL(connectionReady()), this, SLOT(remoteConnectionReady()));
        connect(remoteConnection, SIGNAL(connectionClosed()), this, SLOT(remoteConnectionClosed()));
    }
}

void Server::remoteConnectionReady()
{
    RemoteConnection* remoteConnection(qobject_cast<RemoteConnection*>(sender()));
    if (remoteConnection) {
        m_unconnectedRemoteConnections.removeAll(remoteConnection);
        Remote::Direction remoteDirection(remoteConnection->remoteDirection());
        delete m_remoteConnections.value(remoteDirection, NULL);
        m_remoteConnections.insert(remoteDirection, remoteConnection);
    }
}

void Server::newLocalConnection()
{
    qDebug("Server::newLocalConnection");
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket* socket(m_localServer->nextPendingConnection());
        LocalConnection* localConnection(new LocalConnection(socket, this));
        m_localConnections.append(localConnection);
        connect(localConnection, SIGNAL(updateRequest()), this, SLOT(localUpdate()));
        connect(localConnection, SIGNAL(connectionClosed()), this, SLOT(localConnectionClosed()));
    }
}

void Server::localUpdate()
{
//    LocalConnection* localConnection(qobject_cast<LocalConnection*>(sender()));
//    qDebug("Server::localUpdate - localConnection: %p", localConnection);
//    if (localConnection) {
//        m_view->update(localConnection->geometry());
//    }
    m_view->update();
}

void Server::localConnectionClosed()
{
    LocalConnection* localConnection(qobject_cast<LocalConnection*>(sender()));
    if (localConnection) {
        m_view->update(localConnection->geometry());
        m_localConnections.removeAll(localConnection);
        localConnection->deleteLater();
    }
    if (m_localConnections.isEmpty()) {
        m_localServer->close();
        qApp->exit();
    }
}

QHostAddress Server::myIPv4()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress(QHostAddress::LocalHost))
            return address;
    }
    return QHostAddress();
}

void Server::parseConfigFile()
{
    QFile serverConfigFile(SERVER_CONFIG);
    if (!serverConfigFile.exists()) {
        qWarning("Missing config file: %s", SERVER_CONFIG);
        return;
    }

    if (!serverConfigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Opening config file: %s", SERVER_CONFIG);
        return;
    }

    while (!serverConfigFile.atEnd()) {
        QByteArray line(serverConfigFile.readLine());
        int doubleColonIndex(line.indexOf(":"));
        if (doubleColonIndex < 3) continue;
        QByteArray direction(line.left(doubleColonIndex));
        QByteArray ip(line.mid(doubleColonIndex + 1));

        if (qstricmp(direction, "northwest")) {
            m_remoteConnections.insert(Remote::NorthWest, new RemoteConnection(Remote::NorthWest, ip, this));
        }
        else if (qstricmp(direction, "north")) {
            m_remoteConnections.insert(Remote::North, new RemoteConnection(Remote::North, ip, this));
        }
        else if (qstricmp(direction, "northeast")) {
            m_remoteConnections.insert(Remote::NorthEast, new RemoteConnection(Remote::NorthEast, ip, this));
        }
        else if (qstricmp(direction, "west")) {
            m_remoteConnections.insert(Remote::West, new RemoteConnection(Remote::West, ip, this));
        }
        else if (qstricmp(direction, "east")) {
            m_remoteConnections.insert(Remote::East, new RemoteConnection(Remote::East, ip, this));
        }
        else if (qstricmp(direction, "southwest")) {
            m_remoteConnections.insert(Remote::SouthWest, new RemoteConnection(Remote::SouthWest, ip, this));
        }
        else if (qstricmp(direction, "south")) {
            m_remoteConnections.insert(Remote::South, new RemoteConnection(Remote::South, ip, this));
        }
        else if (qstricmp(direction, "southeast")) {
            m_remoteConnections.insert(Remote::SouthEast, new RemoteConnection(Remote::SouthEast, ip, this));
        }
    }

    serverConfigFile.close();
}

