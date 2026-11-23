// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>
#include <QSettings>
#include <QStandardPaths>

#include "nmtraysettings.h"

class TestSettings : public QObject
{
    Q_OBJECT

private:
    static void clearAll()
    {
        QSettings{QStringLiteral("nm-tray"), QStringLiteral("nm-tray")}.clear();
        QSettings{}.clear();
    }

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        // Simulate running inside lxqt-panel: the host process owns the default QSettings.
        QCoreApplication::setOrganizationName(QStringLiteral("lxqt"));
        QCoreApplication::setApplicationName(QStringLiteral("lxqt-panel-settings-test"));
    }

    void init() { clearAll(); }
    void cleanupTestCase() { clearAll(); }

    void notificationsDefaultToEnabled()
    {
        NmTraySettings s;
        QCOMPARE(s.notificationsEnabled(), true);
    }

    void writesGoToNmTrayConfigNotHostConfig()
    {
        {
            NmTraySettings s;
            s.setNotificationsEnabled(false);
        }

        QSettings nmTray{QStringLiteral("nm-tray"), QStringLiteral("nm-tray")};
        QCOMPARE(nmTray.value(QStringLiteral("enableNotifications")).toBool(), false);

        QSettings host;
        QVERIFY2(host.allKeys().isEmpty(), qPrintable(host.allKeys().join(QLatin1Char(','))));

        NmTraySettings reread;
        QCOMPARE(reread.notificationsEnabled(), false);
    }

    void connectionsEditorDefaultsToNmtui()
    {
        NmTraySettings s;
        QCOMPARE(s.connectionsEditor(),
                 (QStringList{QStringLiteral("xterm"), QStringLiteral("-e"), QStringLiteral("nmtui-edit")}));
    }

    void connectionsEditorReadsExistingNmTrayConfig()
    {
        const QStringList editor{QStringLiteral("nm-connection-editor")};
        {
            QSettings nmTray{QStringLiteral("nm-tray"), QStringLiteral("nm-tray")};
            nmTray.setValue(QStringLiteral("connectionsEditor"), editor);
        }
        NmTraySettings s;
        QCOMPARE(s.connectionsEditor(), editor);
    }
};

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
