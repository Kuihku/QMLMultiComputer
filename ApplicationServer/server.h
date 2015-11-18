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
    explicit Server(QString configFile, QObject* parent = Q_NULLPTR);
    virtual ~Server();

protected: // from class WindowHolder
    virtual void paintWindows(QRect rect, QRegion region, class QPainter* painter);
    void sendImage(QString appUid, QImage image);

protected slots:
    void newRemoteConnection();
    void remoteConnectionReady();
    void remoteConnectionClosed();
    void remoteUpdate(QRect geometry);
    void newLocalConnection();
    void localUpdate(QRect geometry);
    void localGeometryChanged(QString appUid, QRect geometry);
    void localConnectionClosed();

    void launchApplication(QString appUid, QString data);
    void cloneApplicationReceived(class CloneDataMessage* cdm);
    void applicationReceived(class RemoteApplicationMessage* ram);

private:
    QHostAddress myIPv4();
    void parseConfigFile(QString configFile);
    void setupRemoteConnection(class RemoteConnection* remoteConnection);

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
