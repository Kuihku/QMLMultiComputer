#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QDebug>


namespace MessageType {
    enum {
        Undefined = 0,
        Update = 1,
        Move,
        Size,
        Close,
        Geometry,

        RemoteDirection = 0x100,
        RemoteLaunch,
        RemoteView,
        RemoteGeometry,
        RemotePing // For testing purpose
    };
}

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
    static Message* read(class QIODevice* socket);
    bool write(class QIODevice* socket);

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

class COMMUNICATIONSHARED_EXPORT RemoteGeometryMessage : public GeometryMessage
{

public:
    RemoteGeometryMessage(QString appUid = QString(), quint32 port = 0, int x = 0, int y = 0, int width = 0, int height = 0);
    RemoteGeometryMessage(QString appUid, quint32 port, QRect geometry);
    quint32 port() const;
    void setPort(quint32 port);

protected:
    RemoteGeometryMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    RemoteGeometryMessage(const RemoteGeometryMessage& other);

protected:
    int m_port;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const RemoteGeometryMessage& gm);


#endif // MESSAGE_H
