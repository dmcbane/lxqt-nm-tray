// SPDX-License-Identifier: GPL-2.0-or-later
#include "popupanchor.h"

#include <QMenu>
#include <QWidget>
#include <QWindow>

void popup::showAnchored(QMenu *menu, QWidget *anchor, const QRect &geometry)
{
    Q_ASSERT(menu);
    Q_ASSERT(anchor);

    QWidget *host = anchor->window();
    host->winId(); // ensure the host has a native window to parent the popup to
    menu->winId();
    menu->windowHandle()->setTransientParent(host->windowHandle());

    menu->popup(geometry.topLeft());
}
