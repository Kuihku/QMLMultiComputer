#include "localconnection.h"
#include "message.h"

#include <QSharedMemory>
#include <QBuffer>
#include <QPainter>

#include <QDebug>

LocalConnection::LocalConnection(QLocalSocket* socket, QObject* parent) :
    QObject(parent),
    m_socket(socket),
    m_shared(NULL)
{
    connect(m_socket, SIGNAL(aboutToClose()), this, SLOT(socketAboutToClose()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(connectionClosed()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
}

QRect LocalConnection::geometry() const
{
    return m_geometry;
}

void LocalConnection::paintImage(QPainter* painter)
{
    qDebug() << "LocalConnection::paintImage - m_geometry:" << m_geometry;
    painter->drawImage(m_geometry, m_image);
}

void LocalConnection::socketAboutToClose()
{
    qDebug("LocalConnection::socketAboutToClose");
}

void LocalConnection::socketReadyRead()
{
    qDebug() << "LocalConnection::socketReadyRead - bytes available:" << m_socket->bytesAvailable();
    Message* m(Message::read(m_socket));
    if (m) {
        switch (m->type()) {
            case MessageType::Undefined : qWarning("LocalConnection::socketReadyRead - message type: Undefined"); break;
            case MessageType::Update : UpdateView(m->appUid()); break;
            case MessageType::Geometry : {
                qDebug() << "LocalConnection::socketReadyRead - message: Geometry";
                GeometryMessage* gm(dynamic_cast<GeometryMessage*>(m));
                setGeometry(gm->appUid(), gm->geometry());
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

void LocalConnection::socketError(QLocalSocket::LocalSocketError error)
{
    qWarning() << "LocalConnection::socketError - error:" << error;
}

void LocalConnection::UpdateView(QString appUid)
{
    qDebug() << "LocalConnection::UpdateView - appUid:" << appUid;
    if (!m_shared) {
        m_shared = new QSharedMemory(appUid, this);
    }
//    if (m_shared->isAttached()) {
//        m_shared->detach();
//    }
    if (!m_shared->attach(QSharedMemory::ReadOnly)) {
        qWarning() << "LocalConnection::UpdateView - shared memory attach error:" << m_shared->errorString();
        return;
    }

    QBuffer buffer;
    if (!m_shared->lock()) {
        qWarning() << "LocalConnection::UpdateView - shared memory lock error:" << m_shared->errorString();
        return;
    }
    buffer.setData((const char*)m_shared->constData(), m_shared->size());
    buffer.open(QBuffer::ReadOnly);
    QDataStream in(&buffer);
    in >> m_image;
    m_shared->unlock();
    m_shared->detach();
    emit updateRequest();
}

void LocalConnection::setGeometry(QString appUid, QRect geometry)
{
    Q_UNUSED(appUid)
    if (m_geometry != geometry) {
        qDebug() << "LocalConnection::setGeometry - geometry:" << geometry;
        m_geometry = geometry;
        emit updateRequest();
    }
}
