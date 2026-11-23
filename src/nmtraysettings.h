// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <QSettings>
#include <QStringList>

/*!
 * nm-tray's user settings (~/.config/nm-tray/nm-tray.conf).
 *
 * nm-tray uses a default-constructed QSettings, which inside lxqt-panel would resolve to the
 * panel's own config. This class always names nm-tray's file explicitly, so the plugin shares
 * settings with standalone nm-tray and never writes into the host's configuration.
 */
class NmTraySettings
{
public:
    NmTraySettings();

    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool enabled);

    QStringList connectionsEditor() const;

private:
    QSettings mSettings;
};
