#include "runnerview.h"

#include <QQmlEngine>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QImage>
#include <QDateTime>
#include <QDataStream>
#include <QSharedMemory>
#include <QBuffer>
#include <QPainter>
#include <QQuickItem>
#include <QMetaObject>
#include <QCoreApplication>

#include <QDebug>

RunnerView::RunnerView(QString appUid, QString server, QWidget* parent) :
    QQuickWidget(parent),
    m_appUid(appUid),
    m_socket(new QLocalSocket(this)),
    m_shared(new QSharedMemory(appUid, this))
{
    qDebug() << "RunnerView::RunnerView";
    QObject::connect(engine(), SIGNAL(quit()), this, SLOT(quitApplication()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketBytesWritten(qint64)));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    m_socket->connectToServer(server);
    qDebug() << "RunnerView::RunnerView - END";
}

void RunnerView::save()
{
    QImage image(grabFramebuffer());

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
                qWarning("RunnerView::save - m_shared create error");
                return;
            }

        }
        m_shared->lock();
        char* to((char*)m_shared->data());
        const char* from(buffer.data().data());
        memcpy(to, from, qMin(m_shared->size(), size));
        m_shared->unlock();

        Message message(m_appUid, MessageType::Update);
        message.write(m_socket);
    }
}

void RunnerView::socketError(QLocalSocket::LocalSocketError error)
{
    qDebug() << "RunnerView::socketError - error:" << error;
}

void RunnerView::socketConnected()
{
    qDebug() << "RunnerView::socketConnected";
}

void RunnerView::socketDisconnected()
{
    qDebug() << "RunnerView::socketDisconnected";
    m_shared->detach();
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

void RunnerView::socketBytesWritten(qint64 bytes)
{
    qDebug() << "RunnerView::socketBytesWritten - bytes:" << bytes;
}

void RunnerView::readSocket()
{
    qDebug() << "RunnerView::readSocket - bytes available:" << m_socket->bytesAvailable();
    Message* m(Message::read(m_socket));
    if (m) {
        switch (m->type()) {
            case MessageType::Undefined : qWarning("RunnerView::readSocket - message type: Undefined"); break;
            case MessageType::CloneRequest : {
                handleCloneRequest();
                break;
            }
            case MessageType::CloneData : {
                CloneDataMessage* cdm(dynamic_cast<CloneDataMessage*>(m));
                handleCloneData(cdm);
                break;
            }
            default : {
                qDebug() << "RunnerView::readSocket - message:" << *m;
                break;
            }
        }
        delete m;
    }

}

void RunnerView::quitApplication()
{
    m_socket->close();
}

void RunnerView::paintEvent(QPaintEvent* event)
{
    QQuickWidget::paintEvent(event);
    QMetaObject::invokeMethod(this, "save", Qt::QueuedConnection);
}

void RunnerView::resizeEvent(QResizeEvent *event)
{
    QQuickWidget::resizeEvent(event);
    GeometryMessage gm(m_appUid, geometry());
    gm.write(m_socket);
    update();
}

void RunnerView::moveEvent(QMoveEvent *event)
{
    QQuickWidget::moveEvent(event);
    GeometryMessage gm(m_appUid, geometry());
    gm.write(m_socket);
    update();
}

void RunnerView::handleCloneRequest()
{
    CloneDataMessage cdm(m_appUid);
    setItemToMessage(cdm, rootObject());
    cdm.write(m_socket);
}

void RunnerView::setItemToMessage(CloneDataMessage &cdm, QQuickItem *item, int index)
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

void RunnerView::handleCloneData(CloneDataMessage* cdm)
{
    setMessageToItem(cdm, rootObject());
}

void RunnerView::setMessageToItem(CloneDataMessage *cdm, QQuickItem* item, int index)
{
    QStringList indexProperties(cdm->properties(index));
    int indexPropertyCount(indexProperties.count());
    for (int i(0); i < indexPropertyCount; i++) {
        QString propertyName(indexProperties.at(i));
        item->setProperty(propertyName.toLatin1().constData(), cdm->indexPropertyValue(index, propertyName));
    }
    QList<QQuickItem*> childItemList(item->childItems());
    int childItemCount(childItemList.count());
    for (int i(0); i < childItemCount; i++) {
        QQuickItem* nextItem(childItemList.at(i));
        setMessageToItem(cdm, nextItem, ++index);
    }
}
