#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QPoint>
#include <QSize>
#include <QDebug>

enum MessageType {
    Undefined = 0,
    Update = 1,
    Launch,
    Move,
    Size,
    Close
};

#if defined(COMMUNICATION_LIBRARY)
#  define COMMUNICATIONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define COMMUNICATIONSHARED_EXPORT Q_DECL_IMPORT
#endif

class COMMUNICATIONSHARED_EXPORT Message
{

public:
    Message(QString appUid = QString(), int messageType = Undefined);
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

class COMMUNICATIONSHARED_EXPORT UpdateMessage : public Message
{

public:
    UpdateMessage(QString appUid = QString());

protected:
    UpdateMessage(QString appUid, QDataStream& ds);

private:
    UpdateMessage(const UpdateMessage& other);

protected:
    friend class Message;

};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const UpdateMessage& lm);

class COMMUNICATIONSHARED_EXPORT LaunchMessage : public Message
{

public:
    LaunchMessage(QString appUid = QString(), const QString& data = QString());
    QString data() const;
    void setData(QString data);

protected:
    LaunchMessage(QString appUid, QDataStream& ds);
    virtual void writeData(QDataStream& ds);

private:
    LaunchMessage(const LaunchMessage& other);

protected:
    QString m_data;

    friend class Message;
};

COMMUNICATIONSHARED_EXPORT QDebug operator<<(QDebug d, const LaunchMessage& lm);

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
    SizeMessage(QString appUid = QString(), int height = 0, int width = 0);
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

#endif // MESSAGE_H
