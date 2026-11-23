// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>
#include <QMenu>
#include <QWidget>
#include <QWindow>

#include "popupanchor.h"

class TestPopupAnchor : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void setsTransientParentToAnchorWindow()
    {
        QWidget panel;
        panel.resize(400, 30);
        panel.show();
        QVERIFY(QTest::qWaitForWindowExposed(&panel));

        QWidget *button = new QWidget{&panel};
        button->setGeometry(10, 0, 30, 30);
        button->show();

        QMenu menu;
        menu.addAction(QStringLiteral("entry"));

        popup::showAnchored(&menu, button, QRect{QPoint{50, 60}, menu.sizeHint()});

        QVERIFY(menu.isVisible());
        QVERIFY(menu.windowHandle() != nullptr);
        QCOMPARE(menu.windowHandle()->transientParent(), panel.windowHandle());
        menu.close();
    }

    void showsAtRequestedPosition()
    {
        QWidget panel;
        panel.resize(400, 30);
        panel.show();
        QVERIFY(QTest::qWaitForWindowExposed(&panel));

        QMenu menu;
        menu.addAction(QStringLiteral("entry"));

        const QPoint wanted{120, 80};
        popup::showAnchored(&menu, &panel, QRect{wanted, menu.sizeHint()});

        QVERIFY(menu.isVisible());
        QCOMPARE(menu.pos(), wanted);
        menu.close();
    }

    void createsNativeHandlesForUnshownAnchor()
    {
        // The anchor's top-level must get a native window so the popup has a parent
        // surface; without it Wayland refuses (or misplaces) the popup.
        QWidget panel;
        QMenu menu;
        menu.addAction(QStringLiteral("entry"));

        popup::showAnchored(&menu, &panel, QRect{QPoint{5, 5}, menu.sizeHint()});

        QVERIFY(panel.windowHandle() != nullptr);
        QCOMPARE(menu.windowHandle()->transientParent(), panel.windowHandle());
        menu.close();
    }
};

QTEST_MAIN(TestPopupAnchor)
#include "tst_popupanchor.moc"
