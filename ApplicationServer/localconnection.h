#ifndef LOCALCONNECTION_H
#define LOCALCONNECTION_H

#include <QObject>
#include <QLocalSocket>
#include <QImage>
#include <QRect>

class LocalConnection : public QObject
{
    Q_OBJECT
public:
    explicit LocalConnection(QLocalSocket* socket, QObject *parent = 0);
    QRect geometry() const;
    void paintImage(class QPainter* painter);

protected slots:
    void socketAboutToClose();
    void socketReadyRead();
    void socketError(QLocalSocket::LocalSocketError error);

private:
    void UpdateView(QString appUid);
    void setGeometry(QString appUid, QRect geometry);

signals:
    void updateRequest();
    void connectionClosed();

public slots:

private: // data
    QLocalSocket* m_socket;
    class QSharedMemory* m_shared;
    QImage m_image;
    QRect m_geometry;

};

#endif // LOCALCONNECTION_H
