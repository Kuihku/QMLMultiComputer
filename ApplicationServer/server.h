#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QRect>
#include <QImage>

#include "windowholder.h"
#include "remoteconnectioninfo.h"

class Server : public QObject, public WindowHolder
{
    Q_OBJECT
public:
    explicit Server(QString configFile, QObject *parent = 0);
    ~Server();

protected: // from class WindowHolder
    virtual void paintWindows(QRect rect, QRegion region, class QPainter* painter);
    void sendImage(QString appUid, QImage image);

protected slots:
    void newRemoteConnection();
    void remoteConnectionReady();
    void remoteUpdate(QRect geometry);
    void newLocalConnection();
    void localUpdate(QRect geometry);
    void localGeometryChanged(QString appUid, QRect geometry);
    void localConnectionClosed();

    void launchApplication(QString appUid, QString data);

private:
    QHostAddress myIPv4();
    void parseConfigFile(QString configFile);
    void setupRemoteConnection(class RemoteConnection* remoteConnection);

signals:

public slots:

private: // data
    QHostAddress m_myIPv4;
    class MainView* m_view;
    class QLocalServer* m_localServer;
    class QTcpServer* m_remoteServer;
    QList<class LocalConnection*> m_localConnections;
    QMap<Remote::Direction, class RemoteConnection*> m_remoteConnections;
    QList<class RemoteConnection*> m_unconnectedRemoteConnections;

};

#endif // SERVER_H
