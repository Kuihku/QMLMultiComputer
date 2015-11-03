#include "message.h"

#include <QIODevice>
#include <QDataStream>
#include <QThread>

#include <QDebug>


// Message

#define MSGTYPETOSTRING(type) \
    QString(type == Undefined ? "Undefined" : \
    type == Update ? "Update" : \
    type == Launch ? "Launch" : \
    type == Move ? "Move" : \
    type == Size ? "Size" : \
    type == Close ? "Close" : "Type unknown")

Message::Message(QString appUid, int messageType) :
    m_appUid(appUid),
    m_messageType(messageType)
{
}

Message::~Message()
{
}

QString Message::appUid() const
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
        case Geometry : {
            if (bytesAvailable >= (qint64)(4 * sizeof(qint32))) {
                msg = new GeometryMessage(appUid, ds);
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

UpdateMessage::UpdateMessage(QString appUid) :
    Message(appUid, Update)
{
}

UpdateMessage::UpdateMessage(QString appUid, QDataStream &ds) :
    Message(appUid, Update)
{
    Q_UNUSED(ds)
}

UpdateMessage::UpdateMessage(const UpdateMessage& other) :
    Message(other.m_appUid, Update)
{
}

QDebug operator<<(QDebug d, const UpdateMessage& m)
{
    return d << QString("UpdateMessage(appId: %1, type: %2)").arg(m.appUid()).arg(MSGTYPETOSTRING(m.type())).toLocal8Bit().data();
}


// LaunchMessage

LaunchMessage::LaunchMessage(QString appUid, const QString& data) :
    Message(appUid, Launch),
    m_data(data)
{
}

LaunchMessage::LaunchMessage(QString appUid, QDataStream& ds) :
    Message(appUid, Launch)
{
    ds >> m_data;
}

LaunchMessage::LaunchMessage(const LaunchMessage& other) :
    Message(other.m_appUid, Launch),
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

MoveMessage::MoveMessage(QString appUid, int x, int y) :
    Message(appUid, Move),
    m_move(x, y)
{
}

MoveMessage::MoveMessage(QString appUid, QDataStream& ds) :
    Message(appUid, Move)
{
    qint32 x, y;
    ds >> x >> y;
    m_move.setX(x);
    m_move.setY(y);
}

MoveMessage::MoveMessage(const MoveMessage& other) :
    Message(other.m_appUid, other.m_messageType),
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

SizeMessage::SizeMessage(QString appUid, int width, int height) :
    Message(appUid, Size),
    m_size(width, height)
{
}

SizeMessage::SizeMessage(QString appUid, QDataStream& ds) :
    Message(appUid, Size)
{
    qint32 w, h;
    ds >> w >> h;
    m_size.setWidth(w);
    m_size.setHeight(h);
}

SizeMessage::SizeMessage(const SizeMessage& other) :
    Message(other.m_appUid, other.m_messageType),
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


// Geometry

GeometryMessage::GeometryMessage(QString appUid, int x, int y, int width, int height) :
    Message(appUid, Geometry),
    m_geometry(x, y, width, height)
{

}

GeometryMessage::GeometryMessage(QString appUid, QRect geometry) :
    Message(appUid, Geometry),
    m_geometry(geometry)
{
}

QRect GeometryMessage::geometry() const
{
    return m_geometry;
}

int GeometryMessage::x() const
{
    return m_geometry.x();
}

int GeometryMessage::y() const
{
    return m_geometry.y();
}

int GeometryMessage::width() const
{
    return m_geometry.width();
}

int GeometryMessage::height() const
{
    return m_geometry.height();
}

void GeometryMessage::setX(int x)
{
    return m_geometry.setX(x);
}

void GeometryMessage::setY(int y)
{
    return m_geometry.setY(y);
}

void GeometryMessage::setHeight(int height)
{
    return m_geometry.setHeight(height);
}

void GeometryMessage::setWidth(int width)
{
    return m_geometry.setWidth(width);
}

void GeometryMessage::setGeometry(QRect geometry)
{
    m_geometry = geometry;
}

GeometryMessage::GeometryMessage(QString appUid, QDataStream &ds) :
    Message(appUid, Geometry)
{
    qint32 x, y, w, h;
    ds >> x >> y >> w >> h;
    m_geometry.setX(x);;
    m_geometry.setY(y);;
    m_geometry.setWidth(w);
    m_geometry.setHeight(h);
}

void GeometryMessage::writeData(QDataStream &ds)
{
    ds << (qint32)m_geometry.x() << (qint32)m_geometry.y() << (qint32)m_geometry.width() << (qint32)m_geometry.height();
}

GeometryMessage::GeometryMessage(const GeometryMessage &other) :
    Message(other.m_appUid, other.m_messageType),
    m_geometry(other.m_geometry)
{
}

QDebug operator<<(QDebug d, const GeometryMessage& gm)
{
    return d << QString("GeometryMessage(appId: %1, geometry:(%2, %3, %4, %5))").arg(gm.appUid()).arg(gm.x()).arg(gm.y()).arg(gm.width()).arg(gm.height()).toLocal8Bit().data();
}

// EOF
