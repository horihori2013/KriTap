/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_BARS_H
#define KIS_MOBILE_BARS_H

#include <QWidget>

class KisMobileColorSwatch;
class KisMobileSliderRow;
class QLabel;
class QToolButton;

/**
 * Floating top bar of the mobile UI (ibisPaint-style):
 * menu, document title, undo/redo and a canvas-only toggle.
 *
 * The buttons are plain QToolButtons; the controller connects them to
 * the corresponding Krita actions (usually with setDefaultAction()).
 */
class KisMobileTopBar : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileTopBar(QWidget *parent = nullptr);

    void setDocumentTitle(const QString &title);

    QToolButton *menuButton() const;
    QToolButton *undoButton() const;
    QToolButton *redoButton() const;
    QToolButton *canvasOnlyButton() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *m_titleLabel {nullptr};
    QToolButton *m_menuButton {nullptr};
    QToolButton *m_undoButton {nullptr};
    QToolButton *m_redoButton {nullptr};
    QToolButton *m_canvasOnlyButton {nullptr};
};

/**
 * Floating bottom bar of the mobile UI: a tool strip on the left,
 * quick access buttons on the right and an expandable strip with
 * brush size / opacity / flow sliders.
 */
class KisMobileBottomBar : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileBottomBar(QWidget *parent = nullptr);

    void setSliderStripVisible(bool visible);
    bool sliderStripVisible() const;

    QToolButton *toolGridButton() const;
    QToolButton *brushButton() const;
    QToolButton *eraserButton() const;
    QToolButton *layersButton() const;
    QToolButton *slidersButton() const;
    KisMobileColorSwatch *colorSwatch() const;

    KisMobileSliderRow *brushSizeSlider() const;
    KisMobileSliderRow *brushOpacitySlider() const;
    KisMobileSliderRow *brushFlowSlider() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QWidget *m_sliderStrip {nullptr};
    KisMobileSliderRow *m_sizeSlider {nullptr};
    KisMobileSliderRow *m_opacitySlider {nullptr};
    KisMobileSliderRow *m_flowSlider {nullptr};
    QToolButton *m_toolGridButton {nullptr};
    QToolButton *m_brushButton {nullptr};
    QToolButton *m_eraserButton {nullptr};
    QToolButton *m_layersButton {nullptr};
    QToolButton *m_slidersButton {nullptr};
    KisMobileColorSwatch *m_colorSwatch {nullptr};
};

#endif /* KIS_MOBILE_BARS_H */
