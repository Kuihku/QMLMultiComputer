#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QProcess>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QVariant>

#include "remoteconnectioninfo.h"
#include "message.h"

class ApplicationServerTest : public QObject
{
    Q_OBJECT

public:
    ApplicationServerTest();

public slots:
    void incomingTcpConnection();
    void debugApplicationServerStandardOutput();
    void debugApplicationServerStandardError();
    void readTcpSocket();

signals:
    void messageReveived(int);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void ping();

private:
    QTcpServer m_tcpServer;
    QProcess m_applicationServer;
    QTcpSocket* m_socket;
    QList <Message*> m_messageQue;

};

ApplicationServerTest::ApplicationServerTest() : m_socket(NULL)
{
}

void ApplicationServerTest::incomingTcpConnection()
{
    while (m_tcpServer.hasPendingConnections()) {
        if (m_socket) {
            delete m_tcpServer.nextPendingConnection();
        }
        else {
            m_socket = m_tcpServer.nextPendingConnection();
            connect(m_socket, SIGNAL(readyRead()), this, SLOT(readTcpSocket()));
        }
    }
}

void ApplicationServerTest::debugApplicationServerStandardOutput()
{
    qDebug() << m_applicationServer.readAllStandardOutput();
}

void ApplicationServerTest::debugApplicationServerStandardError()
{
    qWarning() << m_applicationServer.readAllStandardError();
}

void ApplicationServerTest::readTcpSocket()
{
    if (m_socket) {
        Message* m(Message::read(m_socket));
        if (m) {
            m_messageQue.append(m);
            emit messageReveived(m->type());
        }
    }
}

void ApplicationServerTest::initTestCase()
{
    QSignalSpy directionSpy(this, SIGNAL(messageReveived(int)));
    connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(incomingTcpConnection()));
    QVERIFY2(m_tcpServer.listen(QHostAddress(TESTTCPSERVERIPADDRESS), REMOTE_PORT), qPrintable(m_tcpServer.errorString()));

    connect(&m_applicationServer, SIGNAL(readyReadStandardOutput()), this, SLOT(debugApplicationServerStandardOutput()));
    connect(&m_applicationServer, SIGNAL(readyReadStandardError()), this, SLOT(debugApplicationServerStandardError()));
    m_applicationServer.setArguments(QStringList() << SERVER_CONFIG);
    m_applicationServer.setProgram(SERVEREXE);
    m_applicationServer.start();
    QVERIFY2(m_applicationServer.waitForStarted(5000), "Application server start error"); // 5 secs
    QTRY_VERIFY2(m_socket != NULL, "Socket not connected");

    QVERIFY2(directionSpy.wait(5000), "Direction message not received");
    bool directionSpyFormatOk(false);
    int directionFormatMessageType(directionSpy.takeFirst().takeFirst().toInt(&directionSpyFormatOk));
    QVERIFY2(directionSpyFormatOk, "Diretion message format failed");
    QVERIFY2(directionFormatMessageType == MessageType::RemoteDirection, "Wrong type in signal in direction message type");
    Message* m(m_messageQue.takeFirst());
    QVERIFY2(m->type() == MessageType::RemoteDirection, "Wrong type in que in direction direction message type");
    delete m;
}

void ApplicationServerTest::cleanupTestCase()
{
    if (m_socket) {
        m_socket->close();
        delete m_socket;
    }
    if (m_applicationServer.state() == QProcess::Running) {
        m_applicationServer.terminate();
    }
    QVERIFY2(m_applicationServer.waitForFinished(5000), "Application server terminate error"); // 5 secs

    m_tcpServer.close();
}

void ApplicationServerTest::cleanup()
{
    while (!m_messageQue.isEmpty()) {
        Message* m(m_messageQue.takeLast());
        qDebug() << "message type:" << m->type();
        delete m;
    }
}

void ApplicationServerTest::ping()
{
    QSignalSpy pingSpy(this, SIGNAL(messageReveived(int)));
    Message m("tst_ping", MessageType::RemotePing);
    m.write(m_socket);

    QVERIFY2(pingSpy.wait(), "Ping message not received");
}

QTEST_GUILESS_MAIN(ApplicationServerTest)

#include "tst_applicationservertest.moc"
