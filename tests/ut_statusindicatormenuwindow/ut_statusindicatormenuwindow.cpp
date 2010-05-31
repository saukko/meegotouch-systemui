/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of systemui.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "ut_statusindicatormenuwindow.h"
#include "statusindicatormenuwindow.h"
#include "pluginlist_stub.h"
#include <MApplication>
#include <MApplicationIfProxy>
#include <MSceneManager>
#include <QtTest/QtTest>
#include <MOverlay>
#include <MButton>
#include <QGraphicsLinearLayout>
#include "notificationarea_stub.h"
#include <MPannableViewport>
#include "x11wrapper_stub.h"

// MApplicationIfProxy stubs (used by StatusIndicatorMenuWindow)
bool mApplicationIfProxyLaunchCalled = false;
QDBusPendingReply<> MApplicationIfProxy::launch()
{
    mApplicationIfProxyLaunchCalled = true;

    return QDBusPendingReply<>();
}

QPair<void*, bool> gSetVisible(0, false);
void QWidget::setVisible(bool visible)
{
    gSetVisible.first = this;
    gSetVisible.second = visible;
}

void Ut_StatusIndicatorMenuWindow::init()
{
    gX11WrapperStub->stubReset();    
    statusIndicatorMenuWindow = new StatusIndicatorMenuWindow;
    gSetVisible.first = 0;
    gSetVisible.second = false;
    mApplicationIfProxyLaunchCalled = false;
    connect(this, SIGNAL(settingsButtonClicked()), statusIndicatorMenuWindow, SLOT(launchControlPanelAndHide()));
    connect(this, SIGNAL(clicked(QPointF)), statusIndicatorMenuWindow, SLOT(hideIfPointBeyondMenu(QPointF)));
    connect(this, SIGNAL(positionOrSizeChanged()), statusIndicatorMenuWindow, SLOT(setPannabilityAndLayout()));
}

void Ut_StatusIndicatorMenuWindow::cleanup()
{
    delete statusIndicatorMenuWindow;
}

void Ut_StatusIndicatorMenuWindow::initTestCase()
{
    int argc = 1;
    char *app_name = (char *)"./ut_statusarea";
    app = new MApplication(argc, &app_name);
}

void Ut_StatusIndicatorMenuWindow::cleanupTestCase()
{
    delete app;
}

void Ut_StatusIndicatorMenuWindow::testMakeVisible()
{
    statusIndicatorMenuWindow->hide();
    QVERIFY(gSetVisible.first == statusIndicatorMenuWindow && !gSetVisible.second);
    statusIndicatorMenuWindow->makeVisible();
    QVERIFY(gSetVisible.first == statusIndicatorMenuWindow && gSetVisible.second);
}

void Ut_StatusIndicatorMenuWindow::testCloseButtonOverlay()
{
    QVERIFY(statusIndicatorMenuWindow->closeButtonOverlay);
    QCOMPARE(statusIndicatorMenuWindow->closeButtonOverlay->objectName(), QString("CloseButtonOverlay"));

    // Check the number of children of overlay and its layout
    QCOMPARE(statusIndicatorMenuWindow->closeButtonOverlay->childItems().count(), 4);
    QCOMPARE(statusIndicatorMenuWindow->closeButtonOverlay->layout()->count(), 3);

    // Check the type of the children of the overlay
    QVERIFY(dynamic_cast<EventEaterWidget *> (statusIndicatorMenuWindow->closeButtonOverlay->layout()->itemAt(0)));
    QVERIFY(dynamic_cast<MButton *> (statusIndicatorMenuWindow->closeButtonOverlay->layout()->itemAt(1)));
    QVERIFY(dynamic_cast<EventEaterWidget *> (statusIndicatorMenuWindow->closeButtonOverlay->layout()->itemAt(2)));

    // Test status indicator menu window visibility
    QVERIFY(gSetVisible.first != statusIndicatorMenuWindow && !gSetVisible.second);
    connect(this, SIGNAL(clicked()), statusIndicatorMenuWindow, SLOT(hide()));
    emit clicked();
    QVERIFY(gSetVisible.first == statusIndicatorMenuWindow && !gSetVisible.second);

    // Test that status indicator menu close overlay accept events outside close button
    EventEaterWidget *overlayWidget = new EventEaterWidget;
    QGraphicsSceneMouseEvent event;
    overlayWidget->mousePressEvent(&event);
    QCOMPARE(event.isAccepted(), true);
    delete overlayWidget;
}

void Ut_StatusIndicatorMenuWindow::testSettingsButtonClicked()
{
    emit settingsButtonClicked();

    QVERIFY(mApplicationIfProxyLaunchCalled);
}

void Ut_StatusIndicatorMenuWindow::testHideIfPointBeyondMenu()
{
    QVERIFY(gSetVisible.first != statusIndicatorMenuWindow && !gSetVisible.second);
    emit clicked(QPointF(0, 0));
    QVERIFY(gSetVisible.first != statusIndicatorMenuWindow && !gSetVisible.second);
    emit clicked(QPointF(statusIndicatorMenuWindow->geometry().width(), statusIndicatorMenuWindow->geometry().height()));
    QVERIFY(gSetVisible.first == statusIndicatorMenuWindow && !gSetVisible.second);
}

void Ut_StatusIndicatorMenuWindow::testCloseButtonPosition()
{
    // When the pannable viewport has not been panned the close button overlay should not be visible
    statusIndicatorMenuWindow->pannableViewport->widget()->setPos(0, 0);
    QCOMPARE(statusIndicatorMenuWindow->closeButtonOverlay->isVisible(), false);

    // When the pannable viewport has not been panned outside the screen the close button overlay should be visible
    statusIndicatorMenuWindow->pannableViewport->widget()->setPos(0, statusIndicatorMenuWindow->geometry().height());
    QCOMPARE(statusIndicatorMenuWindow->closeButtonOverlay->isVisible(), true);
}

void Ut_StatusIndicatorMenuWindow::testSetPannability()
{
    qreal pannedWidgetHeight = statusIndicatorMenuWindow->pannableViewport->widget()->geometry().height();

    // When the pannable viewport is taller than the panned widget there is no need to pan and the pannability should be disabled
    statusIndicatorMenuWindow->pannableViewport->setGeometry(QRectF(0, 0, 1, pannedWidgetHeight + 1));
    emit positionOrSizeChanged();
    QCOMPARE(statusIndicatorMenuWindow->pannableViewport->isEnabled(), false);

    // When the pannable viewport is not taller than the panned widget there is a need to pan and the pannability should be enabled
    statusIndicatorMenuWindow->pannableViewport->setGeometry(QRectF(0, 0, 1, pannedWidgetHeight - 1));
    emit positionOrSizeChanged();
    QCOMPARE(statusIndicatorMenuWindow->pannableViewport->isEnabled(), true);
}

void Ut_StatusIndicatorMenuWindow::testWindowType()
{
    QVERIFY(statusIndicatorMenuWindow->testAttribute(Qt::WA_X11NetWmWindowTypeMenu));
}

void Ut_StatusIndicatorMenuWindow::testPannableAreaBackgroundWidget()
{
    // When the pannable viewport has not been panned the background height should be 0
    statusIndicatorMenuWindow->pannableViewport->widget()->setPos(0, 0);
    emit positionOrSizeChanged();
    QCOMPARE(statusIndicatorMenuWindow->backgroundWidget->minimumHeight(), qreal(0));
    QCOMPARE(statusIndicatorMenuWindow->backgroundWidget->maximumHeight(), qreal(0));

    // When the pannable viewport has been panned the background height should reach the bottom of the screen
    statusIndicatorMenuWindow->pannableViewport->widget()->setPos(0, 10);
    emit positionOrSizeChanged();
    qreal expectedHeight = statusIndicatorMenuWindow->sceneManager()->visibleSceneSize().height() - statusIndicatorMenuWindow->pannableViewport->mapToItem(statusIndicatorMenuWindow->sceneWindow.data(), QPointF()).y();
    QCOMPARE(statusIndicatorMenuWindow->backgroundWidget->minimumHeight(), expectedHeight);
    QCOMPARE(statusIndicatorMenuWindow->backgroundWidget->maximumHeight(), expectedHeight);
}

QTEST_APPLESS_MAIN(Ut_StatusIndicatorMenuWindow)
