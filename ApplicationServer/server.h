#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QList>
#include <QMap>

#include "windowholder.h"

class Server : public QObject, public WindowHolder
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server();

protected: // from class WindowHolder
    virtual void paintWindows(QRect rect, QRegion region, class QPainter* painter);

protected slots:
    void newLocalConnection();
    void localUpdate();
    void localConnectionClosed();

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
    class MainView* m_view;
    class QLocalServer* m_localServer;
    class QTcpServer* m_tcpServer;
    QList<class LocalConnection*> m_localConnections;
    QMap<RemoteDirection, class RemoteConnection*> m_remoteConnections;

};

#endif // SERVER_H
