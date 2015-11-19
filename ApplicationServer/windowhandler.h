#ifndef WINDOWHANDLER
#define WINDOWHANDLER

class WindowHandler {
public:
    virtual void mouseViewEvent(class QMouseEvent*) = 0;
    virtual void wheelViewEvent(class QWheelEvent*) = 0;
    virtual void keyViewEvent(class QKeyEvent*) = 0;
    virtual void paintViewEvent(class QPaintEvent*, class QPainter*) = 0;
};

#endif // WINDOWHANDLER

