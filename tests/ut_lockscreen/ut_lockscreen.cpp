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

#include "lockscreen.h"
#include "ut_lockscreen.h"
#include <MApplication>
#include <MWindow>

void Ut_LockScreen::init()
{
    parent = new MWindow;
    lockScreen = new LockScreen(parent);
}

void Ut_LockScreen::cleanup()
{
    delete lockScreen;
    delete parent;
}

void Ut_LockScreen::initTestCase()
{
    static int argc = 1;
    static char *app_name = (char *)"./ut_lockscreen";
    app = new MApplication(argc, &app_name);

}

void Ut_LockScreen::cleanupTestCase()
{
    delete app;
}

void Ut_LockScreen::testSliderUnlocked()
{
    connect(this, SIGNAL(unlocked()), lockScreen, SLOT(sliderUnlocked()));
    QSignalSpy spy(lockScreen, SIGNAL(unlocked()));
    emit unlocked();
    QCOMPARE(spy.count(), 1);
}

QTEST_APPLESS_MAIN(Ut_LockScreen)