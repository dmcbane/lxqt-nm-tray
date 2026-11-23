// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>
#include <QPluginLoader>

#include <ilxqtpanelplugin.h>

class TestPluginLoad : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadsAsPanelPlugin()
    {
        QPluginLoader loader{QStringLiteral(PLUGIN_PATH)};
        QCOMPARE(loader.metaData().value(QStringLiteral("IID")).toString(),
                 QStringLiteral("lxqt.org/Panel/PluginInterface/3.0"));
        QObject *root = loader.instance();
        QVERIFY2(root, qPrintable(loader.errorString()));
        QVERIFY(qobject_cast<ILXQtPanelPluginLibrary *>(root) != nullptr);
    }
};

QTEST_MAIN(TestPluginLoad)
#include "tst_pluginload.moc"
