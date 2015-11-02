#include <QApplication>
#include <QUrl>
#include <QQuickView>
#include <QQmlEngine>
#include <QWindow>
#include <QMainWindow>
#include <QStringList>
#include <QDir>

#include <QDebug>


#include "framesaver.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList args(app.arguments());

    if (args.size() == 5) {
        QString appUid(args.at(1));
        QString qmlPath(args.at(2));
        QString mainQML(args.at(3));
        QString server(args.at(4));

        qDebug() << "appUid:" << appUid << "- qmlPath:" << qmlPath << "- mainQML:" << mainQML << "- server:" << server;

        if (!QDir::setCurrent(qmlPath)) return -2;

        QMainWindow mainWindow;

        mainWindow.setAttribute(Qt::WA_DontShowOnScreen);

        QQuickView quickView;

        FrameSaver saver(appUid, server, &quickView);

        QObject::connect(quickView.engine(), SIGNAL(quit()), &app, SLOT(quit()));

        QWidget* container = QWidget::createWindowContainer(&quickView, &mainWindow);

        quickView.setSource(QUrl::fromLocalFile(mainQML));
        qDebug("quickView size:( %d, %d )", quickView.width(), quickView.height());

        QSize quickViewSize(quickView.size());

        container->resize(quickViewSize);
        mainWindow.resize(quickViewSize);

        container->setParent(&mainWindow);

        quickView.show();

        mainWindow.show();

        return app.exec();
    }
    return -1;
}

