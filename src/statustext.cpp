// SPDX-License-Identifier: GPL-2.0-or-later
#include "statustext.h"

QString statustext::activeToolTip(const QString &templ, const QString &name, const QString &type)
{
    // multi-arg overload substitutes in one pass, so a "%2" inside a name stays literal
    return templ.arg(name.toHtmlEscaped(), type.toHtmlEscaped());
}

QString statustext::notificationBody(const QString &templ, const QString &type, const QString &name)
{
    return templ.arg(type.toHtmlEscaped(), name.toHtmlEscaped());
}
