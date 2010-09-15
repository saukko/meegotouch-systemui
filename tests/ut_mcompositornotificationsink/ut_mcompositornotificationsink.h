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

#ifndef UT_MCOMPOSITORNOTIFICATIONSINK_H
#define UT_MCOMPOSITORNOTIFICATIONSINK_H

#include <QObject>
#include <QList>
#include <QString>
#include "notificationmanagerinterface.h"
#include <QTimer>

class MApplication;
class MCompositorNotificationSink;

class MockNotificationManager : public QObject, public NotificationManagerInterface
{
    Q_OBJECT
public:
    MockNotificationManager();
    uint addNotification(uint notificationUserId, const NotificationParameters &parameters, uint groupId, int timeout);
    uint addNotification(uint notificationUserId, const NotificationParameters &parameters = NotificationParameters(), uint groupId = 0);
    bool updateNotification(uint notificationUserId, uint notificationId, const NotificationParameters &parameters = NotificationParameters());
    uint addGroup(uint notificationUserId, const NotificationParameters &parameters = NotificationParameters());
    bool updateGroup(uint notificationUserId, uint groupId, const NotificationParameters &parameters = NotificationParameters());
    bool removeGroup(uint notificationUserId, uint groupId);
    uint notificationUserId();
    QList<uint> notificationIdList(uint notificationUserId);
    QList<MNotificationProxy> notificationList(uint notificationUserId);
    QList<MNotificationGroupProxy> notificationGroupList(uint notificationUserId);

    uint nextAvailableNotificationID;
    QList<Notification> notifications;

public slots:
    bool removeNotification(uint notificationUserId, uint notificationId);

signals:
    void notificationRemoved(uint notificationId);
    void notificationUpdated(const Notification &notification);
};

class WindowEventFilter : public QObject
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

class Ut_MCompositorNotificationSink : public QTimer
{
    Q_OBJECT

public:
private:
    MApplication *app;
    MCompositorNotificationSink *sink;
    MockNotificationManager *notificationManager;
    NotificationParameters setupSinkDisabledTests(bool isSystemEvent = false);

signals:
    void statusIndictorMenuVisibilityChanged(bool);

private:
    // Helper for sending the display change event
    void sendOnDisplayChangeEvent();

private slots:
    // Executed once before every test case
    void init();
    // Executed once after every test case
    void cleanup();
    // Executed once before first test case
    void initTestCase();
    // Executed once after last test case
    void cleanupTestCase();

    // Test window properties
    void testNotificationWindowProperties();
    // Test that notifications can be added
    void testAddNotificationWhenWindowNotOpen();
    void testAddNotificationWhenWindowAlreadyOpen();
    // Test that updating with an ID updates the correct notification
    void testUpdateNotification();
    // Test that canceling with an ID cancels the correct notification
    void testRemoveNotification();
    // Test that notification windows are destroyed after a timeout
    void testTimeout();
    // Test that notifications are NOT added while recording
    void testNotificationWhileApplicationEventsDisabled();
    // Test when sink is set to disabled, no notifications are generated
    void testWhenSinkDisableTrueNoBannerCreated();
    // Test when sink is set to disabled, but system notifications are generated
    void testWhenSinkIsSetToDisabledSystemNotificationsAreStillGenerated();

    // Tests notification preview disabling with several possible values
    void testNotificationPreviewsDisabled_data();
    void testNotificationPreviewsDisabled();

    // Test that the window mask is set correctly in all orientations
    void testWindowMasking_data();
    void testWindowMasking();
};

#endif // UT_MCOMPOSITORNOTIFICATIONSINK_H
