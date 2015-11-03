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
    qDebug() << "QMLRunner - start";
    QApplication app(argc, argv);

    QStringList args(app.arguments());

    qDebug() << "QMLRunner - args:" << args;

    if (args.size() == 5) {
        qDebug() << "QMLRunner - step 1";
        QString appUid(args.at(1));
        QString qmlPath(args.at(2));
        QString mainQML(args.at(3));
        QString server(args.at(4));

        qDebug() << "appUid:" << appUid << "- qmlPath:" << qmlPath << "- mainQML:" << mainQML << "- server:" << server;

        if (!QDir::setCurrent(qmlPath)) return -2;

        qDebug() << "QMLRunner - step 2";
        QMainWindow mainWindow;

        qDebug() << "QMLRunner - step 3";
        mainWindow.setAttribute(Qt::WA_DontShowOnScreen);

        qDebug() << "QMLRunner - step 4";
        QQuickView quickView;

        qDebug() << "QMLRunner - step 5";
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

        int returnValue(app.exec());
        qDebug() << "QMLRunner - END, return value:" << returnValue;
        return returnValue;
    }
    qWarning("Startup: QMLRunner appUid qmlPath mainQML server");
    return -1;
}

