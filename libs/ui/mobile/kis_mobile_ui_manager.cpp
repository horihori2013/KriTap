/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_ui_manager.h"

#include "kis_mobile_bars.h"
#include "kis_mobile_brush_sheet.h"
#include "kis_mobile_chrome_host.h"
#include "kis_mobile_color_sheet.h"
#include "kis_mobile_layers_sheet.h"
#include "kis_mobile_sheets.h"
#include "kis_mobile_widgets.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDockWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include <KoColor.h>
#include <kis_action.h>
#include <kis_paintop_preset.h>
#include <klocalizedstring.h>

#include "KisMainWindow.h"
#include "KisPart.h"
#include "KisViewManager.h"
#include <kactioncollection.h>
#include "kis_action_manager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"

KisMobileUiManager::KisMobileUiManager(QObject *parent)
    : QObject(parent)
{
}

KisMobileUiManager *KisMobileUiManager::instance()
{
    static KisMobileUiManager *s_instance = new KisMobileUiManager(QCoreApplication::instance());
    return s_instance;
}

bool KisMobileUiManager::shouldUseMobileUi()
{
    if (qEnvironmentVariableIsSet("KRITA_MOBILE_UI")) {
        return qEnvironmentVariableIntValue("KRITA_MOBILE_UI") != 0;
    }

    KisConfig cfg(true);
    return cfg.mobileUiEnabled();
}

void KisMobileUiManager::install(KisMainWindow *window)
{
    if (m_installed) return;

    if (!window) {
        window = m_window;
    }
    if (!window) return;

    m_window = window;
    m_installed = true;

    hideDesktopChrome();

    m_chromeHost = new KisMobileChromeHost(window, this);
    createSheets();
    wireChrome();

    KisViewManager *viewManager = window->viewManager();
    if (viewManager) {
        connect(viewManager, &KisViewManager::viewChanged,
                this, &KisMobileUiManager::slotViewChanged);

        if (KisAction *fullScreenAction = viewManager->actionManager()->actionByName("fullscreen")) {
            connect(fullScreenAction, &QAction::toggled,
                    this, &KisMobileUiManager::slotRehideDesktopChrome);
        }

        if (KisPart *part = KisPart::instance()) {
            connect(part, &KisPart::documentClosed,
                    this, [this](const QString &) { slotViewChanged(); });
        }
    }

    connect(window, &QWidget::windowTitleChanged,
            this, &KisMobileUiManager::slotUpdateTitle);

    slotViewChanged();
}

void KisMobileUiManager::uninstall()
{
    if (!m_installed) return;

    if (m_chromeHost) {
        m_chromeHost->hideSheet();
        delete m_chromeHost;
        m_chromeHost = nullptr;
    }

    destroySheets();

    if (m_provider) {
        disconnect(m_provider, nullptr, this, nullptr);
        m_provider = nullptr;
    }

    showDesktopChrome();

    m_installed = false;
}

bool KisMobileUiManager::isInstalled() const
{
    return m_installed;
}

KisMobileChromeHost *KisMobileUiManager::chromeHost() const
{
    return m_chromeHost;
}

void KisMobileUiManager::setMobileUiEnabled(bool enabled)
{
    KisConfig cfg(false);
    cfg.setMobileUiEnabled(enabled);

    if (enabled && !m_installed) {
        install(m_window);
    } else if (!enabled && m_installed) {
        uninstall();
    }
}

/******************************************************************/
/* desktop chrome handling                                        */
/******************************************************************/

void KisMobileUiManager::hideDesktopChrome()
{
    if (!m_window) return;

    rememberAndHide(m_window->menuBar());

    const QList<QToolBar *> toolBars = m_window->findChildren<QToolBar *>();
    for (QWidget *toolBar : toolBars) {
        rememberAndHide(toolBar);
    }

    const auto dockWidgets = m_window->dockWidgets();
    for (QWidget *dockWidget : dockWidgets) {
        rememberAndHide(dockWidget);
    }

    rememberAndHide(m_window->statusBar());
}

void KisMobileUiManager::showDesktopChrome()
{
    const QList<QWidget *> widgets = m_hiddenChromeWidgets;
    m_hiddenChromeWidgets.clear();

    for (QWidget *widget : widgets) {
        if (widget) {
            widget->show();
        }
    }
}

void KisMobileUiManager::rememberAndHide(QWidget *widget)
{
    if (!widget) return;

    if (widget->isVisible() && !m_hiddenChromeWidgets.contains(widget)) {
        m_hiddenChromeWidgets.append(widget);
    }
    widget->hide();
}

void KisMobileUiManager::slotRehideDesktopChrome()
{
    // action-driven chrome changes (e.g. canvas-only mode) may re-show
    // the desktop widgets, so hide them again on the next event
    QTimer::singleShot(0, this, [this]() {
        hideDesktopChrome();
        if (m_chromeHost) {
            m_chromeHost->relayout();
        }
    });
}

/******************************************************************/
/* sheets                                                         */
/******************************************************************/

void KisMobileUiManager::createSheets()
{
    KisViewManager *viewManager = m_window ? m_window->viewManager() : nullptr;

    m_toolsSheet = new KisMobileToolsSheet(viewManager);
    m_brushSheet = new KisMobileBrushSheet();
    m_colorSheet = new KisMobileColorSheet();
    m_layersSheet = new KisMobileLayersSheet();
    m_moreSheet = new KisMobileMoreSheet(viewManager);

    if (m_layersSheet) {
        m_layersSheet->setViewManager(viewManager);
    }

    connect(m_brushSheet, &KisMobileBrushSheet::finished, this, [this]() {
        if (m_chromeHost) m_chromeHost->hideSheet();
    });

    connect(m_moreSheet, &KisMobileMoreSheet::hideUiRequested, this, [this]() {
        if (m_chromeHost) {
            m_chromeHost->hideSheet();
            m_chromeHost->setBarsVisible(false);
        }
    });

    connect(m_moreSheet, &KisMobileMoreSheet::disableMobileUiRequested, this, [this]() {
        if (m_chromeHost) m_chromeHost->hideSheet();
        setMobileUiEnabled(false);
    });
}

void KisMobileUiManager::destroySheets()
{
    delete m_toolsSheet.data();
    delete m_brushSheet.data();
    delete m_colorSheet.data();
    delete m_layersSheet.data();
    delete m_moreSheet.data();

    m_toolsSheet = nullptr;
    m_brushSheet = nullptr;
    m_colorSheet = nullptr;
    m_layersSheet = nullptr;
    m_moreSheet = nullptr;
}

void KisMobileUiManager::openSheet(QWidget *content, const QString &title)
{
    if (!m_chromeHost || !content) return;
    m_chromeHost->showSheetContent(content, title);
}

void KisMobileUiManager::slotOpenToolsSheet()
{
    openSheet(m_toolsSheet, i18n("Tools"));
}

void KisMobileUiManager::slotOpenBrushSheet()
{
    if (m_brushSheet) {
        m_brushSheet->aboutToShow();
    }
    openSheet(m_brushSheet, i18n("Brushes"));
}

void KisMobileUiManager::slotOpenColorSheet()
{
    if (m_colorSheet) {
        m_colorSheet->aboutToShow();
    }
    openSheet(m_colorSheet, i18n("Color"));
}

void KisMobileUiManager::slotOpenLayersSheet()
{
    if (m_layersSheet) {
        m_layersSheet->aboutToShow();
    }
    openSheet(m_layersSheet, i18n("Layers"));
}

void KisMobileUiManager::slotOpenMoreSheet()
{
    openSheet(m_moreSheet, i18n("Menu"));
}

/******************************************************************/
/* chrome wiring                                                  */
/******************************************************************/

void KisMobileUiManager::wireChrome()
{
    if (!m_chromeHost) return;

    KisViewManager *viewManager = m_window ? m_window->viewManager() : nullptr;
    KisKActionCollection *actionCollection = viewManager ? viewManager->actionCollection() : nullptr;
    KisActionManager *actionManager = viewManager ? viewManager->actionManager() : nullptr;

    KisMobileTopBar *topBar = m_chromeHost->topBar();
    KisMobileBottomBar *bottomBar = m_chromeHost->bottomBar();

    auto findAction = [actionCollection, actionManager](const QString &name) -> QAction * {
        if (actionCollection) {
            if (QAction *action = actionCollection->action(name)) {
                return action;
            }
        }
        return actionManager ? actionManager->actionByName(name) : nullptr;
    };

    if (QAction *undo = findAction("edit_undo")) {
        topBar->undoButton()->setDefaultAction(undo);
    }
    if (QAction *redo = findAction("edit_redo")) {
        topBar->redoButton()->setDefaultAction(redo);
    }
    if (QAction *canvasOnly = findAction("view_show_canvas_only")) {
        topBar->canvasOnlyButton()->setDefaultAction(canvasOnly);
    }
    if (QAction *eraser = findAction("eraser_preset_action")) {
        bottomBar->eraserButton()->setDefaultAction(eraser);
    }

    connect(topBar->menuButton(), &QAbstractButton::clicked,
            this, &KisMobileUiManager::slotOpenMoreSheet);
    connect(bottomBar->toolGridButton(), &QAbstractButton::clicked,
            this, &KisMobileUiManager::slotOpenToolsSheet);
    connect(bottomBar->brushButton(), &QAbstractButton::clicked,
            this, &KisMobileUiManager::slotOpenBrushSheet);
    connect(bottomBar->layersButton(), &QAbstractButton::clicked,
            this, &KisMobileUiManager::slotOpenLayersSheet);
    connect(bottomBar->colorSwatch(), &QAbstractButton::clicked,
            this, &KisMobileUiManager::slotOpenColorSheet);
    connect(bottomBar->slidersButton(), &QAbstractButton::clicked,
            this, &KisMobileUiManager::slotToggleSliderStrip);

    connect(bottomBar->brushSizeSlider(), &KisMobileSliderRow::valueChanged,
            this, &KisMobileUiManager::slotBrushSizeChanged);
    connect(bottomBar->brushOpacitySlider(), &KisMobileSliderRow::valueChanged,
            this, &KisMobileUiManager::slotBrushOpacityChanged);
    connect(bottomBar->brushFlowSlider(), &KisMobileSliderRow::valueChanged,
            this, &KisMobileUiManager::slotBrushFlowChanged);
}

/******************************************************************/
/* state tracking                                                 */
/******************************************************************/

void KisMobileUiManager::slotViewChanged()
{
    KisViewManager *viewManager = m_window ? m_window->viewManager() : nullptr;
    KisCanvasResourceProvider *provider = viewManager ? viewManager->canvasResourceProvider() : nullptr;

    if (provider != m_provider) {
        if (m_provider) {
            disconnect(m_provider, nullptr, this, nullptr);
        }
        m_provider = provider;

        if (m_provider) {
            connect(m_provider, &KisCanvasResourceProvider::sigPaintOpPresetChanged,
                    this, [this](const KisPaintOpPresetSP) {
                        slotUpdateBrushSliders();
                    });
            connect(m_provider, &KisCanvasResourceProvider::sigFGColorChanged,
                    this, [this](const KoColor &color) {
                        if (m_chromeHost) {
                            m_chromeHost->bottomBar()->colorSwatch()->setColor(color.toQColor());
                        }
                    });
        }
    }

    if (m_colorSheet) {
        m_colorSheet->setResourceProvider(provider);
    }

    if (m_brushSheet) {
        m_brushSheet->setResourceProvider(provider);
        m_brushSheet->setPaintOpBox(viewManager ? viewManager->paintOpBox() : nullptr);
    }

    if (m_layersSheet) {
        m_layersSheet->setCanvas(viewManager ? viewManager->canvasBase() : nullptr);
    }

    slotUpdateBrushSliders();
    slotUpdateTitle();
}

void KisMobileUiManager::slotUpdateBrushSliders()
{
    if (!m_chromeHost || !m_provider) return;

    KisMobileBottomBar *bottomBar = m_chromeHost->bottomBar();

    const qreal size = qMax<qreal>(1.0, m_provider->size());
    bottomBar->brushSizeSlider()->setRange(1.0, qMax<qreal>(1000.0, size * 1.5));
    bottomBar->brushSizeSlider()->setValue(size);
    bottomBar->brushOpacitySlider()->setValue(m_provider->opacity() * 100.0);
    bottomBar->brushFlowSlider()->setValue(m_provider->flow() * 100.0);

    bottomBar->colorSwatch()->setColor(m_provider->fgColor().toQColor());
}

void KisMobileUiManager::slotBrushSizeChanged(qreal value)
{
    if (m_provider) {
        m_provider->setSize(value);
    }
}

void KisMobileUiManager::slotBrushOpacityChanged(qreal value)
{
    if (m_provider) {
        m_provider->setOpacity(value / 100.0);
    }
}

void KisMobileUiManager::slotBrushFlowChanged(qreal value)
{
    if (m_provider) {
        m_provider->setFlow(value / 100.0);
    }
}

void KisMobileUiManager::slotToggleSliderStrip()
{
    if (!m_chromeHost) return;

    KisMobileBottomBar *bottomBar = m_chromeHost->bottomBar();
    const bool visible = !bottomBar->sliderStripVisible();
    bottomBar->setSliderStripVisible(visible);

    if (visible) {
        slotUpdateBrushSliders();
    }

    m_chromeHost->relayout();
}

void KisMobileUiManager::slotUpdateTitle()
{
    if (m_chromeHost && m_window) {
        m_chromeHost->topBar()->setDocumentTitle(m_window->windowTitle());
    }
}