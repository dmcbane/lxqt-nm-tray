/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Adapted from nm-tray's src/tray.cpp, Copyright (c) 2015~now Palo Kisa <palo.kisa@gmail.com>.
 */
#include "nmtraystatus.h"

#include <QCoreApplication>
#include <QItemSelection>

#include <NetworkManagerQt/Manager>

#include "nmmodel.h"
#include "statustext.h"

namespace
{
// Reuse nm-tray's "Tray" translation context so its installed .qm files apply.
QString trTray(const char *text)
{
    return QCoreApplication::translate("Tray", text);
}
}

NmTrayStatus::NmTrayStatus(NmModel *model, QObject *parent)
    : QObject{parent}
    , mIconCurrent{static_cast<icons::Icon>(-1)}
    , mIcon2Show{icons::NETWORK_OFFLINE}
    , mNotificationsEnabled{mSettings.notificationsEnabled()}
    , mNotification{QStringLiteral("org.freedesktop.Notifications"),
                    QStringLiteral("/org/freedesktop/Notifications"),
                    QDBusConnection::sessionBus()}
{
    mActiveConnections.setNmModel(model, NmModel::ActiveConnectionType);

    // postpone icon updates so a burst of state changes yields one repaint
    mIconTimer.setSingleShot(true);
    mIconTimer.setInterval(0);
    connect(&mIconTimer, &QTimer::timeout, this, &NmTrayStatus::updateIcon);

    setShown(QModelIndex{});
    mIconCurrent = mIcon2Show;
    setToolTip(trTray("<pre>No active connection</pre>"));

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged, this,
            [this](const QString &) { primaryConnectionUpdate(); });

    connect(&mActiveConnections, &QAbstractItemModel::rowsInserted, this,
            [this](const QModelIndex &parent, int first, int last) {
                for (int i = first; i <= last; ++i)
                {
                    const QModelIndex index = mActiveConnections.index(i, 0, parent);
                    if (mNotificationsEnabled)
                        mConnectionsToNotify.append(index);
                    updateState(index, false);
                }
            });
    connect(&mActiveConnections, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            [this](const QModelIndex &parent, int first, int last) {
                for (int i = first; i <= last; ++i)
                    updateState(mActiveConnections.index(i, 0, parent), true);
            });
    connect(&mActiveConnections, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &) {
                const auto indexes = QItemSelection{topLeft, bottomRight}.indexes();
                for (const auto &i : indexes)
                    updateState(i, false);
            });

    // NetworkManager::Notifier doesn't emit at startup, so pull the initial state.
    primaryConnectionUpdate();
}

void NmTrayStatus::setNotificationsEnabled(bool enabled)
{
    mNotificationsEnabled = enabled;
    mSettings.setNotificationsEnabled(enabled);
}

void NmTrayStatus::updateState(const QModelIndex &index, bool removing)
{
    notify(index, removing);

    const auto state = static_cast<NetworkManager::ActiveConnection::State>(
        mActiveConnections.data(index, NmModel::ActiveConnectionStateRole).toInt());
    const bool is_primary = mPrimaryConnection == index;

    if (removing || NetworkManager::ActiveConnection::Deactivated == state
        || NetworkManager::ActiveConnection::Deactivating == state)
    {
        if (is_primary)
        {
            mPrimaryConnection = QModelIndex{};
            setShown(mPrimaryConnection);
        }
        else if (mShownConnection == index)
        {
            setShown(mPrimaryConnection);
        }
    }
    else
    {
        if (is_primary || NetworkManager::ActiveConnection::Activating == state)
            setShown(index);
        else if (mShownConnection == index)
            setShown(mPrimaryConnection);
    }

    if (mPrimaryConnection.isValid())
    {
        setToolTip(statustext::activeToolTip(
            trTray("<pre>Connection <strong>%1</strong>(%2) active</pre>"),
            mPrimaryConnection.data(NmModel::NameRole).toString(),
            mPrimaryConnection.data(NmModel::ActiveConnectionTypeStringRole).toString()));
    }
    else
    {
        setToolTip(trTray("<pre>No active connection</pre>"));
    }
}

void NmTrayStatus::primaryConnectionUpdate()
{
    NetworkManager::ActiveConnection::Ptr prim_conn = NetworkManager::primaryConnection();
    if (!prim_conn || !prim_conn->isValid())
    {
        mPrimaryConnection = QModelIndex{};
        setShown(mPrimaryConnection);
        return;
    }

    const QModelIndexList l = mActiveConnections.match(mActiveConnections.index(0, 0, QModelIndex{}),
                                                       NmModel::ActiveConnectionUuidRole, prim_conn->uuid(), -1,
                                                       Qt::MatchExactly);
    // the connection may not be populated in the model yet; rowsInserted will catch it
    if (l.isEmpty())
        return;
    mPrimaryConnection = l.first();
    updateState(mPrimaryConnection, false);
}

void NmTrayStatus::setShown(const QPersistentModelIndex &index)
{
    mShownConnection = index;
    mIcon2Show = mShownConnection.isValid()
        ? static_cast<icons::Icon>(mActiveConnections.data(mShownConnection, NmModel::IconTypeRole).toInt())
        : icons::NETWORK_OFFLINE;
    mIconTimer.start();
}

void NmTrayStatus::updateIcon()
{
    if (mIconCurrent == mIcon2Show)
        return;
    mIconCurrent = mIcon2Show;
    Q_EMIT iconChanged(mIconCurrent);
}

void NmTrayStatus::setToolTip(const QString &toolTip)
{
    if (mToolTip == toolTip)
        return;
    mToolTip = toolTip;
    Q_EMIT toolTipChanged(mToolTip);
}

void NmTrayStatus::notify(const QModelIndex &index, bool removing)
{
    if (!mNotificationsEnabled)
        return;

    QString summary, body;
    if (removing)
    {
        mConnectionsToNotify.removeOne(index);
        summary = trTray("Connection lost");
        body = trTray("No longer connected to %1 '%2'.");
    }
    else
    {
        const int notif_i = mConnectionsToNotify.indexOf(index);
        if (-1 == notif_i
            || NetworkManager::ActiveConnection::Activated
                != static_cast<NetworkManager::ActiveConnection::State>(
                    mActiveConnections.data(index, NmModel::ActiveConnectionStateRole).toInt()))
        {
            return;
        }
        mConnectionsToNotify.removeAt(notif_i); // notify only once
        summary = trTray("Connection established");
        body = trTray("Now connected to %1 '%2'.");
    }

    const auto icon = static_cast<icons::Icon>(mActiveConnections.data(index, NmModel::IconTypeRole).toInt());
    auto reply = mNotification.Notify(
        trTray("NetworkManager(nm-tray)"), 0, icons::getIcon(icon, false).name(), summary,
        statustext::notificationBody(body,
                                     mActiveConnections.data(index, NmModel::ConnectionTypeStringRole).toString(),
                                     mActiveConnections.data(index, NmModel::NameRole).toString()),
        {}, {}, -1);

    auto *watcher = new QDBusPendingCallWatcher{reply, this};
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<uint> r = *w;
        if (r.isError())
            qWarning("lxqt-nm-tray: notification failed: %s", qPrintable(r.error().message()));
        w->deleteLater();
    });
}
