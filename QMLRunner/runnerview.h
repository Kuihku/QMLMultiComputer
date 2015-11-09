#ifndef RUNNERVIEW_H
#define RUNNERVIEW_H

#include <QQuickWidget>
#include <QLocalSocket>

#include "message.h"

class RunnerView : public QQuickWidget
{
    Q_OBJECT
public:
    explicit RunnerView(QString appUid, QString server, QWidget* parent = 0);

protected slots:
    void save();
    void socketError(QLocalSocket::LocalSocketError error);
    void socketConnected();
    void socketDisconnected();
    void socketBytesWritten(qint64 bytes);
    void readSocket();
    void quitApplication();

protected:
    virtual void paintEvent(class QPaintEvent* event);
    virtual void resizeEvent(class QResizeEvent* event);
    virtual void moveEvent(class QMoveEvent* event);

private:
    void handleCloneRequest();
    void setItemToMessage(CloneDataMessage& cdm, class QQuickItem* item, int index = 0);

private:
    QString m_appUid;
    QLocalSocket* m_socket;
    class QSharedMemory* m_shared;

};

#endif // RUNNERVIEW_H
