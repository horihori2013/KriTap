/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_CHROME_HOST_H
#define KIS_MOBILE_CHROME_HOST_H

#include <QPointer>
#include <QObject>

class KisMainWindow;
class KisMobileBottomBar;
class KisMobileScrim;
class KisMobileSheet;
class KisMobileTopBar;
class QToolButton;
class QWidget;

/**
 * Hosts and positions the floating chrome widgets of the mobile UI:
 * the top bar, the bottom bar and the slide-up bottom sheet.
 *
 * Normally the chrome widgets are children of the main window, translucent
 * and floating above the canvas. On platforms where the canvas is rendered
 * on a separate OS-managed surface (and therefore paints over all sibling
 * widgets) the chrome widgets become small frameless toplevel windows
 * pinned to the main window instead — the same approach the popup palette
 * uses.
 */
class KisMobileChromeHost : public QObject
{
    Q_OBJECT
public:
    explicit KisMobileChromeHost(KisMainWindow *window, QObject *parent = nullptr);
    ~KisMobileChromeHost() override;

    KisMobileTopBar *topBar() const;
    KisMobileBottomBar *bottomBar() const;

    /// Show the sheet (a translucent scrim covers the canvas while it is open)
    void showSheetContent(QWidget *content, const QString &title);
    void hideSheet();
    bool sheetVisible() const;

    /// hide all chrome (immersive drawing mode), the restore button stays
    void setBarsVisible(bool visible);
    bool barsVisible() const;

    void relayout();

Q_SIGNALS:
    void barsVisibilityChanged(bool visible);
    void sheetClosed();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *createChromeWidget() const;
    void placeWidget(QWidget *widget, const QRect &geometry) const;
    QRect topBarGeometry() const;
    QRect bottomBarGeometry() const;
    QRect sheetGeometry() const;
    void updateChromeVisibility();

private:
    QPointer<KisMainWindow> m_window;
    QWidget *m_container {nullptr};
    bool m_useSeparateWindows {false};

    KisMobileTopBar *m_topBar {nullptr};
    KisMobileBottomBar *m_bottomBar {nullptr};
    KisMobileScrim *m_scrim {nullptr};
    KisMobileSheet *m_sheet {nullptr};
    QToolButton *m_restoreButton {nullptr};

    bool m_barsVisible {true};
    bool m_sheetOpen {false};
};

#endif /* KIS_MOBILE_CHROME_HOST_H */