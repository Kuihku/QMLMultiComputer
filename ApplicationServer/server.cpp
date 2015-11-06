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
    m_myIPv4 = myIPv4();
    if (!m_myIPv4.isNull()) {
        connect(m_remoteServer, SIGNAL(newConnection()), this, SLOT(newRemoteConnection()));
        if (!m_remoteServer->listen(m_myIPv4, REMOTE_PORT)) {
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
    Q_UNUSED(rect)

    QMapIterator<Remote::Direction, RemoteConnection*> remoteConnectionIterator(m_remoteConnections);
    while (remoteConnectionIterator.hasNext()) {
        remoteConnectionIterator.next();
        remoteConnectionIterator.value()->paintImages(region, painter);
    }

    int connectionCount(m_localConnections.count());
    for (int i(0); i < connectionCount; i++) {
        LocalConnection* localConnection(m_localConnections.at(i));
        if (region.contains(localConnection->geometry())) {
//            qDebug() << "Server::paintWindows";
            QMap<Remote::Direction, QImage> externalImages(localConnection->paintImage(painter));
            if (!externalImages.isEmpty()) {
                QString appUid(localConnection->appUid());
                QMapIterator<Remote::Direction, QImage> extImgIterator(externalImages);
                while (extImgIterator.hasNext()) {
                    extImgIterator.next();
                    RemoteConnection* remoteConnection(m_remoteConnections.value(extImgIterator.key(), NULL));
                    if (remoteConnection) {
                        remoteConnection->sendImage(appUid, extImgIterator.value());
                    }
                }
            }
        }
    }
}

void Server::newRemoteConnection()
{
    qDebug("Server::newLocalConnection");
    while (m_remoteServer->hasPendingConnections()) {
        QTcpSocket* socket(m_remoteServer->nextPendingConnection());
        RemoteConnection* remoteConnection(new RemoteConnection(m_myIPv4, socket, this));
        m_unconnectedRemoteConnections.append(remoteConnection);
        connect(remoteConnection, SIGNAL(connectionReady()), this, SLOT(remoteConnectionReady()));
        connect(remoteConnection, SIGNAL(connectionClosed()), this, SLOT(remoteConnectionClosed()));
        connect(remoteConnection, SIGNAL(imageUpdate(QRect)), this, SLOT(remoteUpdate(QRect)));

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

void Server::remoteUpdate(QRect geometry)
{
    m_view->update(geometry);
}

void Server::newLocalConnection()
{
    qDebug("Server::newLocalConnection");
    while (m_localServer->hasPendingConnections()) {
        QLocalSocket* socket(m_localServer->nextPendingConnection());
        LocalConnection* localConnection(new LocalConnection(socket, this));
        m_localConnections.append(localConnection);
        connect(localConnection, SIGNAL(updateRequest(QRect)), this, SLOT(localUpdate(QRect)));
        connect(localConnection, SIGNAL(geometryChanged(QString QRect)), this, SLOT(localGeometryChanged(QString, QRect)));
        connect(localConnection, SIGNAL(connectionClosed()), this, SLOT(localConnectionClosed()));
    }
}

void Server::localUpdate(QRect geometry)
{
    m_view->update(geometry);
}

void Server::localGeometryChanged(QString appUid, QRect geometry)
{
    QRect viewPort(m_view->rect());

    // check right edge
    bool rightEdge(false);
    if ((geometry.x() + geometry.width()) > (viewPort.x() + viewPort.width())) {
        rightEdge = true;

        RemoteConnection* remoteConnection(m_remoteConnections.value(Remote::East, NULL));
        if (remoteConnection) {
            remoteConnection->updateGeometry(appUid,
                                             0,
                                             geometry.y(),
                                             (geometry.x() - viewPort.x()),
                                             (viewPort.height() - geometry.y() + viewPort.y()));
        }
    }

    // check bottom edge
    if ((geometry.y() + geometry.height()) > (viewPort.y() + viewPort.height())) {
        RemoteConnection* remoteConnection(m_remoteConnections.value(Remote::South, NULL));
        if (remoteConnection) {
            remoteConnection->updateGeometry(appUid,
                                             geometry.x(),
                                             0,
                                             (viewPort.width() - geometry.x() + viewPort.x()),
                                             (geometry.y() - viewPort.y()));
        }

        if(rightEdge) {
            // check bottomright edge
            RemoteConnection* remoteConnection(m_remoteConnections.value(Remote::SouthEast, NULL));
            if (remoteConnection) {
                remoteConnection->updateGeometry(appUid,
                                                 0,
                                                 0,
                                                 (geometry.x() - viewPort.x()),
                                                 (geometry.y() - viewPort.y()));
            }
        }
    }
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
            m_remoteConnections.insert(Remote::NorthWest, new RemoteConnection(m_myIPv4, Remote::NorthWest, ip, this));
        }
        else if (qstricmp(direction, "north")) {
            m_remoteConnections.insert(Remote::North, new RemoteConnection(m_myIPv4, Remote::North, ip, this));
        }
        else if (qstricmp(direction, "northeast")) {
            m_remoteConnections.insert(Remote::NorthEast, new RemoteConnection(m_myIPv4, Remote::NorthEast, ip, this));
        }
        else if (qstricmp(direction, "west")) {
            m_remoteConnections.insert(Remote::West, new RemoteConnection(m_myIPv4, Remote::West, ip, this));
        }
        else if (qstricmp(direction, "east")) {
            m_remoteConnections.insert(Remote::East, new RemoteConnection(m_myIPv4, Remote::East, ip, this));
        }
        else if (qstricmp(direction, "southwest")) {
            m_remoteConnections.insert(Remote::SouthWest, new RemoteConnection(m_myIPv4, Remote::SouthWest, ip, this));
        }
        else if (qstricmp(direction, "south")) {
            m_remoteConnections.insert(Remote::South, new RemoteConnection(m_myIPv4, Remote::South, ip, this));
        }
        else if (qstricmp(direction, "southeast")) {
            m_remoteConnections.insert(Remote::SouthEast, new RemoteConnection(m_myIPv4, Remote::SouthEast, ip, this));
        }
    }

    serverConfigFile.close();
}

