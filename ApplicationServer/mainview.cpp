#include "mainview.h"
#include "windowhandler.h"

#include <QPaintEvent>
#include <QPainter>

MainView::MainView(WindowHandler* windowHandler, QWidget *parent) :
    QWidget(parent),
    m_windowHandler(windowHandler)
{
}

void MainView::mousePressEvent(QMouseEvent* event)
{
    m_windowHandler->mouseViewEvent(event);
}

void MainView::mouseReleaseEvent(QMouseEvent* event)
{
    m_windowHandler->mouseViewEvent(event);
}

void MainView::mouseDoubleClickEvent(QMouseEvent* event)
{
    m_windowHandler->mouseViewEvent(event);
}

void MainView::mouseMoveEvent(QMouseEvent* event)
{
    m_windowHandler->mouseViewEvent(event);
}

void MainView::wheelEvent(QWheelEvent* event)
{
    m_windowHandler->wheelViewEvent(event);
}

void MainView::keyPressEvent(QKeyEvent* event)
{
    m_windowHandler->keyViewEvent(event);
}

void MainView::keyReleaseEvent(QKeyEvent* event)
{
    m_windowHandler->keyViewEvent(event);
}

void MainView::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    m_windowHandler->paintViewEvent(event, &painter);
}
