/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Connection status tracking (icon, tooltip, notifications) for the lxqt-nm-tray panel plugin.
 * Adapted from nm-tray's src/tray.cpp, Copyright (c) 2015~now Palo Kisa <palo.kisa@gmail.com>,
 * with the QSystemTrayIcon parts removed.
 */
#pragma once

#include <QObject>
#include <QPersistentModelIndex>
#include <QTimer>

#include "icons.h"
#include "nmproxy.h"
#include "nmtraysettings.h"
#include "org_freedesktop_notifications.h"

class NmModel;

class NmTrayStatus : public QObject
{
    Q_OBJECT

public:
    explicit NmTrayStatus(NmModel *model, QObject *parent = nullptr);

    icons::Icon icon() const { return mIconCurrent; }
    QString toolTip() const { return mToolTip; }

    bool notificationsEnabled() const { return mNotificationsEnabled; }
    void setNotificationsEnabled(bool enabled);

Q_SIGNALS:
    void iconChanged(icons::Icon icon);
    void toolTipChanged(const QString &toolTip);

private:
    void updateState(const QModelIndex &index, bool removing);
    void primaryConnectionUpdate();
    void setShown(const QPersistentModelIndex &index);
    void updateIcon();
    void notify(const QModelIndex &index, bool removing);
    void setToolTip(const QString &toolTip);

    NmProxy mActiveConnections;
    QPersistentModelIndex mPrimaryConnection;
    QPersistentModelIndex mShownConnection;
    QList<QPersistentModelIndex> mConnectionsToNotify;
    icons::Icon mIconCurrent;
    icons::Icon mIcon2Show;
    QTimer mIconTimer;
    QString mToolTip;
    NmTraySettings mSettings;
    bool mNotificationsEnabled;
    org::freedesktop::Notifications mNotification;
};
