#include "server.h"
#include "localconnection.h"
#include "remoteconnection.h"
#include "mainview.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QFile>
#include <QPainter>

#include <QCoreApplication>

#include <QDebug>

Server::Server(QObject *parent) :
    QObject(parent),
    m_view(new MainView(this)),
    m_localServer(new QLocalServer(this))
{
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
    qDebug() << "Server::paintWindows - rect:" << rect << "- region:" << region;
//    qDebug("Server::paintWindows - painter: %p", painter);
//    Q_UNUSED(rect)
    int connectionCount(m_localConnections.count());
    for (int i(0); i < connectionCount; i++) {
        LocalConnection* localConnection(m_localConnections.at(i));
        if (region.contains(localConnection->geometry())) {
            localConnection->paintImage(painter);
        }
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
    LocalConnection* localConnection(qobject_cast<LocalConnection*>(sender()));
    if (localConnection) {
        qDebug() << "Server::localUpdate - local geometry:" << localConnection->geometry();
        m_view->update(localConnection->geometry());
    }
//    m_view->update();
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
            m_remoteConnections.insert(Server::NorthWest, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "north")) {
            m_remoteConnections.insert(Server::North, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "northeast")) {
            m_remoteConnections.insert(Server::NorthEast, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "west")) {
            m_remoteConnections.insert(Server::West, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "east")) {
            m_remoteConnections.insert(Server::East, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "southwest")) {
            m_remoteConnections.insert(Server::SouthWest, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "south")) {
            m_remoteConnections.insert(Server::South, new RemoteConnection(ip, this));
        }
        else if (qstricmp(direction, "southeast")) {
            m_remoteConnections.insert(Server::SouthEast, new RemoteConnection(ip, this));
        }
    }

    serverConfigFile.close();
}

