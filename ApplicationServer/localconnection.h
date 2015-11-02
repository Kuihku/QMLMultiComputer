#ifndef LOCALCONNECTION_H
#define LOCALCONNECTION_H

#include <QObject>

class LocalConnection : public QObject
{
    Q_OBJECT
public:
    explicit LocalConnection(QObject *parent = 0);

signals:

public slots:
};

#endif // LOCALCONNECTION_H
