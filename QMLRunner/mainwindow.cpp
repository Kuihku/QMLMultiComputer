#include "mainwindow.h"
#include "message.h"

#include <QQuickView>
#include <QQmlEngine>
#include <QWindow>
#include <QSharedMemory>
#include <QUrl>
#include <QImage>
#include <QBuffer>
#include <QCoreApplication>
#include <QScreen>

#include <QMoveEvent>
#include <QResizeEvent>

#include <QDebug>

#include <QDateTime>

MainWindow::MainWindow(QString appUid, QString mainQML, QString server, QWidget* parent) :
    QWidget(parent),
    m_appUid(appUid),
    m_socket(new QLocalSocket(this)),
    m_quickView(new QQuickView),
    m_container(NULL),
    m_shared(new QSharedMemory(appUid, this))
{
    m_quickView->setResizeMode(QQuickView::SizeViewToRootObject);
    setAttribute(Qt::WA_DontShowOnScreen);
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
//    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketBytesWritten(qint64)));

    connect(m_quickView, SIGNAL(beforeRendering()), this, SLOT(quickViewBeforeRendering()));
    connect(m_quickView, SIGNAL(frameSwapped()), this, SLOT(quickViewFrameChanged()));

    QObject::connect(m_quickView->engine(), SIGNAL(quit()), this, SLOT(qmlApplicationQuit()));

    m_container = QWidget::createWindowContainer(m_quickView, this);

    m_quickView->setSource(QUrl::fromLocalFile(mainQML));
    // qDebug("quickView size:( %d, %d )", m_quickView->width(), m_quickView->height());

    m_quickView->setPosition(100, 100);
    m_container->move(100, 100);
    move(100, 100);
    connect(m_quickView, SIGNAL(widthChanged(int)), this, SLOT(viewGeometryChanged()));
    connect(m_quickView, SIGNAL(heightChanged(int)), this, SLOT(viewGeometryChanged()));
    connect(m_quickView, SIGNAL(xChanged(int)), this, SLOT(viewGeometryChanged()));
    connect(m_quickView, SIGNAL(yChanged(int)), this, SLOT(viewGeometryChanged()));

//    QSize quickViewSize(m_quickView->size());
//    m_container->resize(quickViewSize);
//    resize(quickViewSize);

    // m_container->setParent(this);
    // viewGeometryChanged();
    m_socket->connectToServer(server);
    m_quickView->show();

//    qDebug() << "m_quickView->format.swapBehavior:" << (int) m_quickView->format().swapBehavior();
}

MainWindow::~MainWindow()
{
    delete m_quickView;
}


void MainWindow::socketError(QLocalSocket::LocalSocketError error)
{
    qDebug() << "MainWindow::socketError - error:" << error;
}

void MainWindow::socketConnected()
{
    qDebug() << "MainWindow::socketConnected";
    viewGeometryChanged();
}

void MainWindow::socketDisconnected()
{
    qDebug() << "MainWindow::socketDisconnected";
    m_shared->detach();
}

void MainWindow::socketBytesWritten(qint64 bytes)
{
    qDebug() << "MainWindow::socketBytesWritten - bytes:" << bytes;
}

void MainWindow::moveEvent(QMoveEvent*)
{
//    qDebug() << "MainWindow::moveEvent - Container pos:" << m_container->pos();
}

void MainWindow::resizeEvent(QResizeEvent*)
{
//    qDebug() << "MainWindow::resizeEvent - Container size:" << m_container->size();
}

void MainWindow::viewGeometryChanged()
{
    QSize quickViewSize(m_quickView->size());
    QPoint quickViewPos(m_quickView->position());

    qDebug() << "MainWindow::viewGeometryChanged - pos:" << quickViewPos << "- size:" << quickViewSize;

    m_container->resize(quickViewSize);
    resize(quickViewSize);
    m_container->move(quickViewPos);
    move(quickViewPos);

    if (m_socket->isWritable()) {
        QRect newGeometry(m_quickView->geometry());
        if (m_geometry != newGeometry) {
            m_geometry = newGeometry;
            GeometryMessage gm(m_appUid, m_geometry);
            gm.write(m_socket);
        }
    }
}

void MainWindow::quickViewFrameChanged()
{
    QImage image(m_quickView->grabWindow());

//    QScreen* s(m_quickView->screen());
//    QPixmap image(s->grabWindow(m_quickView->winId()));

    qDebug() << "MainWindow::quickViewFrameChanged - image size:" << image.size();

    if (m_socket->isWritable()) {
        QBuffer buffer;
        buffer.open(QBuffer::WriteOnly);
        QDataStream out(&buffer);
        out << image;
        int size(buffer.size());

        if (m_shared->isAttached()) {
            if (m_shared->size() < size && !m_shared->detach()) {
                return;
            }
        }
        if (!m_shared->isAttached()) {
            if (!m_shared->create(size)) {
                qWarning("FrameSaver::save - m_shared create error");
                return;
            }

        }
        m_shared->lock();
        char* to((char*)m_shared->data());
        const char* from(buffer.data().data());
        memcpy(to, from, qMin(m_shared->size(), size));
        m_shared->unlock();

        Message message(m_appUid, MessageType::Update);
        message.write(m_socket);

        QString fileName(QDateTime::currentDateTime().toString("yyyy.MM.dd.HH.mm.ss.zzz.png"));
        fileName.prepend("images/");
        qDebug() << "FrameSaver::save - fileName:" << fileName.toLatin1().constData();
        if (!image.save(fileName, "PNG")) {
            qDebug() << "FrameSaver::save - Error, fileName:" << fileName.toLatin1().constData();
        }

    }

}

void MainWindow::qmlApplicationQuit()
{
    qDebug("MainWindow::qmlApplicationQuit");
//    m_shared->detach();
    m_socket->close();
    qApp->quit();
}

void MainWindow::quickViewBeforeRendering()
{
    // m_quickView->resetOpenGLState();
    // glClearStencil(0);
    // glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

