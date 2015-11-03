#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QList>
#include <QMap>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server();


protected slots:
    void newLocalConnection();

private:
    void parseConfigFile();

signals:

public slots:

private:
    enum RemoteDirection {
        NorthWest = 1,
        North,
        NorthEast,
        West,
        East,
        SouthWest,
        South,
        SouthEast
    };

private: // data
    class QWidget* m_view;
    class QLocalServer* m_localServer;
    class QTcpServer* m_tcpServer;
    QList<class LocalConnection*> m_localConnections;
    QMap<RemoteDirection, class RemoteConnection*> m_remoteConnections;

};

#endif // SERVER_H
