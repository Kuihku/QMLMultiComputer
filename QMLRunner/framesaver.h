#ifndef FRAMESAVER_H
#define FRAMESAVER_H

#include <QObject>
#include <QLocalSocket>

class FrameSaver : public QObject
{
    Q_OBJECT
public:
    explicit FrameSaver(QString appUid, QString server, class QQuickView* view, QObject *parent = 0);

public slots:
    void save();
    void socketError(QLocalSocket::LocalSocketError error);
    void socketConnected();
    void socketDisconnected();

private:
    QString m_appUid;
    class QQuickView* m_view;
    QLocalSocket* m_socket;
    class QSharedMemory* m_shared;

};

#endif // FRAMESAVER_H
