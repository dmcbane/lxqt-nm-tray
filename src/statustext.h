// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <QString>

/*!
 * Builds the user-visible status strings. Connection names (notably Wi-Fi SSIDs) are
 * untrusted and are HTML-escaped, because tooltips render rich text and notification
 * servers may interpret markup in the body.
 */
namespace statustext
{
QString activeToolTip(const QString &templ, const QString &name, const QString &type);
QString notificationBody(const QString &templ, const QString &type, const QString &name);
}
