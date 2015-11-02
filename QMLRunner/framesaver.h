#ifndef FRAMESAVER_H
#define FRAMESAVER_H

#include <QObject>
#include <QLocalSocket>

class FrameSaver : public QObject
{
    Q_OBJECT
public:
    explicit FrameSaver(QString server, class QQuickView* view, QObject *parent = 0);

public slots:
    void save();
    void socketError(QLocalSocket::LocalSocketError error);

private:
    class QQuickView* m_view;
    QLocalSocket* m_socket;
    class QSharedMemory* m_shared;

};

#endif // FRAMESAVER_H
