#ifndef LOCALCONNECTION_H
#define LOCALCONNECTION_H

#include <QObject>
#include <QLocalSocket>

class LocalConnection : public QObject
{
    Q_OBJECT
public:
    explicit LocalConnection(QLocalSocket* socket, QObject *parent = 0);

protected slots:
    void socketAboutToClose();
    void socketReadyRead();
    void socketError(QLocalSocket::LocalSocketError error);

private:
    void UpdateView(QString appUid);

signals:

public slots:

private: // data
    QLocalSocket* m_socket;
    class QSharedMemory* m_shared;

};

#endif // LOCALCONNECTION_H
