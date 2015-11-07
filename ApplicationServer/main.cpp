#include <QApplication>

#include "server.h"

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    QStringList args(a.arguments());

    QString configFile(SERVER_CONFIG);

    if (args.count() > 1) {
        configFile = args.at(1);
    }

    Server server(configFile);

    return a.exec();
}
