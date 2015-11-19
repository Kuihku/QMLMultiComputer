#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QWidget>

class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(class WindowHandler* windowHandler, QWidget *parent = Q_NULLPTR);

protected:
    virtual void mousePressEvent(class QMouseEvent* event);
    virtual void mouseReleaseEvent(class QMouseEvent* event);
    virtual void mouseDoubleClickEvent(class QMouseEvent* event);
    virtual void mouseMoveEvent(class QMouseEvent* event);
    virtual void wheelEvent(class QWheelEvent* event);
    virtual void keyPressEvent(class QKeyEvent* event);
    virtual void keyReleaseEvent(class QKeyEvent* event);
    virtual void paintEvent(class QPaintEvent* event);

signals:

public slots:

private:
    class WindowHandler* m_windowHandler;
};

#endif // MAINVIEW_H
