// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <QRect>

class QMenu;
class QWidget;

namespace popup
{
/*!
 * Shows \a menu as a popup owned by \a anchor's top-level window, at \a geometry's top-left.
 *
 * On Wayland a client cannot place a window at absolute coordinates; a menu is only placed
 * correctly when the compositor knows its parent surface (xdg_popup). nm-tray as a tray app
 * has no such parent and uses QCursor::pos(), which Wayland doesn't report, so its menu ends
 * up in the wrong place or not at all. Setting the panel window as transient parent fixes that.
 */
void showAnchored(QMenu *menu, QWidget *anchor, const QRect &geometry);
}
