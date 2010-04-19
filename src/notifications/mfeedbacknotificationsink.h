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

#ifndef MFEEDBACKNOTIFICATIONSINK_H_
#define MFEEDBACKNOTIFICATIONSINK_H_

#include "notificationsink.h"

/*!
 * MFeedbackNotificationSink implements the NotificationSink interface for
 * presenting notifications as feedbacks.
 */
class MFeedbackNotificationSink : public NotificationSink
{
    Q_OBJECT
public:
    /*!
     * Constructs a new MFeedbackNotificationSink.
     */
    MFeedbackNotificationSink();

    /*!
     * Destroys the MFeedbackNotificationSink.
     */
    virtual ~MFeedbackNotificationSink();

private:
    /*!
     * Determines feedback id of a notification based on the given notification parameters.
     * \param parameters Notification parameters to determine the feedback id from.
     * \return Logical feedback id as a string
     */
    QString determineFeedbackId(const NotificationParameters &parameters);

    /*!
     * Helper method to return the NotificationManager used.
     */
    NotificationManager &notificationManager();

private slots:
    //! \reimp
    virtual void addNotification(const Notification &notification);
    virtual void removeNotification(uint notificationId);
    //! \reimp_end
};

#endif /* MFEEDBACKNOTIFICATIONSINK_H_ */