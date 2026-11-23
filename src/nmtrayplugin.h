/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * lxqt-panel plugin hosting nm-tray's NetworkManager menus natively in the panel.
 * Menu/actions adapted from nm-tray's src/tray.cpp, Copyright (c) 2015~now Palo Kisa.
 */
#pragma once

#include <QElapsedTimer>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <functional>
#include <memory>
#include <QTimer>
#include <QToolButton>
#include <QTranslator>

#include <ilxqtpanelplugin.h>

#include "nmmodel.h"

class NmTrayStatus;
class QDialog;

class NmTrayPlugin : public QObject, public ILXQtPanelPlugin
{
    Q_OBJECT

public:
    explicit NmTrayPlugin(const ILXQtPanelPluginStartupInfo &startupInfo);
    ~NmTrayPlugin() override;

    QString themeId() const override { return QStringLiteral("NmTray"); }
    Flags flags() const override { return PreferRightAlignment | SingleInstance | NeedsHandle; }
    QWidget *widget() override { return &mButton; }
    void realign() override;
    void activated(ActivationReason reason) override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void toggleConnectionMenu();
    void showContextMenu();
    void showPopup(QMenu *menu);
    void refreshIcon();
    void setActionsStates();
    void toggleDialog(std::unique_ptr<QDialog> &holder, const std::function<QDialog *()> &create);
    void startConnectionsEditor();
    void showAbout();

    QTranslator mTranslator;
    bool mTranslatorInstalled = false;
    NmModel mModel;
    NmTrayStatus *mStatus;
    QToolButton mButton;
    QMenu mContextMenu;
    QPointer<QMenu> mConnectionMenu;
    QElapsedTimer mSinceMenuHidden;
    QTimer mStateTimer;
    QAction *mActEnableNetwork;
    QAction *mActEnableWifi;
    QAction *mActNotifications;
    QAction *mActConnInfo;
    QAction *mRequestScan;
    std::unique_ptr<QDialog> mInfoDialog;
    std::unique_ptr<QDialog> mDebugDialog;
};

class NmTrayPluginLibrary : public QObject, public ILXQtPanelPluginLibrary
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "lxqt.org/Panel/PluginInterface/3.0")
    Q_INTERFACES(ILXQtPanelPluginLibrary)

public:
    ILXQtPanelPlugin *instance(const ILXQtPanelPluginStartupInfo &startupInfo) const override
    {
        return new NmTrayPlugin(startupInfo);
    }
};
