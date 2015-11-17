#include "server.h"
#include "localconnection.h"
#include "remoteconnection.h"
#include "mainview.h"
#include "message.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QPainter>
#include <QMapIterator>
#include <QNetworkInterface>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>

#include <QDebug>

#define LOCALSERVERNAME "QML1"

Server::Server(QString configFile, QObject *parent) :
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
    parseConfigFile(configFile);
    m_view->showFullScreen();

    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(newLocalConnection()));

    if (!m_localServer->listen(LOCALSERVERNAME)) {
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
    qDebug("Server::newRemoteConnection");
    while (m_remoteServer->hasPendingConnections()) {
        QTcpSocket* socket(m_remoteServer->nextPendingConnection());
        RemoteConnection* remoteConnection(new RemoteConnection(m_myIPv4, socket, this));
        setupRemoteConnection(remoteConnection);
        m_unconnectedRemoteConnections.append(remoteConnection);
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

void Server::remoteConnectionClosed()
{
    RemoteConnection* remoteConnection(qobject_cast<RemoteConnection*>(sender()));
    if (remoteConnection) {
        Remote::Direction remoteDirection(m_remoteConnections.key(remoteConnection, Remote::Undefined));
        if (remoteDirection != Remote::Undefined) {
            m_remoteConnections.remove(remoteDirection);
            delete remoteConnection;
        }
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
        connect(localConnection, SIGNAL(geometryChanged(QString, QRect)), this, SLOT(localGeometryChanged(QString, QRect)));
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

    int x(geometry.x());
    int y(geometry.y());
    int width(viewPort.width());
    int height(viewPort.height());

    if (x > width || x < 0 || y > height || y < 0) {

        Remote::Direction launchDirection(Remote::Undefined);

        if (x < 0) {
            if (y < 0) launchDirection = Remote::NorthWest;
            else if (y > height) launchDirection = Remote::SouthWest;
            else launchDirection = Remote::West;
        }
        else if (x > width) {
            if (y < 0) launchDirection = Remote::NorthEast;
            else if (y > height) launchDirection = Remote::SouthEast;
            else launchDirection = Remote::East;
        }
        else {
            if (y < 0) launchDirection = Remote::North;
            else if (y > height) launchDirection = Remote::South;
        }

        RemoteConnection* remoteConnection(m_remoteConnections.value(launchDirection, NULL));

        if (remoteConnection) {
            remoteConnection->ReguestApplicationLaunch(appUid, QString());
            LocalConnection* localConnection(qobject_cast<LocalConnection*>(sender()));
            if (localConnection) {
                connect(localConnection, SIGNAL(cloneDataAvailable(CloneDataMessage*)), remoteConnection, SLOT(localCloneDataAvailable(CloneDataMessage*)), Qt::DirectConnection);
                localConnection->cloneApplication();
                localConnection->close();
            }
            else {
                qWarning() << "Server::localGeometryChanged - Error no localConnection, appUid:" << appUid << " geometry:" <<  geometry.topLeft() << "> viewport:" << viewPort.size();
            }
        }
    }
    else {

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
        qDebug("Server::localConnectionClosed - last connection closed");
//        m_localServer->close();
//        m_remoteServer->close();
//        qApp->exit();
    }
}

void Server::launchApplication(QString appUid, QString data)
{
    Q_UNUSED(data)

    int localConnectionsCount(m_localConnections.count());
    for (int i(0); i < localConnectionsCount; i++) {
        if (m_localConnections.at(i)->appUid().compare(appUid) == 0) {
            // TODO: clone with new appUid
            qWarning() << "Server::launchApplication - Error: trying to launch application that already exists, appUid:" << appUid;
            return;
        }
    }

    QString applicationPath(APPLICATION_PATH);
    applicationPath.append("/");
    applicationPath.append(appUid);
    QDir applicationDir(applicationPath);
    if (applicationDir.exists(applicationPath)) {
        if (!QProcess::startDetached(QMLRUNNEREXE, QStringList() << appUid << applicationPath << "main.qml" << LOCALSERVERNAME, applicationPath)) {
            qWarning() << "Server::launchApplication - Error in QMLRunner QProcess launch, appUid:" << appUid;
        }
    }
    else {
        RemoteConnection* remoteConnection(qobject_cast<RemoteConnection*>(sender()));
        if (remoteConnection) {
            remoteConnection->getApplication(appUid);
        }
        else {
            qWarning() << "Server::launchApplication - Error in new QMLRunner launch, remoteConnection sender not found for appUid:" << appUid;
        }
    }
}

void Server::cloneApplicationReceived(CloneDataMessage* cdm)
{
    int localConnectionCount(m_localConnections.count());
    QString appUid(cdm->appUid());
    for (int i(0); i < localConnectionCount; i++) {
        LocalConnection* localConnection(m_localConnections.at(i));
        if (localConnection->appUid().compare(appUid) == 0) {
            localConnection->setProperties(cdm);
            return;
        }
    }
}

void Server::applicationReceived(RemoteApplicationMessage *ram)
{
    QString appUid(ram->appUid());
    QString applicationPath(APPLICATION_PATH);
    applicationPath.append("/");
    applicationPath.append(appUid);
    QStringList files(ram->files());
    int fileCount(files.count());
    for (int i(0); i < fileCount; i++) {
        QString fileName(files.at(i));
        QString fullPath(applicationPath);
        fullPath.append("/");
        fullPath.append(fileName);
        QFile f(fullPath);
        if (!f.open(QIODevice::WriteOnly)) {
            qWarning() << "Server::applicationReceived - Error opening file:" << fullPath << "- code:" << f.errorString();
            continue;
        }
        f.write(ram->fileData(fileName));
        f.flush();
        f.close();
    }
    QMetaObject::invokeMethod(this, "launchApplication", Qt::QueuedConnection, Q_ARG(QString, appUid), Q_ARG(QString, QString()));
}

QHostAddress Server::myIPv4()
{
    QHostAddress myAddress;
#ifdef MYIPADDRESS
    myAddress = QHostAddress(MYIPADDRESS);
#else
    QList<QHostAddress> allAddresses(QNetworkInterface::allAddresses());
    int allAddressesCount(allAddresses.count());
    for (int i(0); i < allAddressesCount && myAddress.isNull(); i++) {
        QHostAddress address(allAddresses.at(i));
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress(QHostAddress::LocalHost)) {
            myAddress = address;
        }
    }
#endif
    qDebug() << "Server::myIPv4 - myAddress:" << myAddress;
    return myAddress;
}

void Server::parseConfigFile(QString configFile)
{
    QFile serverConfigFile(configFile);
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
        QByteArray ip(line.mid(doubleColonIndex + 1).simplified());

        Remote::Direction remoteDirection(Remote::Undefined);

        if (qstricmp(direction, "northwest") == 0) {
            remoteDirection = Remote::NorthWest;
        }
        else if (qstricmp(direction, "north") == 0) {
            remoteDirection = Remote::North;
        }
        else if (qstricmp(direction, "northeast") == 0) {
            remoteDirection = Remote::NorthEast;
        }
        else if (qstricmp(direction, "west") == 0) {
            remoteDirection = Remote::West;
        }
        else if (qstricmp(direction, "east") == 0) {
            remoteDirection = Remote::East;
        }
        else if (qstricmp(direction, "southwest") == 0) {
            remoteDirection = Remote::SouthWest;
        }
        else if (qstricmp(direction, "south") == 0) {
            remoteDirection = Remote::South;
        }
        else if (qstricmp(direction, "southeast") == 0) {
            remoteDirection = Remote::SouthEast;
        }

        qDebug() << "Server::parseConfigFile - remoteDirection:" << remoteDirection << "- direction:" << direction;

        if (remoteDirection != Remote::Undefined) {
            RemoteConnection* remoteConnection(new RemoteConnection(m_myIPv4, remoteDirection, ip, this));
            setupRemoteConnection(remoteConnection);
            m_remoteConnections.insert(remoteDirection, remoteConnection);
        }

    }
    serverConfigFile.close();
}

void Server::setupRemoteConnection(RemoteConnection* remoteConnection)
{
    connect(remoteConnection, SIGNAL(connectionReady()), this, SLOT(remoteConnectionReady()));
    connect(remoteConnection, SIGNAL(connectionClosed()), this, SLOT(remoteConnectionClosed()));
    connect(remoteConnection, SIGNAL(imageUpdate(QRect)), this, SLOT(remoteUpdate(QRect)));
    connect(remoteConnection, SIGNAL(launchApplication(QString, QString)), this, SLOT(launchApplication(QString, QString)));
    connect(remoteConnection, SIGNAL(cloneApplicationReceived(CloneDataMessage*)), this, SLOT(cloneApplicationReceived(CloneDataMessage*)), Qt::DirectConnection);
    connect(remoteConnection, SIGNAL(applicationReceived(class RemoteApplicationMessage*)), this, SLOT(applicationReceived(class RemoteApplicationMessage*)), Qt::DirectConnection);

}

