/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_bars.h"

#include "kis_mobile_style.h"
#include "kis_mobile_widgets.h"

#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QToolButton>
#include <QVBoxLayout>

#include <kis_icon.h>
#include <klocalizedstring.h>

namespace
{
    void paintEdgeStrip(QPaintEvent *event, QWidget *widget, bool topEdge)
    {
        QPainter painter(widget);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setClipRect(event->rect());

        // translucent strip fading towards the canvas
        QLinearGradient gradient(0, 0, 0, widget->height());
        if (topEdge) {
            gradient.setColorAt(0.0, QColor(22, 22, 27, 238));
            gradient.setColorAt(1.0, QColor(22, 22, 27, 165));
        } else {
            gradient.setColorAt(0.0, QColor(22, 22, 27, 165));
            gradient.setColorAt(1.0, QColor(22, 22, 27, 238));
        }
        painter.fillRect(widget->rect(), gradient);

        // hairline separator on the inner edge
        painter.fillRect(topEdge ? QRect(0, widget->height() - 1, widget->width(), 1)
                                 : QRect(0, 0, widget->width(), 1),
                         KisMobileStyle::separatorColor());
    }
}

/******************************************************************/
/* KisMobileTopBar                                                */
/******************************************************************/

KisMobileTopBar::KisMobileTopBar(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(6);

    m_menuButton = KisMobileStyle::createIconButton(this,
                                                    i18n("Menu"),
                                                    KisMobileStyle::smallButtonSize,
                                                    KisMobileStyle::smallIconSize);
    m_menuButton->setIcon(KisIconUtils::loadIcon("application-menu"));

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(13);
    m_titleLabel->setFont(titleFont);

    m_undoButton = KisMobileStyle::createIconButton(this,
                                                    i18n("Undo"),
                                                    KisMobileStyle::smallButtonSize,
                                                    KisMobileStyle::smallIconSize);
    m_undoButton->setIcon(KisIconUtils::loadIcon("edit-undo"));

    m_redoButton = KisMobileStyle::createIconButton(this,
                                                    i18n("Redo"),
                                                    KisMobileStyle::smallButtonSize,
                                                    KisMobileStyle::smallIconSize);
    m_redoButton->setIcon(KisIconUtils::loadIcon("edit-redo"));

    m_canvasOnlyButton = KisMobileStyle::createIconButton(this,
                                                          i18n("Canvas Only"),
                                                          KisMobileStyle::smallButtonSize,
                                                          KisMobileStyle::smallIconSize);
    m_canvasOnlyButton->setIcon(KisIconUtils::loadIcon("view-fullscreen"));
    m_canvasOnlyButton->setCheckable(true);

    layout->addWidget(m_menuButton);
    layout->addWidget(m_titleLabel, 1);
    layout->addWidget(m_undoButton);
    layout->addWidget(m_redoButton);
    layout->addWidget(m_canvasOnlyButton);

    setStyleSheet(KisMobileStyle::rootStyleSheet());
    setFixedHeight(KisMobileStyle::barHeight);
}

void KisMobileTopBar::setDocumentTitle(const QString &title)
{
    const int availableWidth = m_titleLabel->width();
    QFontMetrics metrics(m_titleLabel->font());
    m_titleLabel->setText(metrics.elidedText(title, Qt::ElideMiddle, qMax(40, availableWidth)));
}

QToolButton *KisMobileTopBar::menuButton() const      { return m_menuButton; }
QToolButton *KisMobileTopBar::undoButton() const      { return m_undoButton; }
QToolButton *KisMobileTopBar::redoButton() const      { return m_redoButton; }
QToolButton *KisMobileTopBar::canvasOnlyButton() const { return m_canvasOnlyButton; }

void KisMobileTopBar::paintEvent(QPaintEvent *event)
{
    paintEdgeStrip(event, this, true);
}

/******************************************************************/
/* KisMobileBottomBar                                             */
/******************************************************************/

KisMobileBottomBar::KisMobileBottomBar(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(4);

    // Expandable quick-settings strip: brush size / opacity / flow
    m_sliderStrip = new QWidget(this);
    QVBoxLayout *stripLayout = new QVBoxLayout(m_sliderStrip);
    stripLayout->setContentsMargins(4, 2, 4, 2);
    stripLayout->setSpacing(0);

    m_sizeSlider = new KisMobileSliderRow(i18n("Size"), m_sliderStrip);
    m_sizeSlider->setRange(1, 1000);
    m_opacitySlider = new KisMobileSliderRow(i18n("Opacity"), m_sliderStrip);
    m_opacitySlider->setRange(0, 100);
    m_flowSlider = new KisMobileSliderRow(i18n("Flow"), m_sliderStrip);
    m_flowSlider->setRange(0, 100);

    stripLayout->addWidget(m_sizeSlider);
    stripLayout->addWidget(m_opacitySlider);
    stripLayout->addWidget(m_flowSlider);
    m_sliderStrip->setVisible(false);

    // Buttons row
    QWidget *buttonsRow = new QWidget(this);
    QHBoxLayout *rowLayout = new QHBoxLayout(buttonsRow);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(6);

    m_toolGridButton = KisMobileStyle::createCaptionButton(buttonsRow, i18n("Tools"), i18n("Tools"));
    m_toolGridButton->setIcon(KisIconUtils::loadIcon("view-grid"));

    m_brushButton = KisMobileStyle::createCaptionButton(buttonsRow, i18n("Brushes"), i18n("Brushes"));
    m_brushButton->setIcon(KisIconUtils::loadIcon("krita_tool_brush"));

    m_eraserButton = KisMobileStyle::createCaptionButton(buttonsRow, i18n("Eraser"), i18n("Eraser"));
    m_eraserButton->setIcon(KisIconUtils::loadIcon("draw-eraser"));
    m_eraserButton->setCheckable(true);

    QWidget *spacer = new QWidget(buttonsRow);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_colorSwatch = new KisMobileColorSwatch(buttonsRow);
    m_colorSwatch->setToolTip(i18n("Color"));

    m_slidersButton = KisMobileStyle::createCaptionButton(buttonsRow, i18n("Size"), i18n("Brush size and opacity"));
    m_slidersButton->setIcon(KisIconUtils::loadIcon("brush-option-slider-1"));
    m_slidersButton->setCheckable(true);

    m_layersButton = KisMobileStyle::createCaptionButton(buttonsRow, i18n("Layers"), i18n("Layers"));
    m_layersButton->setIcon(KisIconUtils::loadIcon("paintLayer"));

    rowLayout->addWidget(m_toolGridButton);
    rowLayout->addWidget(m_brushButton);
    rowLayout->addWidget(m_eraserButton);
    rowLayout->addWidget(spacer, 1);
    rowLayout->addWidget(m_colorSwatch, 0, Qt::AlignVCenter);
    rowLayout->addWidget(m_slidersButton);
    rowLayout->addWidget(m_layersButton);

    layout->addWidget(m_sliderStrip);
    layout->addWidget(buttonsRow);

    setStyleSheet(KisMobileStyle::rootStyleSheet());
}

void KisMobileBottomBar::setSliderStripVisible(bool visible)
{
    m_sliderStrip->setVisible(visible);
    m_slidersButton->setChecked(visible);
    updateGeometry();
}

bool KisMobileBottomBar::sliderStripVisible() const
{
    return m_sliderStrip->isVisible();
}

QToolButton *KisMobileBottomBar::toolGridButton() const   { return m_toolGridButton; }
QToolButton *KisMobileBottomBar::brushButton() const      { return m_brushButton; }
QToolButton *KisMobileBottomBar::eraserButton() const     { return m_eraserButton; }
QToolButton *KisMobileBottomBar::layersButton() const     { return m_layersButton; }
QToolButton *KisMobileBottomBar::slidersButton() const    { return m_slidersButton; }
KisMobileColorSwatch *KisMobileBottomBar::colorSwatch() const { return m_colorSwatch; }

KisMobileSliderRow *KisMobileBottomBar::brushSizeSlider() const     { return m_sizeSlider; }
KisMobileSliderRow *KisMobileBottomBar::brushOpacitySlider() const  { return m_opacitySlider; }
KisMobileSliderRow *KisMobileBottomBar::brushFlowSlider() const     { return m_flowSlider; }

QSize KisMobileBottomBar::sizeHint() const
{
    int height = KisMobileStyle::barHeight + 28;
    if (m_sliderStrip->isVisible()) {
        height += m_sliderStrip->sizeHint().height();
    }
    return QSize(QWidget::sizeHint().width(), height);
}

void KisMobileBottomBar::paintEvent(QPaintEvent *event)
{
    paintEdgeStrip(event, this, false);
}
