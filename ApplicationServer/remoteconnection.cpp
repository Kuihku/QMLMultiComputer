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
#include <QDir>

#include <QDebug>

#define FIRSTREMOTEAPPPORT 49152
#define LASTREMOTEAPPPORT 65534

RemoteConnection::RemoteConnection(QHostAddress myIPv4, Remote::Direction remoteDirection, QByteArray ip, QObject *parent) :
    QObject(parent),
    m_nextUdpPort(FIRSTREMOTEAPPPORT),
    m_myIPv4(myIPv4),
    m_remoteDirection(remoteDirection),
    m_remoteSocket(new QTcpSocket(this))
{
    QHostAddress remoteAddress(QString::fromLatin1(ip));
    if (remoteAddress.isNull()) {
        qWarning() << "RemoteConnection::RemoteConnection - error ip:" << ip << "- address:" << remoteAddress;
    }
    else {
        setupRemoteSocket();
        m_remoteSocket->connectToHost(remoteAddress, REMOTE_PORT);
    }
}

RemoteConnection::RemoteConnection(QHostAddress myIPv4, QTcpSocket* socket, QObject *parent) :
    QObject(parent),
    m_nextUdpPort(FIRSTREMOTEAPPPORT),
    m_myIPv4(myIPv4),
    m_remoteDirection(Remote::Undefined),
    m_remoteSocket(socket)
{
    m_remoteSocket->setParent(this);
    setupRemoteSocket();
}

RemoteConnection::~RemoteConnection()
{
    m_remoteSocket->close();
    qDebug("RemoteConnection::~RemoteConnection: %p, m_remoteSocket: %p ", this, m_remoteSocket);
}

Remote::Direction RemoteConnection::remoteDirection() const
{
    return m_remoteDirection;
}

void RemoteConnection::updateGeometry(QString appUid, int x, int y, int width, int height)
{
    GeometryMessage gm(appUid, x, y, width, height);
    qDebug() << "RemoteConnection::updateGeometry - geometry:" << gm.geometry();
    gm.write(m_remoteSocket);
}

void RemoteConnection::sendImage(QString appUid, const QImage& image)
{
    RemoteApplication* remoteApplication(m_remoteApplications.value(appUid, NULL));
    qDebug("RemoteConnection::sendImage - m_remoteDirection: %s, appUid: %s, remoteApplication: %p", REMOTETOSTRING(m_remoteDirection), qPrintable(appUid), remoteApplication);
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

void RemoteConnection::ReguestApplicationLaunch(QString appUid, QString data)
{
    RemoteLaunchMessage rlm(appUid, data);
    rlm.write(m_remoteSocket);
}

void RemoteConnection::getApplication(QString appUid)
{
    Message rgam(appUid, MessageType::RemoteGetApplication);
    rgam.write(m_remoteSocket);
}

void RemoteConnection::localCloneDataAvailable(CloneDataMessage* cdm)
{
    cdm->write(m_remoteSocket);
}

void RemoteConnection::socketConnected()
{
    qDebug("RemoteConnection::socketConnected");
    RemoteDirectionMessage rdm(QString(), m_remoteDirection);
    Q_ASSERT(rdm.write(m_remoteSocket));
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
            case MessageType::Geometry: {
                GeometryMessage* gm(dynamic_cast<GeometryMessage*>(m));
                handleGeometryUpdate(gm->appUid(), gm->geometry());
            break;
            }
            case MessageType::RemoteLaunch: {
                RemoteLaunchMessage* rlm(dynamic_cast<RemoteLaunchMessage*>(m));
                emit launchApplication(rlm->appUid(), rlm->data());
            }
            case MessageType::CloneData : {
                CloneDataMessage* cdm(dynamic_cast<CloneDataMessage*>(m));
                emit cloneApplicationReceived(cdm);
                break;
            }
            case MessageType::RemoteGetApplication : {
                handleApplicationRequest(m->appUid());
                break;
            }
            case MessageType::RemotePort : {
                RemotePortMessage* rpm(dynamic_cast<RemotePortMessage*>(m));
                handleRemotePort(rpm->appUid(), rpm->port());
                break;
            }
            case MessageType::RemoteApplication : {
                RemoteApplicationMessage* ram(dynamic_cast<RemoteApplicationMessage*>(m));
                applicationReceived(ram);
                break;
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

void RemoteConnection::handleRemoteDirection(Remote::Direction remoteDirection)
{
    qDebug("RemoteConnection::handleRemoteDirection - remoteDirection: %s", qPrintable(REMOTETOSTRING(remoteDirection)));
    if (remoteDirection == Remote::NorthWest) {
        m_remoteDirection = Remote::SouthEast;
    }
    else if (remoteDirection == Remote::North) {
        m_remoteDirection = Remote::South;
    }
//    else if (remoteDirection == Remote::NorthEast) {
//        m_remoteDirection = Remote::SouthWest;
//    }
    else if (remoteDirection == Remote::West) {
        m_remoteDirection = Remote::East;
    }
    else if (remoteDirection == Remote::East) {
        m_remoteDirection = Remote::West;
    }
//    else if (remoteDirection == Remote::SouthWest) {
//        m_remoteDirection = Remote::NorthEast;
//    }
    else if (remoteDirection == Remote::South) {
        m_remoteDirection = Remote::North;
    }
    else if (remoteDirection == Remote::SouthEast) {
        m_remoteDirection = Remote::NorthWest;
    }
    emit connectionReady();
}

void RemoteConnection::handleGeometryUpdate(QString appUid, QRect rect)
{
    RemoteApplication* remoteApplication(m_remoteApplications.value(appUid, NULL));
    qDebug("RemoteConnection::handleGeometryUpdate - m_remoteDirection: %s, appUid: %s, remoteApplication: %p", REMOTETOSTRING(m_remoteDirection), qPrintable(appUid), remoteApplication);
    qDebug() << "RemoteConnection::handleGeometryUpdate - rect:" << rect;
    if (!remoteApplication) {
        remoteApplication = new RemoteApplication(m_myIPv4, m_nextUdpPort++, this);
        if (m_nextUdpPort > LASTREMOTEAPPPORT) {
            m_nextUdpPort = FIRSTREMOTEAPPPORT;
        }
        connect(remoteApplication, SIGNAL(imageUpdate(QRect)), this, SIGNAL(imageUpdate(QRect)));
        m_remoteApplications.insert(appUid, remoteApplication);
        RemotePortMessage rpm(appUid, remoteApplication->port());
        rpm.write(m_remoteSocket);
    }
    remoteApplication->updateGeometry(rect);
}

void RemoteConnection::handleApplicationRequest(QString appUid)
{
    QString applicationPath(APPLICATION_PATH);
    applicationPath.append("/");
    applicationPath.append(appUid);
    QDir applicationDir(applicationPath);
    if (applicationDir.exists()) {
        RemoteApplicationMessage ram(appUid);
        QStringList files(applicationDir.entryList(QDir::Files));
        int fileCount(files.count());
        for (int i(0); i < fileCount; i++) {
            QString fileName(files.at(i));
            QString fullPath(applicationPath);
            fullPath.append("/");
            fullPath.append(fileName);
            QFile f(fullPath);
            if (!f.open(QIODevice::ReadOnly)) {
                qWarning() << "RemoteConnection::remoteApplicationRequest - Error opening file:" << fullPath << "- code:" << f.errorString();
                continue;
            }
            ram.setFileData(fileName, f.readAll());
            f.close();
        }
        ram.write(m_remoteSocket);
    }
}

void RemoteConnection::handleRemotePort(QString appUid, int port)
{
    RemoteApplication* remoteApplication(m_remoteApplications.value(appUid, NULL));
    qDebug("RemoteConnection::handleRemotePort - m_remoteDirection: %s, appUid: %s, remoteApplication: %p", REMOTETOSTRING(m_remoteDirection), qPrintable(appUid), remoteApplication);
    if (!remoteApplication) {
        remoteApplication = new RemoteApplication(m_remoteSocket->peerAddress(), port, this);
        m_remoteApplications.insert(appUid, remoteApplication);
    }
}

void RemoteConnection::setupRemoteSocket()
{
    connect(m_remoteSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_remoteSocket, SIGNAL(disconnected()), this, SIGNAL(connectionClosed()));
    connect(m_remoteSocket, SIGNAL(readyRead()), this, SLOT(readSocket()));
}

