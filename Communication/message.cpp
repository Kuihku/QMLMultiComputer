#include "message.h"

#include <QIODevice>
#include <QDataStream>
#include <QThread>

#include <QDebug>


// Message

#define MSGTYPETOSTRING(type) \
    QString(type == Undefined ? "Undefined" :\
    type == Update ? "Update" :\
    type == Launch ? "Launch" :\
    type == Move ? "Move" :\
    type == Size ? "Size" :\
    type == Close ? "Close" : "Type unknown")

Message::Message(quint32 appUid, int messageType, QObject* parent) :
    QObject(parent),
    m_appUid(appUid),
    m_messageType(messageType)
{
}

quint32 Message::appUid() const
{
    return m_appUid;
}

int Message::type() const
{
    return m_messageType;
}

Message* Message::read(QIODevice* socket)
{
    Message* msg(NULL);
    if (socket &&
        socket->isReadable() &&
        socket->bytesAvailable() > (int)sizeof(quint32)) {
        QDataStream ds(socket);
        qint32 messageType;
        quint32 dataSize;
        QString appUid;
        ds >> dataSize;
        qint64 bytesAvailable(socket->bytesAvailable());
        if (bytesAvailable < dataSize) return msg;
        ds >> messageType >> appUid;
        switch (messageType) {
        case Update : {
            if (bytesAvailable > 0) {
                msg = new UpdateMessage(appUid, ds);
            }
            break;
        }
        case Launch : {
            if (bytesAvailable > 0) {
                msg = new LaunchMessage(appUid, ds);
            }
            break;
        }
        case Move : {
            if (bytesAvailable >= (qint64)(2 * sizeof(qint32))) {
                msg = new MoveMessage(appUid, ds);
            }
            break;
        }
        case Size : {
            if (bytesAvailable >= (qint64)(2 * sizeof(qint32))) {
                msg = new SizeMessage(appUid, ds);
            }
            break;
        }
        default : {
//            socket->readAll();
        }
        }

        if (ds.status() != QDataStream::Ok) {
            delete msg;
            msg = NULL;
        }
    }
    return msg;
}

bool Message::write(QIODevice* socket)
{
    if (socket &&
        socket->isWritable() &&
        m_messageType != Undefined) {
        QByteArray data;
        QDataStream ds(&data, QIODevice::WriteOnly);
        ds << (quint32)0 << (qint32)m_messageType << m_appUid;
        writeData(ds);
        ds.device()->seek(0);
        ds << (quint32)(data.size() - sizeof(quint32));
        socket->write(data);
        return true;
    }
    return false;
}

void Message::writeData(QDataStream& ds)
{
    Q_UNUSED(ds);
}

QDebug operator<<(QDebug d, const Message& m)
{
    return d << QString("Message(appId: %1, type: %2)").arg(m.appUid()).arg(MSGTYPETOSTRING(m.type())).toLocal8Bit().data();
}


// UpdateMessage

UpdateMessage::UpdateMessage(quint32 appUid, const QString &data, QObject *parent) :
    Message(appUid, Update, parent)
{

}

UpdateMessage::UpdateMessage(quint32 appUid, QDataStream &ds) :
    Message(appUid, Update, NULL)
{

}

UpdateMessage::UpdateMessage(const UpdateMessage& other)
{

}

QDebug operator<<(QDebug d, const UpdateMessage& m)
{
    return d << QString("UpdateMessage(appId: %1, type: %2)").arg(m.appUid()).arg(MSGTYPETOSTRING(m.type())).toLocal8Bit().data();
}


// LaunchMessage

LaunchMessage::LaunchMessage(const QString& data, QObject* parent) :
    Message(appUid, Launch, parent),
    m_data(data)
{
}

LaunchMessage::LaunchMessage(quint32 appUid, QDataStream& ds) :
    Message(appUid, Launch, NULL)
{
    ds >> m_data;
}

LaunchMessage::LaunchMessage(const LaunchMessage& other) :
    Message(other.m_appUid, other.m_messageType, other.parent()),
    m_data(other.m_data)
{
}

void LaunchMessage::writeData(QDataStream& ds)
{
    ds << m_data;
}

QString LaunchMessage::data() const
{
    return m_data;
}

void LaunchMessage::setData(QString data)
{
    m_data = data;
}

QDebug operator<<(QDebug d, const LaunchMessage& lm)
{
    return d << QString("LaunchMessage(appId: %1, data: %2)").arg(lm.appUid()).arg(lm.data()).toLocal8Bit().data();
}

// MoveMessage

MoveMessage::MoveMessage(quint32 appUid, int x, int y, QObject* parent) :
    Message(appUid, Move, parent),
    m_move(x, y)
{
}

MoveMessage::MoveMessage(quint32 appUid, QDataStream& ds) :
    Message(appUid, Move, NULL)
{
    qint32 x, y;
    ds >> x >> y;
    m_move.setX(x);
    m_move.setY(y);
}

MoveMessage::MoveMessage(const MoveMessage& other) :
    Message(other.m_appUid, other.m_messageType, other.parent()),
    m_move(other.m_move)
{
}

void MoveMessage::writeData(QDataStream& ds)
{
    ds << (qint32)m_move.x() << (qint32)m_move.y();
}

QPoint MoveMessage::topLeft() const
{
    return m_move;
}

int MoveMessage::x() const
{
    return m_move.x();
}

int MoveMessage::y() const
{
    return m_move.y();
}

void MoveMessage::setX(int x)
{
    m_move.setX(x);
}

void MoveMessage::setY(int y)
{
    m_move.setY(y);
}

QDebug operator<<(QDebug d, const MoveMessage& mm)
{
    return d << QString("MoveMessage(appId: %1, topLeft:(%2, %3))").arg(mm.appUid()).arg(mm.x()).arg(mm.y()).toLocal8Bit().data();
}

// SizeMessage

SizeMessage::SizeMessage(quint32 appUid, int width, int height, QObject* parent) :
    Message(appUid, Size, parent),
    m_size(width, height)
{
}

SizeMessage::SizeMessage(quint32 appUid, QDataStream& ds) :
    Message(appUid, Size, NULL)
{
    qint32 w, h;
    ds >> w >> h;
    m_size.setWidth(w);
    m_size.setHeight(h);
}

SizeMessage::SizeMessage(const SizeMessage& other) :
    Message(other.m_appUid, other.m_messageType, other.parent()),
    m_size(other.m_size)
{
}

void SizeMessage::writeData(QDataStream& ds)
{
    ds << (qint32)m_size.width() << (qint32)m_size.height();
}

QSize SizeMessage::size() const
{
    return m_size;
}

int SizeMessage::width() const
{
    return m_size.width();
}

int SizeMessage::height() const
{
    return m_size.height();
}

void SizeMessage::setHeight(int height)
{
    m_size.setHeight(height);
}

void SizeMessage::setWidth(int width)
{
    m_size.setWidth(width);
}

QDebug operator<<(QDebug d, const SizeMessage& sm)
{
    return d << QString("SizeMessage(appId: %1, size:(%2, %3))").arg(sm.appUid()).arg(sm.width()).arg(sm.height()).toLocal8Bit().data();
}

// EOF
