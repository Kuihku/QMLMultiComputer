#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>

#include "remoteconnectioninfo.h"

class RemoteConnection : public QObject
{
    Q_OBJECT

public:
    RemoteConnection(Remote::Direction remoteDirection, QByteArray ip, QObject* parent = NULL);
    RemoteConnection(class QTcpSocket* socket, QObject* parent = NULL);
    Remote::Direction remoteDirection() const;

signals:
    void connectionReady();

protected slots:
    void socketConnected();
    void socketDisconnected();
    void readSocket();

private:
    void handleRemoteDirection(Remote::Direction remoteDicrection);

private:
    Remote::Direction m_remoteDirection;
    class QTcpSocket* m_remoteSocket;
    quint32 m_ip;

};

#endif // REMOTECONNECTION_H
