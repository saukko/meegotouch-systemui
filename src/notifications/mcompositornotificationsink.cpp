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

#include <QGraphicsScene>
#include <QTimer>
#include <MInfoBanner>
#include <MWindow>
#include <MScene>
#include <MSceneManager>
#include <MOrientationChangeEvent>
#include "notificationmanager.h"
#include "mcompositornotificationsink.h"
#include "notificationwidgetparameterfactory.h"
#include "sysuid.h"

MCompositorNotificationSink::MCompositorNotificationSink() :
    orientationChangeSignalConnected(false),
    sinkDisabled(false)
{
}

MCompositorNotificationSink::~MCompositorNotificationSink()
{
    // Destroy the remaining notifications
    foreach(uint id, idToNotification.keys()) {
        notificationDone(id, false);
    }
}

void MCompositorNotificationSink::addNotification(const Notification &notification)
{
    if (!canAddNotification(notification)) return;
    if(sinkDisabled) {
        emit notificationAdded(notification);
        return;
    }
    if (!orientationChangeSignalConnected) {
        // Get informed about orientation changes
        connect(Sysuid::sysuid(), SIGNAL(orientationChangeFinished(const M::Orientation &)), this, SLOT(rotateInfoBanners(const M::Orientation &)));
        orientationChangeSignalConnected = true;
    }

    if (idToNotification.contains(notification.notificationId())) {
        // The notification already exists so update it
        updateNotification(notification);
    } else {
        // Create a new toplevel window for the info banner
        QGraphicsScene *scene = new QGraphicsScene;
        QGraphicsView *view = new QGraphicsView(scene);
        view->setFrameStyle(0);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setAttribute(Qt::WA_X11NetWmWindowTypeNotification);

        // Create a timer for the info banner; make it a child of the window so it is destroyed automatically
        QTimer *timer = new QTimer(view);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
        timer->setProperty("notificationId", notification.notificationId());

        // Create info banner widget
        MInfoBanner *infoBanner = createInfoBanner(notification);
        infoBanner->setManagedManually(true);

        // Rotate the view to the current orientation and make the view as big as the info banner and
        setViewSizeAndRotation(*view, *infoBanner);
        view->show();

        // Add the info banner widget to the scene
        scene->addItem(infoBanner);

        // Keep track of the mapping between IDs and private notification information classes
        idToNotification.insert(notification.notificationId(), new MCompositorNotificationSinkNotification(view, timer, infoBanner));

        // Make the info banner disappear after the timeout
        timer->start(notification.timeout());
        emit notificationAdded(notification);
    }
}

void MCompositorNotificationSink::updateNotification(const Notification &notification)
{
    MCompositorNotificationSinkNotification *sinkNotification = idToNotification.value(notification.notificationId());

    if (sinkNotification != NULL && sinkNotification->infoBanner != NULL) {
        // Update the info banner widget
        sinkNotification->infoBanner->setImageID(notification.parameters().value(NotificationWidgetParameterFactory::imageIdKey()).toString());
        sinkNotification->infoBanner->setBodyText(notification.parameters().value(NotificationWidgetParameterFactory::bodyKey()).toString());
        sinkNotification->infoBanner->setIconID(determineIconId(notification.parameters()));

        // Update the info banner's actions
        updateActions(sinkNotification->infoBanner, notification.parameters());
    }
}

void MCompositorNotificationSink::removeNotification(uint notificationId)
{
    notificationDone(notificationId, false);
}

void MCompositorNotificationSink::notificationDone(uint notificationId, bool notificationIdInUse)
{
    MCompositorNotificationSinkNotification *sinkNotification = idToNotification.take(notificationId);
    if (sinkNotification != NULL) {
        // Destroy the notification window immediately; this also destroys the timer
        QGraphicsScene *scene = sinkNotification->view->scene();
        delete sinkNotification->view;

        // Remove the info banner from the scene and destroy it
        scene->removeItem(sinkNotification->infoBanner);
        delete sinkNotification->infoBanner;

        // Destroy the scene and the notification information
        delete scene;
        delete sinkNotification;
    }

    if (notificationIdInUse) {
        idToNotification.insert(notificationId, NULL);
    }
}

void MCompositorNotificationSink::timeout()
{
    QTimer *timer = qobject_cast<QTimer *>(sender());

    if (timer != NULL) {
        // Get the notification ID from the timer
        bool ok = false;
        uint notificationId = timer->property("notificationId").toUInt(&ok);

        if (ok) {
            // Remove the notification
            notificationDone(notificationId, true);
        }
    }
}

MCompositorNotificationSink::MCompositorNotificationSinkNotification::MCompositorNotificationSinkNotification(QGraphicsView *view, QTimer *timer, MInfoBanner *infoBanner) :
    view(view),
    timer(timer),
    infoBanner(infoBanner)
{
}

MCompositorNotificationSink::MCompositorNotificationSinkNotification::~MCompositorNotificationSinkNotification()
{
}

void MCompositorNotificationSink::rotateInfoBanners(const M::Orientation &orientation)
{
    foreach(MCompositorNotificationSinkNotification * notification, idToNotification.values()) {
        // Rotate the views of all existing notifications to the current orientation and resize their sizes accordingly
        if (notification != NULL && notification->view != NULL && notification->infoBanner != NULL) {
            QGraphicsScene *scene = notification->view->scene();
            if (scene != NULL) {
                // Notify each item on the notification view's scene about the new orientation
                MOrientationChangeEvent event(orientation);
                Q_FOREACH(QGraphicsItem * item, scene->items()) {
                    scene->sendEvent(item, &event);
                }
            }

            setViewSizeAndRotation(*notification->view, *notification->infoBanner);
        }
    }
}

void MCompositorNotificationSink::setViewSizeAndRotation(QGraphicsView &view, const MInfoBanner &infoBanner)
{
    QTransform transform;
    transform.rotate(Sysuid::sysuid()->orientationAngle());
    view.setTransform(transform);
    if (Sysuid::sysuid()->orientation() == M::Landscape) {
        view.setFixedSize(infoBanner.preferredSize().width(), infoBanner.preferredSize().height());
    } else {
        view.setFixedSize(infoBanner.preferredSize().height(), infoBanner.preferredSize().width());
    }
}

void MCompositorNotificationSink::setDisabled(bool disabled)
{
    sinkDisabled = disabled;
}