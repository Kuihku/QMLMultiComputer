#include "remoteconnection.h"

RemoteConnection::RemoteConnection(QByteArray ip, QObject *parent) :
    QObject(parent),
    m_ip(ip)
{

}

