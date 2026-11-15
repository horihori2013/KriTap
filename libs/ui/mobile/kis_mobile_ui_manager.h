/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_UI_MANAGER_H
#define KIS_MOBILE_UI_MANAGER_H

#include <QList>
#include <QPointer>
#include <QObject>

class KisCanvasResourceProvider;
class KisMainWindow;
class KisMobileBrushSheet;
class KisMobileChromeHost;
class KisMobileColorSheet;
class KisMobileLayersSheet;
class KisMobileMoreSheet;
class KisMobileToolsSheet;
class QWidget;

/**
 * Controller of the smartphone-friendly (ibisPaint-style) UI.
 *
 * When installed it hides the desktop chrome (menu bar, toolbars, dockers)
 * and puts the floating mobile chrome in place: a top bar, a bottom bar and
 * slide-up sheets for tools, brushes, color, layers and file actions.
 *
 * The mobile UI is enabled by default on Android and can be toggled on other
 * platforms with the "mobileUiEnabled" config entry or the KRITA_MOBILE_UI
 * environment variable (0/1).
 */
class KisMobileUiManager : public QObject
{
    Q_OBJECT
public:
    static KisMobileUiManager *instance();

    /// Returns true when the mobile UI should be used on startup
    static bool shouldUseMobileUi();

    void install(KisMainWindow *window);
    void uninstall();
    bool isInstalled() const;

    KisMobileChromeHost *chromeHost() const;

public Q_SLOTS:
    /// runtime toggle; writes the config entry and installs/uninstalls
    void setMobileUiEnabled(bool enabled);

private Q_SLOTS:
    void slotOpenToolsSheet();
    void slotOpenBrushSheet();
    void slotOpenColorSheet();
    void slotOpenLayersSheet();
    void slotOpenMoreSheet();
    void slotViewChanged();
    void slotUpdateBrushSliders();
    void slotBrushSizeChanged(qreal value);
    void slotBrushOpacityChanged(qreal value);
    void slotBrushFlowChanged(qreal value);
    void slotToggleSliderStrip();
    void slotUpdateTitle();
    void slotRehideDesktopChrome();

private:
    explicit KisMobileUiManager(QObject *parent = nullptr);

    void hideDesktopChrome();
    void showDesktopChrome();
    void rememberAndHide(QWidget *widget);
    void openSheet(QWidget *content, const QString &title);
    void createSheets();
    void destroySheets();
    void wireChrome();

private:
    QPointer<KisMainWindow> m_window;
    KisMobileChromeHost *m_chromeHost {nullptr};

    QPointer<KisMobileToolsSheet> m_toolsSheet;
    QPointer<KisMobileBrushSheet> m_brushSheet;
    QPointer<KisMobileColorSheet> m_colorSheet;
    QPointer<KisMobileLayersSheet> m_layersSheet;
    QPointer<KisMobileMoreSheet> m_moreSheet;

    QPointer<KisCanvasResourceProvider> m_provider;

    QList<QWidget *> m_hiddenChromeWidgets;
    bool m_installed {false};
};

#endif /* KIS_MOBILE_UI_MANAGER_H */