#include "message.h"

#include <QIODevice>
#include <QDataStream>
#include <QMouseEvent>
#include <QThread>

#include <QDebug>


// Message

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
        socket->bytesAvailable() > 2 * (int)sizeof(quint32)) {

        QDataStream ds(socket);
        qint32 messageType;
        quint32 dataSize;
        QString appUid;
        ds >> dataSize;
        qint64 bytesAvailable(socket->bytesAvailable());
        if (bytesAvailable < dataSize) {
            qWarning() << "Message::read - bytesAvailable:" << bytesAvailable << "<" << "dataSize:" << dataSize;
            return msg;
        }
        ds >> messageType >> appUid;
        qDebug() << "Message::read - messageType:" << MSGTYPETOSTRING(messageType);
        switch (messageType) {
        case MessageType::CloneData : {
            if (bytesAvailable > 0) {
                msg = new CloneDataMessage(appUid, ds);
            }
            break;
        }
        case MessageType::Mouse : {
            if (bytesAvailable > 0) {
                msg = new MouseMessage(appUid, ds);
            }
            break;
        }
        case MessageType::RemoteLaunch : {
            if (bytesAvailable > 0) {
                msg = new RemoteLaunchMessage(appUid, ds);
            }
            break;
        }
        case MessageType::Move : {
            if (bytesAvailable >= (qint64)(2 * sizeof(qint32))) {
                msg = new MoveMessage(appUid, ds);
            }
            break;
        }
        case MessageType::Size : {
            if (bytesAvailable >= (qint64)(2 * sizeof(qint32))) {
                msg = new SizeMessage(appUid, ds);
            }
            break;
        }
        case MessageType::Geometry : {
            if (bytesAvailable >= (qint64)(4 * sizeof(qint32))) {
                msg = new GeometryMessage(appUid, ds);
            }
            break;
        }
        case MessageType::RemoteDirection : {
            if (bytesAvailable >= (qint64)(sizeof(qint32))) {
                msg = new RemoteDirectionMessage(appUid, ds);
            }
            break;
        }
        case MessageType::RemoteApplication : {
            if (bytesAvailable > 0) {
                msg = new RemoteApplicationMessage(appUid, ds);
            }
            break;
        }
        case MessageType::RemotePort : {
            if (bytesAvailable > 0) {
                msg = new RemotePortMessage(appUid, ds);
            }
            break;
        }
        case MessageType::Update : ; // fall through
        case MessageType::CloneRequest: ; // fall through
        case MessageType::RemoteView : ; // fall through
        case MessageType::RemoteGetApplication: ; // fall through
        case MessageType::RemotePing : ; // fall through
        case MessageType::Close : {
            msg = new Message(appUid, messageType);
            break;
        }
        default : {
            qWarning("Message::read - Error: Unhandled message: %s", qPrintable(MSGTYPETOSTRING(messageType)));
        }
        } // end of switch

        if (ds.status() != QDataStream::Ok) {
            delete msg;
            msg = NULL;
        }
    }
    return msg;
}

qint64 Message::write(QIODevice* socket)
{
    qint64  bytesWritten(0);
    if (!socket) {
        qWarning("Message::write - Error socket is NULL");
    }
    else if (!socket->isWritable()) {
        qWarning("Message::write - Error socket not opened for writing");
    }
    else if (m_messageType == MessageType::Undefined) {
        qWarning("Message::write - Error: message type is Undefined");
    }
    else {
        QByteArray data;
        QDataStream ds(&data, QIODevice::WriteOnly);
        ds << (quint32)0 << (qint32)m_messageType << m_appUid;
        writeData(ds);
        ds.device()->seek(0);
        ds << (quint32)(data.size() - sizeof(quint32));
        bytesWritten = socket->write(data);
    }
    return bytesWritten;
}

void Message::writeData(QDataStream& ds)
{
    Q_UNUSED(ds);
}

QDebug operator<<(QDebug d, const Message& m)
{
    return d << QString("Message(appId: %1, type: %2)").arg(m.appUid()).arg(MSGTYPETOSTRING(m.type())).toLocal8Bit().data();
}

// RemoteLaunchMessage

RemoteLaunchMessage::RemoteLaunchMessage(QString appUid, const QString& data) :
    Message(appUid, MessageType::RemoteLaunch),
    m_data(data)
{
}

RemoteLaunchMessage::RemoteLaunchMessage(QString appUid, QDataStream& ds) :
    Message(appUid, MessageType::RemoteLaunch)
{
    ds >> m_data;
}

RemoteLaunchMessage::RemoteLaunchMessage(const RemoteLaunchMessage& other) :
    Message(other.m_appUid, MessageType::RemoteLaunch),
    m_data(other.m_data)
{
}

void RemoteLaunchMessage::writeData(QDataStream& ds)
{
    ds << m_data;
}

QString RemoteLaunchMessage::data() const
{
    return m_data;
}

void RemoteLaunchMessage::setData(QString data)
{
    m_data = data;
}

QDebug operator<<(QDebug d, const RemoteLaunchMessage& lm)
{
    return d << QString("RemoteLaunchMessage(appId: %1, data: %2)").arg(lm.appUid()).arg(lm.data()).toLocal8Bit().data();
}

// MoveMessage

MoveMessage::MoveMessage(QString appUid, int x, int y) :
    Message(appUid, MessageType::Move),
    m_move(x, y)
{
}

MoveMessage::MoveMessage(QString appUid, QDataStream& ds) :
    Message(appUid, MessageType::Move)
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
    Message(appUid, MessageType::Size),
    m_size(width, height)
{
}

SizeMessage::SizeMessage(QString appUid, QDataStream& ds) :
    Message(appUid, MessageType::Size)
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
    Message(appUid, MessageType::Geometry),
    m_geometry(x, y, width, height)
{
}

GeometryMessage::GeometryMessage(QString appUid, QRect geometry) :
    Message(appUid, MessageType::Geometry),
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
    Message(appUid, MessageType::Geometry)
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


// RemoteDirectionMessage

RemoteDirectionMessage::RemoteDirectionMessage(QString appUid, int remoteDirection) :
    Message(appUid, MessageType::RemoteDirection),
    m_remoteDirection(remoteDirection)
{
}

RemoteDirectionMessage::RemoteDirectionMessage(QString appUid, QDataStream& ds) :
    Message(appUid, MessageType::RemoteDirection)
{
    ds >> m_remoteDirection;
}

RemoteDirectionMessage::RemoteDirectionMessage(const RemoteDirectionMessage& other) :
    Message(other.m_appUid, other.m_messageType),
    m_remoteDirection(other.m_remoteDirection)
{
}

void RemoteDirectionMessage::writeData(QDataStream& ds)
{
    ds << m_remoteDirection;
}

int RemoteDirectionMessage::remoteDirection() const
{
    return m_remoteDirection;
}

void RemoteDirectionMessage::setRemoteDirection(int remoteDirection)
{
    m_remoteDirection = remoteDirection;
}

QDebug operator<<(QDebug d, const RemoteDirectionMessage& rdm)
{
    return d << QString("RemoteDirectionMessage(appId: %1, RemoteDirection: %2)").arg(rdm.appUid()).arg(rdm.remoteDirection()).toLocal8Bit().data();
}


// RemotePortMessage

RemotePortMessage::RemotePortMessage(QString appUid, quint32 port) :
    Message(appUid, MessageType::RemotePort),
    m_port(port)
{
}

quint32 RemotePortMessage::port() const
{
    return m_port;
}

void RemotePortMessage::setPort(quint32 port)
{
    m_port = port;
}

RemotePortMessage::RemotePortMessage(QString appUid, QDataStream &ds) :
    Message(appUid, MessageType::RemotePort)
{
    ds >> m_port;
}

void RemotePortMessage::writeData(QDataStream &ds)
{
    ds << (qint32)m_port;
}

RemotePortMessage::RemotePortMessage(const RemotePortMessage &other) :
    Message(other.m_appUid, other.m_messageType),
    m_port(other.m_port)
{
}

QDebug operator<<(QDebug d, const RemotePortMessage& rpm)
{
    return d << QString("RemotePortMessage(appId: %1, port: %2)").arg(rpm.port()).arg(rpm.appUid()).toLocal8Bit().data();
}


// CloneDataMessage

CloneDataMessage::CloneDataMessage(QString appUid) :
    Message(appUid, MessageType::CloneData)
{
}

void CloneDataMessage::setIndexPropertyValue(int index, QString property, QVariant value)
{
    QMap<QString, QVariant> map(m_indexPropertyValues.value(index, QMap<QString, QVariant>()));
    map.insert(property, value);
    m_indexPropertyValues.insert(index, map);
}

const QVariant CloneDataMessage::indexPropertyValue(int index, QString property) const
{
    QMap<QString, QVariant> map(m_indexPropertyValues.value(index, QMap<QString, QVariant>()));
    return map.value(property, QVariant());
}

QStringList CloneDataMessage::properties(int index) const
{
    return m_indexPropertyValues.value(index, QMap<QString, QVariant>()).keys();
}

CloneDataMessage::CloneDataMessage(QString appUid, QDataStream &ds) :
    Message(appUid, MessageType::CloneData)
{
    quint32 mapCount;
    ds >> mapCount;
    for (int i(0); i < (int)mapCount; i++) {
        quint32 index;
        ds >> index;
        QVariant inVariant;
        ds >> inVariant;
        if (inVariant.type() == QVariant::Map) {
            m_indexPropertyValues.insert(index, inVariant.toMap());
        }
    }
}

void CloneDataMessage::writeData(QDataStream &ds)
{
    QList<int> mapIndexes(m_indexPropertyValues.keys());
    int mapCount(mapIndexes.count());
    ds << (quint32) mapCount;
    for (int i(0); i < mapCount; i++) {
        int index(mapIndexes.at(i));
        ds << (quint32) index;
        QVariant outVariant(m_indexPropertyValues.value(index));
        ds << outVariant;
    }
}

CloneDataMessage::CloneDataMessage(const CloneDataMessage &other) :
    Message(other.appUid(), MessageType::CloneData),
    m_indexPropertyValues(other.m_indexPropertyValues)
{
}

QDebug operator<<(QDebug d, const CloneDataMessage& cdm)
{
    return d << QString("CloneDataMessage(appId: %1)").arg(cdm.appUid()).toLocal8Bit().data();
}


// InputMessage

InputMessage::InputMessage(QString appUid, int messageType, QEvent::Type eventType, Qt::KeyboardModifiers modifiers, quint64 timestamp) :
    Message(appUid, messageType),
    m_eventType(eventType),
    m_modifiers(modifiers),
    m_timestamp(timestamp)
{
}

QEvent::Type InputMessage::eventType() const
{
    return m_eventType;
}

Qt::KeyboardModifiers InputMessage::modifiers() const
{
    return m_modifiers;
}

quint64 InputMessage::timestamp() const
{
    return m_timestamp;
}

InputMessage::InputMessage(QString appUid, int messageType, QDataStream &ds) :
    Message(appUid, messageType)
{
    quint32 eventType, modifiers;
    ds >> eventType >> modifiers;
    m_eventType = (QEvent::Type)eventType;
    m_modifiers = (Qt::KeyboardModifiers) modifiers;
    ds >> m_timestamp;
}

void InputMessage::writeData(QDataStream &ds)
{
    ds << (quint32)m_eventType << (quint32)m_modifiers;
    ds << m_timestamp;
}

InputMessage::InputMessage(const InputMessage &other) :
    Message(other.m_appUid, other.m_messageType),
    m_eventType(other.m_eventType),
    m_modifiers(other.m_modifiers),
    m_timestamp(other.m_timestamp)
{
}


// MouseMessage
MouseMessage::MouseMessage(QString appUid, QMouseEvent* event) :
    InputMessage(appUid, MessageType::Mouse, event->type(), event->modifiers(), event->timestamp()),
    m_localPos(event->localPos()),
    m_windowPos(event->windowPos()),
    m_screenPos(event->screenPos()),
    m_button(event->button()),
    m_buttons(event->buttons())
{
}

MouseMessage::MouseMessage(QString appUid, QDataStream &ds) :
    InputMessage(appUid, MessageType::Mouse, ds)
{
    quint32 button, buttons;
    ds >> button >> buttons;
    m_button = (Qt::MouseButton)button;
    m_buttons = (Qt::MouseButtons)buttons;
    ds >> m_localPos >> m_windowPos >> m_screenPos;
}

void MouseMessage::writeData(QDataStream &ds)
{
    InputMessage::writeData(ds);
    ds << (quint32)m_button << (quint32)m_buttons << m_localPos << m_windowPos << m_screenPos;
}

QPointF MouseMessage::localPos() const
{
    return m_localPos;
}

QPointF MouseMessage::windowPos() const
{
    return m_windowPos;
}

QPointF MouseMessage::screenPos() const
{
    return m_screenPos;
}

Qt::MouseButton MouseMessage::button() const
{
    return m_button;
}

Qt::MouseButtons MouseMessage::buttons() const
{
    return m_buttons;
}

void MouseMessage::addX(int x)
{
    m_localPos.setX(m_localPos.x() + x);
    m_windowPos.setX(m_localPos.x() + x);
    m_screenPos.setX(m_localPos.x() + x);
}

void MouseMessage::addY(int y)
{
    m_localPos.setY(m_localPos.y() + y);
    m_windowPos.setY(m_localPos.y() + y);
    m_screenPos.setY(m_localPos.y() + y);
}

QMouseEvent* MouseMessage::createMouseEvent() const
{
    QMouseEvent* mouseEvent(new QMouseEvent(m_eventType, m_localPos, m_windowPos, m_screenPos, m_button, m_buttons, m_modifiers));
    mouseEvent->setTimestamp(m_timestamp);
    return mouseEvent;
}

MouseMessage::MouseMessage(const MouseMessage& other) :
    // InputMessage(appUid, MessageType::Mouse, other.eventType(), other.modifiers()),
    InputMessage(other),
    m_localPos(other.localPos()),
    m_windowPos(other.windowPos()),
    m_screenPos(other.screenPos()),
    m_button(other.button()),
    m_buttons(other.buttons())
{
}

QDebug operator<<(QDebug d, const MouseMessage& mm)
{
    return d << QString("MouseMessage(appId: %1, type: %2)").arg(mm.appUid()).arg(mm.eventType()).toLocal8Bit().data();
}


// RemoteApplicationMessage

RemoteApplicationMessage::RemoteApplicationMessage(QString appUid) :
    Message(appUid, MessageType::RemoteApplication)
{
}

QStringList RemoteApplicationMessage::files() const
{
    return m_fileData.keys();
}

void RemoteApplicationMessage::setFileData(QString fileName, QByteArray fileData)
{
    m_fileData.insert(fileName, QVariant::fromValue<QByteArray>(fileData));
}

QByteArray RemoteApplicationMessage::fileData(QString fileName) const
{
    return m_fileData.value(fileName, QVariant()).toByteArray();
}

RemoteApplicationMessage::RemoteApplicationMessage(QString appUid, QDataStream &ds) :
    Message(appUid, MessageType::RemoteApplication)
{
    QVariant inVariant;
    ds >> inVariant;
    if (inVariant.type() == QVariant::Map) {
        m_fileData = inVariant.toMap();
    }
}

void RemoteApplicationMessage::writeData(QDataStream &ds)
{
    QVariant outVariant(m_fileData);
    ds << outVariant;
}

RemoteApplicationMessage::RemoteApplicationMessage(const RemoteApplicationMessage &other) :
    Message(other.m_appUid, other.m_messageType),
    m_fileData(other.m_fileData)
{
}

QDebug operator<<(QDebug d, const RemoteApplicationMessage& ram)
{
    return d << QString("RemoteApplicationMessage(appId: %1, file count: %2)").arg(ram.appUid()).arg(ram.files().count()).toLocal8Bit().data();
}


// EOF


