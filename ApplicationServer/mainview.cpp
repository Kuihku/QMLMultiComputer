#include "mainview.h"
#include "windowholder.h"

#include <QPaintEvent>
#include <QPainter>

MainView::MainView(WindowHolder* windowHolder, QWidget *parent) :
    QWidget(parent),
    m_windowHolder(windowHolder)
{

}

void MainView::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    m_windowHolder->paintWindows(event->rect(), event->region(), &painter);
}

