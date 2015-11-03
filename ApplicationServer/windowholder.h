#ifndef WINDOWHOLDER
#define WINDOWHOLDER

#include <QRect>
#include <QRegion>

class WindowHolder {
public:
    virtual void paintWindows(QRect, QRegion, class QPainter*) = 0;
};

#endif // WINDOWHOLDER

