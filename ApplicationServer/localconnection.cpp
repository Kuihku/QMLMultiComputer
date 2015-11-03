#include "localconnection.h"
#include "message.h"

#include <QSharedMemory>
#include <QBuffer>

#include <QDebug>

LocalConnection::LocalConnection(QLocalSocket* socket, QObject* parent) :
    QObject(parent),
    m_socket(socket),
    m_shared(NULL)
{
    connect(m_socket, SIGNAL(aboutToClose()), this, SLOT(socketAboutToClose()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
}

void LocalConnection::socketAboutToClose()
{
    qDebug("LocalConnection::socketAboutToClose");
}

void LocalConnection::socketReadyRead()
{
    Message* m(Message::read(m_socket));
    if (m) {
        switch (m->type()) {
        case MessageType::Undefined : qWarning("LocalConnection::socketReadyRead - message type: Undefined"); break;
        case MessageType::Update : UpdateView(m->appUid()); break;
        default :
            qDebug() << "LocalConnection::socketReadyRead - message:" << *m;
            break;
        }
    }
}

void LocalConnection::socketError(QLocalSocket::LocalSocketError error)
{
    qWarning() << "LocalConnection::socketError - error:" << error;
}

void LocalConnection::UpdateView(QString appUid)
{
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
    if (!m_shared->lock()) {
        qWarning() << "LocalConnection::UpdateView - shared memory lock error:" << m_shared->errorString();
        return;
    }

    const char* from((const char*)m_shared->data());
    QBuffer buffer;
    buffer.setData(from);
    m_shared->unlock();

}
