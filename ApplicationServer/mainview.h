#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QWidget>

class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(class WindowHolder* windowHolder, QWidget *parent = Q_NULLPTR);

protected:
    virtual void paintEvent(class QPaintEvent* event);

signals:

public slots:

private:
    class WindowHolder* m_windowHolder;
};

#endif // MAINVIEW_H
