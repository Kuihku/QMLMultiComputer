#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QDebug>
#include <QEvent>

namespace MessageType {
    enum {
        Undefined = 0,
        Update = 1,
        Move,
        Size,
        Close,
        Geometry,
        CloneRequest,
        CloneData,
        Mouse,
        RemoteDirection,
        RemoteLaunch,
        RemoteView,
        RemotePort,
        RemoteGetApplication,
        RemoteApplication,

        // For testing purpose
        RemotePing

#ifdef COMMUNICATION_TEST
        // This must be last in this enum for testing purpose
        , MessageTypeLast

#endif // COMMUNICATION_TEST
    };
}

#define MSGTYPETOSTRING(type) \
    QString(type == MessageType::Undefined ? "Undefined" : \
    type == MessageType::Update ? "Update" : \
    type == MessageType::Move ? "Move" : \
    type == MessageType::Size ? "Size" : \
    type == MessageType::Close ? "Close" : \
    type == MessageType::Geometry ? "Geometry" : \
    type == MessageType::CloneRequest ? "CloneRequest" : \
    type == MessageType::CloneData ? "CloneData" : \
    type == MessageType::Mouse ? "Mouse" : \
    type == MessageType::RemoteDirection ? "RemoteDirection" : \
    type == MessageType::RemoteLaunch ? "RemoteLaunch" : \
    type == MessageType::RemoteView ? "RemoteView" : \
    type == MessageType::RemotePort ? "RemotePort" : \
    type == MessageType::RemoteGetApplication ? "RemoteGetApplication" : \
    type == MessageType::RemoteApplication ? "RemoteApplication" : \
    type == MessageType::RemotePing? "RemotePing" : QString("Type unknown: %1").arg(type))

#if defined(COMMUNICATION_LIBRARY)
#  define COMMUNICATIONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define COMMUNICATIONSHARED_EXPORT Q_DECL_IMPORT
#endif

class COMMUNICATIONSHARED_EXPORT Message
{

public:
    Message(QString appUid = QString(), int messageType = MessageType::Undefined);
    virtual ~Message();
    QString appUid() const;
    int type() const;

    // note: on static method Message::read returned Message* ownership is changed,
    // i.e. caller must delete Message*
    static Message* read(class QIODevice* socket);
    qint64 write(class QIODevice* socket);

protected:
    virtual void writeData(QDataStream& ds);

private:
    Message(const Message& other);

protected:
    QString m_appUid;
    int m_messageType;

};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const Message& m);

class COMMUNICATIONSHARED_EXPORT RemoteLaunchMessage : public Message
{

public:
    RemoteLaunchMessage(QString appUid = QString(), const QString& data = QString());
    QString data() const;
    void setData(QString data);

protected:
    RemoteLaunchMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    RemoteLaunchMessage(const RemoteLaunchMessage& other);

protected:
    QString m_data;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const RemoteLaunchMessage& lm);

class COMMUNICATIONSHARED_EXPORT RemoteApplicationMessage : public Message
{

public:
    RemoteApplicationMessage(QString appUid = QString());
    QStringList files() const;
    void setFileData(QString fileName, QByteArray fileData);
    QByteArray fileData(QString fileName) const;

protected:
    RemoteApplicationMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    RemoteApplicationMessage(const RemoteApplicationMessage& other);

protected:
    QMap<QString, QVariant> m_fileData;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const RemoteApplicationMessage& ram);


class COMMUNICATIONSHARED_EXPORT MoveMessage : public Message
{

public:
    MoveMessage(QString appUid = QString(), int x = 0, int y = 0);
    QPoint topLeft() const;
    int x() const;
    int y() const;
    void setX(int x);
    void setY(int y);

protected:
    MoveMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    MoveMessage(const MoveMessage& other);

protected:
    QPoint m_move;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const MoveMessage& mm);

class COMMUNICATIONSHARED_EXPORT CloneDataMessage : public Message
{

public:
    CloneDataMessage(QString appUid = QString());
    void setIndexPropertyValue(int index, QString property, QVariant value);
    const QVariant indexPropertyValue(int index, QString property) const;
    QStringList properties(int index) const;

protected:
    CloneDataMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    CloneDataMessage(const CloneDataMessage& other);

protected:
    QMap<int, QMap<QString, QVariant> > m_indexPropertyValues;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const MoveMessage& mm);

class COMMUNICATIONSHARED_EXPORT InputMessage : public Message
{

public:
    InputMessage(QString appUid = QString(), int messageType = MessageType::Undefined, QEvent::Type eventType = QEvent::None, Qt::KeyboardModifiers modifiers = Qt::NoModifier, quint64 timestamp = 0);
    QEvent::Type eventType() const;
    Qt::KeyboardModifiers modifiers() const;
    quint64 timestamp() const;

protected:
    InputMessage(QString appUid, int messageType, QDataStream& ds);
    virtual void writeData(QDataStream& ds);
    InputMessage(const InputMessage& other);

protected:
    QEvent::Type m_eventType;
    Qt::KeyboardModifiers m_modifiers;
    quint64 m_timestamp;

//    friend class Message;
};


class COMMUNICATIONSHARED_EXPORT MouseMessage : public InputMessage
{

public:
    MouseMessage(QString appUid, class QMouseEvent* event);
    QPointF localPos() const;
    QPointF windowPos() const;
    QPointF screenPos() const;
    Qt::MouseButton button() const;
    Qt::MouseButtons buttons() const;

    void addX(int x);
    void addY(int y);

    // Note: Returned QMouseEvent* ownership is changed and caller is responsible to delete
    class QMouseEvent* createMouseEvent() const;

protected:
    MouseMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    MouseMessage(const MouseMessage& other);

protected:
    QPointF m_localPos;
    QPointF m_windowPos;
    QPointF m_screenPos;
    Qt::MouseButton m_button;
    Qt::MouseButtons m_buttons;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const MoveMessage& mm);


class COMMUNICATIONSHARED_EXPORT SizeMessage : public Message
{

public:
    SizeMessage(QString appUid = QString(), int width = 0, int height = 0);
    QSize size() const;
    int width() const;
    int height() const;
    void setHeight(int height);
    void setWidth(int width);

protected:
    SizeMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    SizeMessage(const SizeMessage& other);

protected:
    QSize m_size;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const SizeMessage& sm);

class COMMUNICATIONSHARED_EXPORT GeometryMessage : public Message
{

public:
    GeometryMessage(QString appUid = QString(), int x = 0, int y = 0, int width = 0, int height = 0);
    GeometryMessage(QString appUid, QRect geometry);
    QRect geometry() const;
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    void setX(int x);
    void setY(int y);
    void setHeight(int height);
    void setWidth(int width);
    void setGeometry(QRect geometry);

protected:
    GeometryMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    GeometryMessage(const GeometryMessage& other);

protected:
    QRect m_geometry;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const GeometryMessage& gm);

class COMMUNICATIONSHARED_EXPORT RemoteDirectionMessage : public Message
{

public:
    RemoteDirectionMessage(QString appUid = QString(), int remoteDirection = 0);
    int remoteDirection() const;
    void setRemoteDirection(int remoteDirection);

protected:
    RemoteDirectionMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    RemoteDirectionMessage(const RemoteDirectionMessage& other);

protected:
    qint32 m_remoteDirection;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const RemoteDirectionMessage& sm);

class COMMUNICATIONSHARED_EXPORT RemotePortMessage : public Message
{

public:
    RemotePortMessage(QString appUid = QString(), quint32 port = 0);
    quint32 port() const;
    void setPort(quint32 port);

protected:
    RemotePortMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    RemotePortMessage(const RemotePortMessage& other);

protected:
    int m_port;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const RemotePortMessage& gm);


#endif // MESSAGE_H
