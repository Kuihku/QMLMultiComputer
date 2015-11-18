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
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketBytesWritten(qint64)));
    connect(m_socket, SIGNAL(aboutToClose()), this, SLOT(socketAboutToClose()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(connectionClosed()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
}

QRect LocalConnection::geometry() const
{
    return m_geometry;
}

QMap<Remote::Direction, QImage> LocalConnection::paintImage(QPainter* painter)
{
    QRect viewPort(painter->viewport());
    qDebug() << "LocalConnection::paintImage - m_geometry:" << m_geometry << "- viewPort:" << viewPort;
    QRect outRect(viewPort.intersected(m_geometry));
    painter->drawImage(outRect, m_image.copy(outRect));
    QMap<Remote::Direction, QImage> extImgs;
    if (!viewPort.contains(m_geometry, true)) {
        // check right edge
        bool rightEdge(false);
        if ((m_geometry.x() + m_geometry.width()) > (viewPort.x() + viewPort.width())) {
            rightEdge = true;
            QImage img(m_image.copy((viewPort.width() + viewPort.x() - m_geometry.x()),
                                    0,
                                    (m_geometry.x() + m_geometry.width() - viewPort.width() - viewPort.x()),
                                    m_geometry.height()));
            extImgs.insert(Remote::East, img);
        }


        // check bottom edge
        if ((m_geometry.y() + m_geometry.height()) > (viewPort.y() + viewPort.height())) {
            extImgs.insert(Remote::South, m_image.copy(0,
                                                      (viewPort.height() + viewPort.y() - m_geometry.y()),
                                                      m_geometry.width(),
                                                      (m_geometry.y() + m_geometry.height() - viewPort.height() - viewPort.y())));

            if(rightEdge) {
                // check bottomright edge
                extImgs.insert(Remote::SouthEast, m_image.copy((viewPort.x() + viewPort.width() - m_geometry.x()),
                                                               (viewPort.y() + viewPort.height() - m_geometry.y()),
                                                               (m_geometry.x() + m_geometry.width() - viewPort.width() - viewPort.x()),
                                                               (m_geometry.y() + m_geometry.height() - viewPort.height() - viewPort.y())));
            }
        }

    }
    return extImgs;
}

QString LocalConnection::appUid() const
{
    return m_appUid;
}

void LocalConnection::cloneApplication()
{
    Message m(m_appUid, MessageType::CloneRequest);
    m.write(m_socket);
}

void LocalConnection::setProperties(CloneDataMessage *cdm)
{
    cdm->write(m_socket);
}

void LocalConnection::close()
{
    Message m(m_appUid, MessageType::Close);
    m.write(m_socket);
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
            case MessageType::CloneData : {
                qDebug() << "LocalConnection::socketReadyRead - message: CloneData";
                CloneDataMessage* cdm(dynamic_cast<CloneDataMessage*>(m));
                emit cloneDataAvailable(cdm);
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

void LocalConnection::socketBytesWritten(qint64 bytes)
{
    qDebug("LocalConnection::socketBytesWritten - bytes: %d", (int)bytes);
}

void LocalConnection::UpdateView(QString appUid)
{
    m_appUid = appUid;
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
    buffer.close();
    emit updateRequest(m_geometry);
}

void LocalConnection::setGeometry(QString appUid, QRect geometry)
{
    m_appUid = appUid;
    if (m_geometry != geometry) {
        qDebug() << "LocalConnection::setGeometry - geometry:" << geometry;
        m_geometry = geometry;
        emit geometryChanged(appUid, m_geometry);
    }
}
