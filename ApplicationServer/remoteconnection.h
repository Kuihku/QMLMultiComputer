#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>
#include <QMap>
#include <QHostAddress>

#include "remoteconnectioninfo.h"

class RemoteConnection : public QObject
{
    Q_OBJECT

public:
    RemoteConnection(QHostAddress myIPv4, Remote::Direction remoteDirection, QByteArray ip, QObject* parent = NULL);
    RemoteConnection(QHostAddress myIPv4, class QTcpSocket* socket, QObject* parent = NULL);
    Remote::Direction remoteDirection() const;
    void updateGeometry(QString appUid, int x, int y, int width, int height);
    void sendImage(QString appUid, const QImage& image);

signals:
    void connectionReady();

protected slots:
    void socketConnected();
    void socketDisconnected();
    void readSocket();

private:
    void handleRemoteDirection(Remote::Direction remoteDicrection);
    void handleGeometryUpdate(QString appUid, quint16 port, QRect rect);

private:
    quint16 m_nextUdpPort;
    QHostAddress m_myIPv4;
    Remote::Direction m_remoteDirection;
    class QTcpSocket* m_remoteSocket;
    QMap<QString, class RemoteApplication*> m_remoteApplications;
    quint32 m_ip;

};

#endif // REMOTECONNECTION_H
