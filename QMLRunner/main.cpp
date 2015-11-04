#include <QObject>
#include <QApplication>

#include <QStringList>
#include <QDir>

#include <QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    qDebug() << "QMLRunner - start";
    QApplication app(argc, argv);

    QStringList args(app.arguments());

    qDebug() << "QMLRunner - args:" << args;

    if (args.size() == 5) {
        QString appUid(args.at(1));
        QString qmlPath(args.at(2));
        QString mainQML(args.at(3));
        QString server(args.at(4));

        qDebug() << "appUid:" << appUid << "- qmlPath:" << qmlPath << "- mainQML:" << mainQML << "- server:" << server;

        if (!QDir::setCurrent(qmlPath)) return -2;

        MainWindow mainWindow(appUid, mainQML, server);
/*
        mainWindow.setAttribute(Qt::WA_DontShowOnScreen);

        QQuickView quickView;

        FrameSaver saver(appUid, server, &quickView);
        QObject::connect(&mainWindow, SIGNAL(geometryChanged(const QRect&)), &saver, SLOT(geometryChanged(const QRect&)));

        QObject::connect(quickView.engine(), SIGNAL(quit()), &app, SLOT(quit()));

        QWidget* container = QWidget::createWindowContainer(&quickView, &mainWindow);

        quickView.setSource(QUrl::fromLocalFile(mainQML));
        qDebug("quickView size:( %d, %d )", quickView.width(), quickView.height());

        QSize quickViewSize(quickView.size());

        container->resize(quickViewSize);
        mainWindow.resize(quickViewSize);

        container->setParent(&mainWindow);

        quickView.show();
*/
        mainWindow.show();

        int returnValue(app.exec());
        qDebug() << "QMLRunner - END, return value:" << returnValue;
        return returnValue;
    }
    qWarning("Startup: QMLRunner appUid qmlPath mainQML server");
    return -1;
}

