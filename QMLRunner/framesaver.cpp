#include "framesaver.h"

#include <QQuickView>
#include <QImage>
#include <QDateTime>
#include <QDataStream>
#include <QSharedMemory>
#include <QBuffer>
#include <QPainter>
#include <QQuickItem>

#include <QDebug>

FrameSaver::FrameSaver(QString appUid, QString server, QQuickView* view, QObject *parent) :
    QObject(parent),
    m_appUid(appUid),
    m_view(view),
    m_socket(new QLocalSocket(this)),
    m_shared(new QSharedMemory(appUid, this))
{
    qDebug() << "FrameSaver::FrameSaver";
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketBytesWritten(qint64)));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    m_socket->connectToServer(server);
    connect(view, SIGNAL(frameSwapped()), this, SLOT(save()));
    connect(view, SIGNAL(widthChanged(int)), this, SLOT(viewGeometryChanged()));
    connect(view, SIGNAL(heightChanged(int)), this, SLOT(viewGeometryChanged()));
    connect(view, SIGNAL(xChanged(int)), this, SLOT(viewGeometryChanged()));
    connect(view, SIGNAL(yChanged(int)), this, SLOT(viewGeometryChanged()));
    qDebug() << "FrameSaver::FrameSaver - END";
}

void FrameSaver::save()
{
    QImage image(m_view->grabWindow());

//    if (!m_shared->isAttached()) {
//        m_shared->create()
//    }


    if (m_socket->isWritable()) {
        QBuffer buffer;
        buffer.open(QBuffer::WriteOnly);
        QDataStream out(&buffer);
        out << image;
        int size(buffer.size());

        if (m_shared->isAttached()) {
            if (m_shared->size() < size && !m_shared->detach()) {
                return;
            }
        }
        if (!m_shared->isAttached()) {
            if (!m_shared->create(size)) {
                qWarning("FrameSaver::save - m_shared create error");
                return;
            }

        }
        m_shared->lock();
        char* to((char*)m_shared->data());
        const char* from(buffer.data().data());
        memcpy(to, from, qMin(m_shared->size(), size));
        m_shared->unlock();

//        QPainter p(&image);
//        p.drawText(10, 20, QString::number(realSize));

        Message message(m_appUid, MessageType::Update);
        message.write(m_socket);

//        QDataStream pingOut(m_socket);
//        pingOut << (int)1;
        // qDebug() << "FrameSaver::save";
    }

//     QString fileName(QDateTime::currentDateTime().toString("yyyy.MM.dd.HH.mm.ss.zzz.png"));
//     fileName.prepend("images/");
//     qDebug() << "FrameSaver::save - fileName:" << fileName.toLatin1().constData();
//    if (!image.save(fileName, "PNG")) {
//        qDebug() << "FrameSaver::save - Error, fileName:" << fileName.toLatin1().constData();
    //    }
}

void FrameSaver::socketError(QLocalSocket::LocalSocketError error)
{
    qDebug() << "FrameSaver::socketError - error:" << error;
}

void FrameSaver::socketConnected()
{
    qDebug() << "FrameSaver::socketConnected";
}

void FrameSaver::socketDisconnected()
{
    qDebug() << "FrameSaver::socketDisconnected";
    m_shared->detach();
}

void FrameSaver::socketBytesWritten(qint64 bytes)
{
    qDebug() << "FrameSaver::socketBytesWritten - bytes:" << bytes;
}

void FrameSaver::viewGeometryChanged()
{
    GeometryMessage gm(m_appUid, m_view->geometry());
    gm.write(m_socket);
}

void FrameSaver::readSocket()
{
    qDebug() << "FrameSaver::readSocket - bytes available:" << m_socket->bytesAvailable();
    Message* m(Message::read(m_socket));
    if (m) {
        switch (m->type()) {
            case MessageType::Undefined : qWarning("FrameSaver::readSocket - message type: Undefined"); break;
            case MessageType::CloneRequest : {
                handleCloneRequest();
                break;
            }
            default : {
                qDebug() << "FrameSaver::readSocket - message:" << *m;
                break;
            }
        }
        delete m;
    }

}

void FrameSaver::handleCloneRequest()
{
    CloneDataMessage cdm(m_appUid);
    setItemToMessage(cdm, m_view->rootObject());
    cdm.write(m_socket);
}

void FrameSaver::setItemToMessage(CloneDataMessage &cdm, QQuickItem *item, int index)
{
    QList<QByteArray> properties(item->dynamicPropertyNames());
    const QMetaObject* metaObject(item->metaObject());
    int metaObjectPropertyCount(metaObject->propertyCount());
    for (int i(metaObject->propertyOffset()); i < metaObjectPropertyCount; i++) {
        properties.append(QByteArray(metaObject->property(i).name()));
    }
    int propertyCount(properties.count());
    for (int i(0); i < propertyCount; i++) {
        QByteArray propertyName(properties.at(i));
        cdm.setIndexPropertyValue(index, propertyName, item->property(propertyName.constData()));
    }

    QList<QQuickItem*> childItemList(item->childItems());
    int childItemCount(childItemList.count());
    for (int i(0); i < childItemCount; i++) {
        QQuickItem* nextItem(childItemList.at(i));
        setItemToMessage(cdm, nextItem, ++index);
    }
}
