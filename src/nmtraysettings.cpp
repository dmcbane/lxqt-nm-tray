// SPDX-License-Identifier: GPL-2.0-or-later
#include "nmtraysettings.h"

namespace
{
const QString ENABLE_NOTIFICATIONS = QStringLiteral("enableNotifications");
const QString CONNECTIONS_EDITOR = QStringLiteral("connectionsEditor");
}

NmTraySettings::NmTraySettings()
    : mSettings{QStringLiteral("nm-tray"), QStringLiteral("nm-tray")}
{
}

bool NmTraySettings::notificationsEnabled() const
{
    return mSettings.value(ENABLE_NOTIFICATIONS, true).toBool();
}

void NmTraySettings::setNotificationsEnabled(bool enabled)
{
    mSettings.setValue(ENABLE_NOTIFICATIONS, enabled);
}

QStringList NmTraySettings::connectionsEditor() const
{
    return mSettings.value(CONNECTIONS_EDITOR,
                           QStringList{QStringLiteral("xterm"), QStringLiteral("-e"), QStringLiteral("nmtui-edit")})
        .toStringList();
}
