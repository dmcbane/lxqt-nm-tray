/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Menu/actions adapted from nm-tray's src/tray.cpp, Copyright (c) 2015~now Palo Kisa.
 */
#include "nmtrayplugin.h"

#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QLocale>
#include <QMessageBox>
#include <QProcess>

#include <NetworkManagerQt/Manager>

#include "connectioninfo.h"
#include "icons.h"
#include "nmlist.h"
#include "nmtraysettings.h"
#include "nmtraystatus.h"
#include "popupanchor.h"
#include "windowmenu.h"

namespace
{
QString trTray(const char *text)
{
    return QCoreApplication::translate("Tray", text);
}

// A click on the button while its popup is open first closes the popup (Qt eats the press);
// without this guard the release would immediately reopen it.
constexpr qint64 REOPEN_GUARD_MS = 250;
}

NmTrayPlugin::NmTrayPlugin(const ILXQtPanelPluginStartupInfo &startupInfo)
    : QObject{}
    , ILXQtPanelPlugin{startupInfo}
{
    // Only nm-tray's own catalogue; the panel already installs the Qt one.
    if (mTranslator.load(QLatin1String("nm-tray_") + QLocale::system().name(),
                         QStringLiteral(NM_TRAY_TRANSLATION_DIR)))
        mTranslatorInstalled = QCoreApplication::installTranslator(&mTranslator);

    mStatus = new NmTrayStatus{&mModel, this};

    mButton.setAutoRaise(true);
    mButton.setContextMenuPolicy(Qt::CustomContextMenu);
    mButton.installEventFilter(this); // icon theme changes
    connect(&mButton, &QToolButton::clicked, this, &NmTrayPlugin::toggleConnectionMenu);
    connect(&mButton, &QWidget::customContextMenuRequested, this, &NmTrayPlugin::showContextMenu);
    connect(mStatus, &NmTrayStatus::iconChanged, this, &NmTrayPlugin::refreshIcon);
    connect(mStatus, &NmTrayStatus::toolTipChanged, &mButton, &QToolButton::setToolTip);
    mButton.setToolTip(mStatus->toolTip());
    refreshIcon();

    // context menu (same entries as nm-tray, minus Quit: removing the plugin is the panel's job)
    mActEnableNetwork = mContextMenu.addAction(trTray("Enable Networking"));
    mActEnableWifi = mContextMenu.addAction(trTray("Enable Wi-Fi"));
    mContextMenu.addSeparator();
    mActNotifications = mContextMenu.addAction(trTray("Enable notifications"));
    mContextMenu.addSeparator();
    mActConnInfo = mContextMenu.addAction(QIcon::fromTheme(QStringLiteral("dialog-information")),
                                          trTray("Connection information"));
    QAction *debugInfo = mContextMenu.addAction(QIcon::fromTheme(QStringLiteral("dialog-information")),
                                                trTray("Debug information"));
    mRequestScan = mContextMenu.addAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
                                          trTray("Wi-Fi - request scan"));
    QAction *edit = mContextMenu.addAction(QIcon::fromTheme(QStringLiteral("document-edit")),
                                           trTray("Edit connections..."));
    mContextMenu.addSeparator();
    QAction *about = mContextMenu.addAction(QIcon::fromTheme(QStringLiteral("help-about")), trTray("About"));

    mActEnableNetwork->setCheckable(true);
    mActEnableWifi->setCheckable(true);
    mActNotifications->setCheckable(true);
    mActNotifications->setChecked(mStatus->notificationsEnabled());

    connect(mActEnableNetwork, &QAction::triggered, this, [](bool on) { NetworkManager::setNetworkingEnabled(on); });
    connect(mActEnableWifi, &QAction::triggered, this, [](bool on) { NetworkManager::setWirelessEnabled(on); });
    connect(mActNotifications, &QAction::triggered, mStatus, &NmTrayStatus::setNotificationsEnabled);
    connect(mActConnInfo, &QAction::triggered, this,
            [this] { toggleDialog(mInfoDialog, [this] { return new ConnectionInfo{&mModel}; }); });
    connect(debugInfo, &QAction::triggered, this, [this] {
        toggleDialog(mDebugDialog, [this] { return new NmList{trTray("nm-tray info"), &mModel}; });
    });
    connect(mRequestScan, &QAction::triggered, &mModel, &NmModel::requestAllWifiScan);
    connect(edit, &QAction::triggered, this, &NmTrayPlugin::startConnectionsEditor);
    connect(about, &QAction::triggered, this, &NmTrayPlugin::showAbout);

    // coalesce bursts of NetworkManager property changes
    mStateTimer.setSingleShot(true);
    mStateTimer.setInterval(200);
    connect(&mStateTimer, &QTimer::timeout, this, &NmTrayPlugin::setActionsStates);
    const auto restart = [this] { mStateTimer.start(); };
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, restart);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, restart);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessHardwareEnabledChanged, this, restart);
    setActionsStates();
}

NmTrayPlugin::~NmTrayPlugin()
{
    // Everything referencing mModel must go before mModel itself is destroyed;
    // mStatus is a QObject child and would otherwise outlive it.
    delete mConnectionMenu.data();
    mInfoDialog.reset();
    mDebugDialog.reset();
    delete mStatus;
    if (mTranslatorInstalled)
        QCoreApplication::removeTranslator(&mTranslator);
}

void NmTrayPlugin::realign()
{
    const int size = panel()->iconSize();
    mButton.setIconSize(QSize{size, size});
}

void NmTrayPlugin::activated(ActivationReason reason)
{
    if (reason == Trigger)
        toggleConnectionMenu();
}

bool NmTrayPlugin::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == &mButton && event->type() == QEvent::ThemeChange)
        refreshIcon();
    return QObject::eventFilter(watched, event);
}

void NmTrayPlugin::toggleConnectionMenu()
{
    if (mConnectionMenu && mConnectionMenu->isVisible())
    {
        mConnectionMenu->close();
        return;
    }
    if (mSinceMenuHidden.isValid() && mSinceMenuHidden.elapsed() < REOPEN_GUARD_MS)
        return;

    // Fresh menu each time, as nm-tray does: WindowMenu builds its view from the live model.
    auto *menu = new WindowMenu{&mModel};
    menu->setAttribute(Qt::WA_DeleteOnClose);
    connect(menu, &QMenu::aboutToHide, this, [this] { mSinceMenuHidden.start(); });
    mConnectionMenu = menu;
    showPopup(menu);
}

void NmTrayPlugin::showContextMenu()
{
    if (mContextMenu.isVisible())
        return;
    connect(&mContextMenu, &QMenu::aboutToHide, this, [this] { mSinceMenuHidden.start(); },
            Qt::SingleShotConnection);
    showPopup(&mContextMenu);
}

void NmTrayPlugin::showPopup(QMenu *menu)
{
    menu->adjustSize();
    const QRect geometry = calculatePopupWindowPos(menu->sizeHint());
    willShowWindow(menu);
    popup::showAnchored(menu, &mButton, geometry);
}

void NmTrayPlugin::refreshIcon()
{
    // getIcon picks among theme candidates, so re-query on theme change too
    mButton.setIcon(icons::getIcon(mStatus->icon(), false));
}

void NmTrayPlugin::setActionsStates()
{
    const bool net_enabled = NetworkManager::isNetworkingEnabled();
    mActEnableNetwork->setChecked(net_enabled);
    mActEnableWifi->setChecked(NetworkManager::isWirelessEnabled());
    const bool wifi_possible = net_enabled && NetworkManager::isWirelessHardwareEnabled();
    mActEnableWifi->setEnabled(wifi_possible);
    mRequestScan->setEnabled(wifi_possible);
    mActConnInfo->setEnabled(net_enabled);
}

void NmTrayPlugin::toggleDialog(std::unique_ptr<QDialog> &holder, const std::function<QDialog *()> &create)
{
    if (!holder)
    {
        holder.reset(create());
        // don't delete the dialog from inside its own signal
        connect(holder.get(), &QDialog::finished, this, [&holder] { holder.release()->deleteLater(); });
    }
    QDialog *dialog = holder.get();
    if (dialog->isHidden() || dialog->isMinimized())
    {
        dialog->showNormal();
        dialog->activateWindow();
        dialog->raise();
    }
    else
    {
        dialog->close();
    }
}

void NmTrayPlugin::startConnectionsEditor()
{
    const QStringList editor = NmTraySettings{}.connectionsEditor();
    if (editor.isEmpty() || editor.front().isEmpty())
    {
        QMessageBox::warning(nullptr, QStringLiteral("nm-tray"),
                             tr("No connection editor configured. Set \"connectionsEditor\" in "
                                "~/.config/nm-tray/nm-tray.conf."));
        return;
    }
    if (!QProcess::startDetached(editor.front(), editor.mid(1)))
    {
        QMessageBox::warning(nullptr, QStringLiteral("nm-tray"),
                             tr("Could not start connection editor: %1").arg(editor.join(QLatin1Char(' '))));
    }
}

void NmTrayPlugin::showAbout()
{
    QMessageBox::about(nullptr, trTray("%1 about").arg(QStringLiteral("lxqt-nm-tray")),
                       tr("<strong>lxqt-nm-tray</strong> %1 hosts "
                          "<a href=\"https://github.com/palinek/nm-tray\">nm-tray</a> %2 "
                          "as a native LXQt panel plugin.")
                           .arg(QStringLiteral(LXQT_NM_TRAY_VERSION), QStringLiteral(NM_TRAY_VERSION)));
}
