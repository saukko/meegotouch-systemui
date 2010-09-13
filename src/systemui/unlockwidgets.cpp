/* -*- Mode: C; indent-tabs-mode: s; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et ai sw=4 ts=4 sts=4: tw=80 cino="(0,W2s,i2s,t0,l1,:0" */
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
#include "unlockwidgets.h"

#include <QTime>
#include <QDateTime>

#include <QGraphicsLinearLayout>

#include <MLabel>
#include <MImageWidget>

#undef DEBUG
#include "debug.h"

UnlockHeader::UnlockHeader () :
    m_TimeLabel (0),
    m_DateLabel (0)
{
    QGraphicsLinearLayout   *datetimeBox;
    QGraphicsLinearLayout   *lockliftBox;

    setObjectName ("LockLiftArea");

    /*
     * The label that shows the hour and minute
     */
    m_TimeLabel = new MLabel;
    m_TimeLabel->setObjectName ("lockscreenTimeLabel");

    /*
     * The label that shows the date
     */
    m_DateLabel = new MLabel;
    m_DateLabel->setObjectName ("lockscreenDateLabel");

    /*
     * The lock icon @ right side
     */
    m_icon = new MImageWidget;
    m_icon->setVisible (true);
    m_icon->setImage ("icon-m-common-locked", QSize (32, 32));
    m_icon->setZoomFactor (1.0);
    m_icon->setObjectName ("lockscreenIconLocked");

    /*
     * Construct the Date/Time box
     */
    datetimeBox = new QGraphicsLinearLayout (Qt::Vertical);
    datetimeBox->setContentsMargins (0., 0., 0., 0.);
    datetimeBox->setSpacing (0.0);
    datetimeBox->addItem (m_TimeLabel);
    datetimeBox->setAlignment (m_TimeLabel, Qt::AlignLeft);
    datetimeBox->addItem (m_DateLabel);
    datetimeBox->setAlignment (m_DateLabel, Qt::AlignLeft);

    /*
     * Create the main container
     */
    lockliftBox = new QGraphicsLinearLayout (Qt::Horizontal);
    lockliftBox->setContentsMargins (0., 0., 0., 0.);
    lockliftBox->addItem (datetimeBox);
    lockliftBox->setAlignment (datetimeBox, Qt::AlignLeft | Qt::AlignTop);
    lockliftBox->addItem (m_icon);
    lockliftBox->setAlignment (m_icon, Qt::AlignRight | Qt::AlignVCenter);

    /*
     * And set the layout
     */
    setLayout (lockliftBox);

#ifdef HAVE_QMSYSTEM
    m_time = new Maemo::QmTime;

    connect (m_time, SIGNAL (timeOrSettingsChanged (Maemo::QmTimeWhatChanged)),
             this, SLOT (timeSettingsChanged (Maemo::QmTimeWhatChanged)));

    timeSettingsChanged (Maemo::QmTimeOnlySettingsChanged);
#endif

    /*
     * And initialize the widgets contents
     */
    m_locale.connectSettings ();
    updateDateTime ();
}

void
UnlockHeader::setActive (bool active)
{
    SYS_DEBUG ("active = %s", SYS_BOOL (active));
    m_icon->setVisible (active);

    update ();
}

UnlockHeader::~UnlockHeader ()
{
}

void
UnlockHeader::updateDateTime ()
{
    QString  text;
    bool     needUpdate = false;

    if ((m_TimeLabel == 0) || (m_DateLabel == 0))
        return;

    QDateTime now (QDateTime::currentDateTime ());

    /*
     * I suspect that the unnecessary updates are causing unnecessary damage
     * events. We got this method called in every seconds and we show the
     * minutes, so 59 of every 60 calls we should not update the screen.
     *
     * Please note, that this not prevents the updates to happen when the touch
     * screen is off, but it lower the updates by the ratio of 1/60.
     */
    text = m_locale.formatDateTime (now, MLocale::DateNone, MLocale::TimeShort);
    if (text != m_TimeLabel->text()) {
        m_TimeLabel->setText (text);
        needUpdate = true;
    }

    text = m_locale.formatDateTime (now, MLocale::DateFull, MLocale::TimeNone);
    if (text != m_DateLabel->text()) {
        m_DateLabel->setText (text);
        needUpdate = true;
    }

    if (needUpdate)
        update ();
}

#ifdef HAVE_QMSYSTEM
void
UnlockHeader::timeSettingsChanged (Maemo::QmTimeWhatChanged what)
{
    Q_UNUSED (what);
    bool format24h = m_time->getTimeFormat () == Maemo::QmTime::format24h;

    m_locale.setTimeFormat24h (
        format24h ?
        MLocale::TwentyFourHourTimeFormat24h :
        MLocale::TwelveHourTimeFormat24h);

    updateDateTime ();
}
#endif

UnlockArea::UnlockArea () :
    m_enabled (false),
    m_active (false)
{
    QGraphicsLinearLayout   *layout =
        new QGraphicsLinearLayout (Qt::Vertical);

    setObjectName ("LockLandArea");

    // Create the unlock icon
    MImageWidget    *unlock_icon;

    unlock_icon = new MImageWidget;
    unlock_icon->setImage ("icon-m-common-unlocked", QSize (32, 32));
    unlock_icon->setZoomFactor (1.0);
    unlock_icon->setObjectName ("LockScreenUnlockIcon");

    m_unlock_icon = new MWidget;
    QGraphicsLinearLayout   *icon_layout =
        new QGraphicsLinearLayout (Qt::Horizontal);
    icon_layout->addStretch ();
    icon_layout->addItem (unlock_icon);
    icon_layout->addStretch ();
    m_unlock_icon->setLayout (icon_layout);

    // Add the unlock icon centered
    layout->addStretch ();
    layout->addItem (m_unlock_icon);
    layout->addStretch ();

    setLayout (layout);

    updateState ();
}

UnlockArea::~UnlockArea ()
{
    // Free the resources here...
}

void
UnlockArea::setActive (bool active)
{
    SYS_DEBUG ("active = %s", SYS_BOOL (active));
    if (m_enabled == false)
        return; /* no-op when disabled... */

    if (m_active == active)
        return;

    m_active = active;

    updateState ();
}

void
UnlockArea::setEnabled (bool enabled)
{
    SYS_DEBUG ("enabled = %s", SYS_BOOL (enabled));
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    // Set active to false when enable val. has been changed...
    m_active = false;

    updateState ();
}

void
UnlockArea::updateState ()
{
    SYS_DEBUG ("");

    if (m_enabled == false)
    {
        setObjectName ("LockLandArea");
    }
    else if (m_active == true)
    {
        setObjectName ("LockLandAreaActive");
    }
    else
    {
        setObjectName ("LockLandAreaDragged");
    }

    m_unlock_icon->setVisible (m_enabled);
    m_unlock_icon->update ();

    // After object-name change a screen-refresh is needed:
    update ();
}
