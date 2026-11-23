/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_widgets.h"

#include "kis_mobile_style.h"

#include <QHBoxLayout>
#include <klocalizedstring.h>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QScroller>
#include <QSignalBlocker>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    const int dragDismissThreshold = 60;
}

/******************************************************************/
/* KisMobileSheet                                                 */
/******************************************************************/

KisMobileSheet::KisMobileSheet(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_header = new QWidget(this);
    m_header->setFixedHeight(KisMobileStyle::dragHandleHeight + 34);
    m_header->setAttribute(Qt::WA_AcceptTouchEvents, true);

    QHBoxLayout *headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(16, 4, 8, 4);

    m_titleLabel = new QLabel(title, m_header);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(15);
    m_titleLabel->setFont(titleFont);

    m_closeButton = KisMobileStyle::createIconButton(m_header,
                                                     i18n("Close"),
                                                     KisMobileStyle::smallButtonSize,
                                                     KisMobileStyle::smallIconSize);

    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_closeButton, 0, Qt::AlignVCenter);

    m_contentHost = new QWidget(this);
    m_contentLayout = new QVBoxLayout(m_contentHost);
    m_contentLayout->setContentsMargins(12, 4, 12, 12);
    m_contentLayout->setSpacing(8);

    layout->addWidget(m_header);
    layout->addWidget(m_contentHost, 1);

    setStyleSheet(KisMobileStyle::rootStyleSheet());

    connect(m_closeButton, &QAbstractButton::clicked, this, &KisMobileSheet::closeRequested);
}

KisMobileSheet::~KisMobileSheet()
{
}

void KisMobileSheet::setContentWidget(QWidget *widget)
{
    if (QWidget *old = contentWidget()) {
        m_contentLayout->removeWidget(old);
        old->hide();
    }
    m_contentLayout->addWidget(widget);
    widget->show();
}

QWidget *KisMobileSheet::contentWidget() const
{
    return m_contentLayout->count() > 0 ? m_contentLayout->itemAt(0)->widget() : nullptr;
}

void KisMobileSheet::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

QString KisMobileSheet::title() const
{
    return m_titleLabel->text();
}

void KisMobileSheet::setPreferredHeight(int height)
{
    m_preferredHeight = height;
}

int KisMobileSheet::preferredHeight() const
{
    return m_preferredHeight;
}

QAbstractButton *KisMobileSheet::closeButton() const
{
    return m_closeButton;
}

bool KisMobileSheet::inHeaderArea(const QPoint &pos) const
{
    return pos.y() < m_header->geometry().bottom() &&
           pos.y() >= m_header->geometry().top() &&
           m_closeButton->geometry().adjusted(-8, -8, 8, 8).contains(pos) == false;
}

void KisMobileSheet::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setClipRect(event->rect());

    // bottom-sheet shape: rounded top corners only, flush at the lower edge
    const int radius = KisMobileStyle::sheetCornerRadius;
    QPainterPath path;
    path.moveTo(0, height());
    path.lineTo(0, radius);
    path.arcTo(0, 0, 2 * radius, 2 * radius, 180, -90);
    path.lineTo(width() - radius, 0);
    path.arcTo(width() - 2 * radius, 0, 2 * radius, 2 * radius, 90, -90);
    path.lineTo(width(), height());
    path.closeSubpath();
    painter.setPen(Qt::NoPen);
    painter.setBrush(KisMobileStyle::sheetBackgroundColor());
    painter.drawPath(path);

    // grab handle
    painter.setBrush(QColor(255, 255, 255, 70));
    painter.drawRoundedRect(QRect(width() / 2 - 26, 8, 52, 5), 2, 2);
}

void KisMobileSheet::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && inHeaderArea(event->pos())) {
        m_dragging = true;
        m_dragStartY = event->globalPos().y();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void KisMobileSheet::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void KisMobileSheet::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragging) {
        m_dragging = false;
        if (event->globalPos().y() - m_dragStartY > dragDismissThreshold) {
            Q_EMIT closeRequested();
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

/******************************************************************/
/* KisMobileScrim                                                 */
/******************************************************************/

KisMobileScrim::KisMobileScrim(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
}

void KisMobileScrim::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setClipRect(event->rect());
    painter.fillRect(rect(), KisMobileStyle::scrimColor());
}

void KisMobileScrim::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

/******************************************************************/
/* KisMobileColorSwatch                                           */
/******************************************************************/

KisMobileColorSwatch::KisMobileColorSwatch(QWidget *parent)
    : QAbstractButton(parent)
{
    setFixedSize(KisMobileStyle::smallButtonSize, KisMobileStyle::smallButtonSize);
    setAttribute(Qt::WA_AcceptTouchEvents, true);
}

void KisMobileColorSwatch::setColor(const QColor &color)
{
    if (m_color == color) return;
    m_color = color;
    update();
}

QColor KisMobileColorSwatch::color() const
{
    return m_color;
}

void KisMobileColorSwatch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF swatchRect = QRectF(rect()).adjusted(3, 3, -3, -3);

    painter.setPen(QPen(KisMobileStyle::separatorColor(), 1.5));
    painter.setBrush(m_color);
    painter.drawRoundedRect(swatchRect, 10, 10);

    if (isDown()) {
        painter.setBrush(QColor(255, 255, 255, 40));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(swatchRect, 10, 10);
    }
}

/******************************************************************/
/* KisMobileSliderRow                                             */
/******************************************************************/

KisMobileSliderRow::KisMobileSliderRow(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setFixedWidth(72);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPixelSize(12);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(QStringLiteral("color: rgba(158, 158, 166, 255);"));

    m_slider = new QSlider(Qt::Horizontal, this);
    KisMobileStyle::tuneSlider(m_slider);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setFixedWidth(52);
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_valueLabel);

    setMinimumHeight(KisMobileStyle::sliderRowHeight);

    connect(m_slider, &QSlider::valueChanged, this, &KisMobileSliderRow::slotSliderMoved);
}

void KisMobileSliderRow::setRange(qreal minimum, qreal maximum, int steps)
{
    m_minimum = minimum;
    m_maximum = maximum;
    m_steps = qMax(2, steps);
    m_slider->setRange(0, m_steps);
    setValue(value());
}

void KisMobileSliderRow::setValue(qreal value)
{
    const qreal clamped = qBound(m_minimum, value, m_maximum);
    const int position = qRound((clamped - m_minimum) / (m_maximum - m_minimum) * m_steps);
    const QSignalBlocker blocker(m_slider);
    m_slider->setValue(position);
    m_valueLabel->setText(formatValue(clamped));
}

qreal KisMobileSliderRow::value() const
{
    return m_minimum + (qreal(m_slider->value()) / m_steps) * (m_maximum - m_minimum);
}

void KisMobileSliderRow::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void KisMobileSliderRow::slotSliderMoved(int position)
{
    Q_UNUSED(position);
    const qreal newValue = value();
    m_valueLabel->setText(formatValue(newValue));
    Q_EMIT valueChanged(newValue);
}

QString KisMobileSliderRow::formatValue(qreal value) const
{
    return QString::number(qRound(value));
}

/******************************************************************/
/* KisMobileScrollArea                                            */
/******************************************************************/

KisMobileScrollArea::KisMobileScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setStyleSheet(KisMobileStyle::rootStyleSheet());
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    if (viewport()) {
        QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);
    }
}

/******************************************************************/
/* KisMobileTouch                                                 */
/******************************************************************/

namespace KisMobileTouch
{

void enableTouchScrolling(QWidget *root)
{
    if (!root) return;

    QList<QAbstractScrollArea *> areas = root->findChildren<QAbstractScrollArea *>();
    if (QAbstractScrollArea *self = qobject_cast<QAbstractScrollArea *>(root)) {
        areas.append(self);
    }

    for (QAbstractScrollArea *area : areas) {
        if (area->viewport()) {
            QScroller::grabGesture(area->viewport(), QScroller::LeftMouseButtonGesture);
        }
    }
}

}
