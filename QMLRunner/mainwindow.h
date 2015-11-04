#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLocalSocket>
#include <QRect>

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QString appUid, QString mainQML, QString server, QWidget *parent = 0);
    virtual ~MainWindow();

protected:
    virtual void moveEvent(class QMoveEvent* event);
    virtual void resizeEvent(class QResizeEvent* event);

protected slots:
    void socketError(QLocalSocket::LocalSocketError error);
    void socketConnected();
    void socketDisconnected();
    void socketBytesWritten(qint64 bytes);
    void viewGeometryChanged();
    void quickViewFrameChanged();
    void qmlApplicationQuit();
    void quickViewBeforeRendering();

private: // data
    QString m_appUid;
    QLocalSocket* m_socket;
    class QQuickView* m_quickView;
    class QWidget* m_container;
    QRect m_geometry;
    class QSharedMemory* m_shared;
};

#endif // MAINWINDOW_H
