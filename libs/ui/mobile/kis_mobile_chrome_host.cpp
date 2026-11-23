/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_chrome_host.h"

#include "kis_mobile_bars.h"
#include "kis_mobile_style.h"
#include "kis_mobile_widgets.h"

#include <QEvent>
#include <QToolButton>

#include <kis_icon.h>
#include <klocalizedstring.h>

#include "KisMainWindow.h"
#include "KisPlatformPluginInterfaceFactory.h"

namespace
{
    int sheetTopInset() { return 24; }
}

KisMobileChromeHost::KisMobileChromeHost(KisMainWindow *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_useSeparateWindows(KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS())
{
    m_topBar = new KisMobileTopBar(m_useSeparateWindows ? nullptr : m_window.data());
    m_bottomBar = new KisMobileBottomBar(m_useSeparateWindows ? nullptr : m_window.data());
    m_scrim = new KisMobileScrim(m_useSeparateWindows ? nullptr : m_window.data());
    m_sheet = new KisMobileSheet(QString(), m_useSeparateWindows ? nullptr : m_window.data());
    m_restoreButton = KisMobileStyle::createIconButton(m_useSeparateWindows ? nullptr : m_window.data(),
                                                       i18n("Show UI"),
                                                       KisMobileStyle::smallButtonSize,
                                                       KisMobileStyle::smallIconSize);

    m_restoreButton->setIcon(KisIconUtils::loadIcon("application-menu"));

    QList<QWidget*> chromeWidgets {m_topBar, m_bottomBar, m_scrim, m_sheet, m_restoreButton};
    for (QWidget *widget : chromeWidgets) {
        widget->setProperty("kisMobileChrome", true);
        if (m_useSeparateWindows) {
            widget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
            widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->setAttribute(Qt::WA_ShowWithoutActivating);
        }
        widget->setAttribute(Qt::WA_AcceptTouchEvents, true);
    }

    m_scrim->hide();
    m_sheet->hide();
    m_restoreButton->hide();

    if (m_window) {
        m_window->installEventFilter(this);
    }

    connect(m_scrim, &KisMobileScrim::clicked, this, &KisMobileChromeHost::hideSheet);
    connect(m_sheet, &KisMobileSheet::closeRequested, this, &KisMobileChromeHost::hideSheet);
    connect(m_restoreButton, &QAbstractButton::clicked, this, [this]() {
        setBarsVisible(true);
    });

    relayout();

    m_topBar->show();
    m_bottomBar->show();
}

KisMobileChromeHost::~KisMobileChromeHost()
{
    delete m_restoreButton;
    delete m_sheet;
    delete m_scrim;
    delete m_bottomBar;
    delete m_topBar;
}

KisMobileTopBar *KisMobileChromeHost::topBar() const      { return m_topBar; }
KisMobileBottomBar *KisMobileChromeHost::bottomBar() const { return m_bottomBar; }

void KisMobileChromeHost::showSheetContent(QWidget *content, const QString &title)
{
    if (!content) return;

    m_sheet->setTitle(title);
    m_sheet->setContentWidget(content);

    // everything inside a sheet must be scrollable with a finger
    KisMobileTouch::enableTouchScrolling(content);
    content->show();
    m_sheetOpen = true;

    m_scrim->show();
    m_sheet->show();

    relayout();

    m_scrim->raise();
    m_sheet->raise();
}

void KisMobileChromeHost::hideSheet()
{
    m_sheetOpen = false;
    m_sheet->hide();
    m_scrim->hide();
    Q_EMIT sheetClosed();
}

bool KisMobileChromeHost::sheetVisible() const
{
    return m_sheetOpen;
}

void KisMobileChromeHost::setBarsVisible(bool visible)
{
    if (m_barsVisible == visible) return;
    m_barsVisible = visible;
    updateChromeVisibility();
    relayout();
    Q_EMIT barsVisibilityChanged(visible);
}

bool KisMobileChromeHost::barsVisible() const
{
    return m_barsVisible;
}

QWidget *KisMobileChromeHost::createChromeWidget() const
{
    QWidget *widget = m_useSeparateWindows ? new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint)
                                           : new QWidget(m_window.data());
    if (m_useSeparateWindows) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
        widget->setAttribute(Qt::WA_ShowWithoutActivating);
    }
    widget->setAttribute(Qt::WA_AcceptTouchEvents, true);
    return widget;
}

void KisMobileChromeHost::placeWidget(QWidget *widget, const QRect &geometry) const
{
    if (!widget) return;

    if (m_useSeparateWindows) {
        const QPoint origin = m_window ? m_window->mapToGlobal(QPoint(0, 0)) : QPoint();
        widget->setGeometry(geometry.translated(origin));
    } else {
        widget->setGeometry(geometry);
    }
}

QRect KisMobileChromeHost::topBarGeometry() const
{
    const QRect area = m_window ? m_window->rect() : QRect(0, 0, 800, 600);
    // edge-anchored full-width strip (ibisPaint-style)
    return QRect(0, 0, area.width(), KisMobileStyle::barHeight);
}

QRect KisMobileChromeHost::bottomBarGeometry() const
{
    const QRect area = m_window ? m_window->rect() : QRect(0, 0, 800, 600);
    const int height = m_bottomBar->sizeHint().height();
    return QRect(0, area.height() - height, area.width(), height);
}

QRect KisMobileChromeHost::sheetGeometry() const
{
    const QRect area = m_window ? m_window->rect() : QRect(0, 0, 800, 600);

    // near-fullscreen bottom sheet that never leaves the window
    const int maxVisible = area.height() - sheetTopInset();
    int height = m_sheet->preferredHeight();
    if (height <= 0) {
        height = area.height() * 4 / 5;
    }
    height = qBound(qMin(300, maxVisible), height, maxVisible);

    // full-width bottom sheet anchored to the lower edge
    return QRect(0, area.height() - height, area.width(), height);
}

void KisMobileChromeHost::relayout()
{
    if (!m_window) return;

    placeWidget(m_topBar, topBarGeometry());
    placeWidget(m_bottomBar, bottomBarGeometry());
    placeWidget(m_sheet, sheetGeometry());
    placeWidget(m_scrim, m_window->rect());
    placeWidget(m_restoreButton,
                QRect(topBarGeometry().width() - KisMobileStyle::smallButtonSize - 8,
                      8,
                      KisMobileStyle::smallButtonSize,
                      KisMobileStyle::smallButtonSize));

    updateChromeVisibility();

    m_topBar->raise();
    m_bottomBar->raise();

    if (m_sheetOpen && m_window && m_window->isVisible() && !m_window->isMinimized()) {
        m_scrim->raise();
        m_sheet->raise();
    }
    if (m_restoreButton->isVisible()) {
        m_restoreButton->raise();
    }
}

void KisMobileChromeHost::updateChromeVisibility()
{
    const bool windowVisible = m_window && m_window->isVisible() && !m_window->isMinimized();

    if (m_barsVisible && windowVisible) {
        if (!m_topBar->isVisible()) m_topBar->show();
        if (!m_bottomBar->isVisible()) m_bottomBar->show();
    } else {
        m_topBar->hide();
        m_bottomBar->hide();
    }

    // always leave a way back from the immersive mode
    const bool showRestore = !m_barsVisible && windowVisible;
    if (showRestore != m_restoreButton->isVisible()) {
        m_restoreButton->setVisible(showRestore);
    }

    // the sheet and the scrim follow the main window
    if (!windowVisible) {
        m_scrim->hide();
        m_sheet->hide();
    } else if (m_sheetOpen) {
        m_scrim->show();
        m_sheet->show();
    }
}

bool KisMobileChromeHost::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_window) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
        case QEvent::WindowStateChange:
        case QEvent::ActivationChange:
        case QEvent::ScreenChangeInternal:
            relayout();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}