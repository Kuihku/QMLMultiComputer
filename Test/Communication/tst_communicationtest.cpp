#include <QString>
#include <QtTest>
#include <QBuffer>
#include <QDateTime>
#include <QtGlobal>
#include <QDataStream>

#include "message.h"


class Buffer : public QBuffer
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
public:
#ifndef QT_NO_QOBJECT
     explicit Buffer(QObject *parent = Q_NULLPTR);
     Buffer(QByteArray *buf, QObject *parent = Q_NULLPTR);
#else
     Buffer();
     explicit Buffer(QByteArray *buf);
#endif
     virtual qint64 bytesAvailable() const;

private:
    Q_DISABLE_COPY(Buffer)
};

#ifndef QT_NO_QOBJECT
Buffer::Buffer(QObject *parent) : QBuffer(parent)
{
}

Buffer::Buffer(QByteArray *buf, QObject *parent) : QBuffer(buf, parent)
{
}

#else
Buffer::Buffer()
{
}

Buffer::Buffer(QByteArray *buf) : QBuffer(parent)
{
}
#endif

qint64 Buffer::bytesAvailable() const
{
    return buffer().size() - pos();
}

class CommunicationTest : public QObject
{
    Q_OBJECT

public:
    CommunicationTest();

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void buf();
    void message_data();
    void message();

private:
    Buffer m_buf;
    QByteArray m_byteArray;

};

CommunicationTest::CommunicationTest()
{
}

void CommunicationTest::initTestCase()
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
    m_buf.setBuffer(&m_byteArray);
}

void CommunicationTest::init()
{
    m_buf.open(QIODevice::ReadWrite);
}

void CommunicationTest::cleanup()
{
    m_buf.close();
    m_byteArray.clear();
}

void CommunicationTest::buf()
{
    int appUid1(qrand());
    QDataStream ds1(&m_buf);
    ds1 << appUid1;

    m_buf.seek(0);

    QDataStream ds2(&m_buf);
    int appUid2;
    ds2 >> appUid2;

    QCOMPARE(appUid1, appUid2);
}

void CommunicationTest::message_data()
{
    QTest::addColumn<int>("messageType");
    int mType(MessageType::Undefined);
    while (mType < MessageType::MessageTypeLast) {
        QTest::newRow(MSGTYPETOSTRING(mType).toLatin1().constData()) << mType;
        mType++;
    }
}

void CommunicationTest::message()
{
    QFETCH(int, messageType);
    QString appUid(QString::number(qrand()));
    qDebug() << "appUid:" << appUid;
    Message out(appUid, messageType);
    if (messageType == MessageType::Undefined) {
        QVERIFY(!out.write(&m_buf));
        return;
    }

    QVERIFY(m_buf.isWritable());
    QVERIFY(out.write(&m_buf));

    m_buf.seek(0);

    QVERIFY(m_buf.isReadable());
    QVERIFY(m_buf.bytesAvailable() > (int)sizeof(quint32));

    Message* in(Message::read(&m_buf));
    qDebug("in: %p", in);
    QVERIFY(in != NULL);
    QCOMPARE(appUid, in->appUid());
    QCOMPARE(messageType, in->type());
    delete in;
    QVERIFY(messageType == messageType);
}

QTEST_APPLESS_MAIN(CommunicationTest)

#include "tst_communicationtest.moc"
