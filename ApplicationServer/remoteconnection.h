#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>

class RemoteConnection : public QObject
{
    Q_OBJECT

public:
    RemoteConnection(QByteArray ip, QObject* parent = NULL);

private:
    QByteArray m_ip;

};

#endif // REMOTECONNECTION_H
