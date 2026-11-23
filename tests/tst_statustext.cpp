// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>

#include "statustext.h"

class TestStatusText : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void toolTipShowsNameAndType()
    {
        const QString t = statustext::activeToolTip(QStringLiteral("<pre>%1 (%2)</pre>"),
                                                    QStringLiteral("HomeNet"), QStringLiteral("Wi-Fi"));
        QCOMPARE(t, QStringLiteral("<pre>HomeNet (Wi-Fi)</pre>"));
    }

    void toolTipEscapesMarkupInNames()
    {
        // SSIDs are chosen by whoever runs a nearby access point.
        const QString t = statustext::activeToolTip(QStringLiteral("<pre>%1 (%2)</pre>"),
                                                    QStringLiteral("<img src=x>&"), QStringLiteral("<b>"));
        QCOMPARE(t, QStringLiteral("<pre>&lt;img src=x&gt;&amp; (&lt;b&gt;)</pre>"));
    }

    void placeholdersInNamesAreNotExpanded()
    {
        const QString t = statustext::activeToolTip(QStringLiteral("%1/%2"), QStringLiteral("net%2"),
                                                    QStringLiteral("wifi"));
        QCOMPARE(t, QStringLiteral("net%2/wifi"));
    }

    void notificationBodyEscapesMarkup()
    {
        const QString b = statustext::notificationBody(QStringLiteral("Now connected to %1 '%2'."),
                                                       QStringLiteral("Wi-Fi"), QStringLiteral("<a href=x>evil</a>"));
        QCOMPARE(b, QStringLiteral("Now connected to Wi-Fi '&lt;a href=x&gt;evil&lt;/a&gt;'."));
    }
};

QTEST_GUILESS_MAIN(TestStatusText)
#include "tst_statustext.moc"
