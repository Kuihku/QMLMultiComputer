#include <QApplication>
#include <QUrl>
#include <QStringList>
#include <QDir>

#include <QDebug>

#include "runnerview.h"

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

        RunnerView mainWindow(appUid, server);
        mainWindow.setAttribute(Qt::WA_DontShowOnScreen);
        mainWindow.setSource(QUrl::fromLocalFile(mainQML));
        mainWindow.show();

        int returnValue(app.exec());
        qDebug() << "QMLRunner - END, return value:" << returnValue;
        return returnValue;
    }
    qWarning("Startup: QMLRunner appUid qmlPath mainQML server");
    return -1;
}

