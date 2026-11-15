/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_color_sheet.h"

#include "kis_mobile_style.h"
#include "kis_mobile_widgets.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

#include <KoColor.h>

#include "kis_canvas_resource_provider.h"

namespace
{
    const int pickerHeight = 170;

    void paintCheckerboard(QPainter &painter, const QRect &rect, int cellSize = 8)
    {
        painter.save();
        painter.setClipRect(rect);
        painter.fillRect(rect, QColor(255, 255, 255));
        painter.setBrush(QColor(180, 180, 180));
        painter.setPen(Qt::NoPen);
        for (int y = rect.top(); y <= rect.bottom(); y += cellSize) {
            for (int x = rect.left(); x <= rect.right(); x += cellSize) {
                if (((x - rect.left()) / cellSize + (y - rect.top()) / cellSize) % 2 == 0) {
                    painter.drawRect(QRect(x, y, cellSize, cellSize));
                }
            }
        }
        painter.restore();
    }
}

/******************************************************************/
/* Small internal widgets                                          */
/******************************************************************/

/**
 * Saturation/value square for a fixed hue.
 */
class KisMobileSvSquare : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileSvSquare(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(pickerHeight);
        setAttribute(Qt::WA_AcceptTouchEvents, true);
    }

    void setHsv(qreal hue, qreal saturation, qreal value)
    {
        m_hue = hue;
        m_saturation = saturation;
        m_value = value;
        update();
    }

Q_SIGNALS:
    void svChanged(qreal saturation, qreal value);
    void commitRequested();

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF area = QRectF(rect()).adjusted(1, 1, -1, -1);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor::fromHsvF(qBound<qreal>(0.0, m_hue, 1.0), 1.0, 1.0));
        painter.drawRect(area);

        QLinearGradient whiteGradient(area.topLeft(), area.topRight());
        whiteGradient.setColorAt(0.0, QColor(255, 255, 255, 255));
        whiteGradient.setColorAt(1.0, QColor(255, 255, 255, 0));
        painter.setBrush(whiteGradient);
        painter.drawRect(area);

        QLinearGradient blackGradient(area.bottomLeft(), area.topLeft());
        blackGradient.setColorAt(0.0, QColor(0, 0, 0, 255));
        blackGradient.setColorAt(1.0, QColor(0, 0, 0, 0));
        painter.setBrush(blackGradient);
        painter.drawRect(area);

        // marker
        const QPointF marker(area.left() + m_saturation * area.width(),
                              area.top() + (1.0 - m_value) * area.height());
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(255, 255, 255, 230), 2.5));
        painter.drawEllipse(marker, 8, 8);
        painter.setPen(QPen(QColor(0, 0, 0, 160), 1.5));
        painter.drawEllipse(marker, 10, 10);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        updateFromPosition(event->pos());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (event->buttons() & Qt::LeftButton) {
            updateFromPosition(event->pos());
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        updateFromPosition(event->pos());
        Q_EMIT commitRequested();
        event->accept();
    }

private:
    void updateFromPosition(const QPoint &pos)
    {
        const qreal s = qBound<qreal>(0.0, qreal(pos.x()) / qMax(1, width()), 1.0);
        const qreal v = 1.0 - qBound<qreal>(0.0, qreal(pos.y()) / qMax(1, height()), 1.0);
        m_saturation = s;
        m_value = v;
        update();
        Q_EMIT svChanged(s, v);
    }

private:
    qreal m_hue {0.0};
    qreal m_saturation {0.0};
    qreal m_value {1.0};
};

/**
 * Vertical-free rainbow hue bar.
 */
class KisMobileHueBar : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileHueBar(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFixedHeight(28);
        setAttribute(Qt::WA_AcceptTouchEvents, true);
    }

    void setHue(qreal hue)
    {
        m_hue = hue;
        update();
    }

Q_SIGNALS:
    void hueChanged(qreal hue);
    void commitRequested();

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);

        const QRectF area = QRectF(rect()).adjusted(1, 1, -1, -1);

        QLinearGradient gradient(area.topLeft(), area.topRight());
        for (int i = 0; i <= 12; ++i) {
            gradient.setColorAt(qreal(i) / 12.0, QColor::fromHsvF(qreal(i) / 12.0 * 0.999, 1.0, 1.0));
        }
        painter.fillRect(area, gradient);

        // marker
        const qreal x = area.left() + qBound<qreal>(0.0, m_hue, 1.0) * area.width();
        painter.setPen(QPen(QColor(255, 255, 255, 230), 2.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(x - 4, area.top(), 8, area.height()), 3, 3);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        updateFromPosition(event->pos());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (event->buttons() & Qt::LeftButton) {
            updateFromPosition(event->pos());
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        updateFromPosition(event->pos());
        Q_EMIT commitRequested();
        event->accept();
    }

private:
    void updateFromPosition(const QPoint &pos)
    {
        m_hue = qBound<qreal>(0.0, qreal(pos.x()) / qMax(1, width()), 1.0);
        update();
        Q_EMIT hueChanged(m_hue);
    }

private:
    qreal m_hue {0.0};
};

/**
 * Horizontal alpha bar with checkerboard background.
 */
class KisMobileAlphaBar : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileAlphaBar(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFixedHeight(28);
        setAttribute(Qt::WA_AcceptTouchEvents, true);
    }

    void setAlpha(qreal alpha)
    {
        m_alpha = alpha;
        update();
    }

    void setBaseColor(const QColor &color)
    {
        m_baseColor = color;
        update();
    }

Q_SIGNALS:
    void alphaChanged(qreal alpha);
    void commitRequested();

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);

        const QRectF area = QRectF(rect()).adjusted(1, 1, -1, -1);
        paintCheckerboard(painter, area.toRect());

        QLinearGradient gradient(area.topLeft(), area.topRight());
        QColor opaque = m_baseColor;
        opaque.setAlpha(255);
        QColor transparent = opaque;
        transparent.setAlpha(0);
        gradient.setColorAt(0.0, transparent);
        gradient.setColorAt(1.0, opaque);
        painter.fillRect(area, gradient);

        const qreal x = area.left() + qBound<qreal>(0.0, m_alpha, 1.0) * area.width();
        painter.setPen(QPen(QColor(255, 255, 255, 230), 2.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(x - 4, area.top(), 8, area.height()), 3, 3);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        updateFromPosition(event->pos());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (event->buttons() & Qt::LeftButton) {
            updateFromPosition(event->pos());
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        updateFromPosition(event->pos());
        Q_EMIT commitRequested();
        event->accept();
    }

private:
    void updateFromPosition(const QPoint &pos)
    {
        m_alpha = qBound<qreal>(0.0, qreal(pos.x()) / qMax(1, width()), 1.0);
        update();
        Q_EMIT alphaChanged(m_alpha);
    }

private:
    qreal m_alpha {1.0};
    QColor m_baseColor {Qt::black};
};

/**
 * Composition of the pickers above plus a color history row.
 */
class KisMobileColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileColorPicker(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        m_svSquare = new KisMobileSvSquare(this);
        m_hueBar = new KisMobileHueBar(this);
        m_alphaBar = new KisMobileAlphaBar(this);

        QWidget *previewRow = new QWidget(this);
        QHBoxLayout *previewLayout = new QHBoxLayout(previewRow);
        previewLayout->setContentsMargins(0, 0, 0, 0);
        previewLayout->setSpacing(10);

        m_previewSwatch = new KisMobileColorSwatch(previewRow);
        m_previewSwatch->setFixedSize(38, 38);
        m_valueLabel = new QLabel(previewRow);
        m_valueLabel->setStyleSheet(QStringLiteral("color: rgba(158, 158, 166, 255);"));

        previewLayout->addWidget(m_previewSwatch);
        previewLayout->addWidget(m_valueLabel, 1);

        m_historyHost = new QWidget(this);
        m_historyLayout = new QHBoxLayout(m_historyHost);
        m_historyLayout->setContentsMargins(0, 0, 0, 0);
        m_historyLayout->setSpacing(6);
        m_historyLayout->addStretch(1);

        layout->addWidget(m_svSquare, 1);
        layout->addWidget(m_hueBar);
        layout->addWidget(m_alphaBar);
        layout->addWidget(previewRow);
        layout->addWidget(m_historyHost);

        connect(m_svSquare, &KisMobileSvSquare::svChanged, this, &KisMobileColorPicker::slotSvChanged);
        connect(m_hueBar, &KisMobileHueBar::hueChanged, this, &KisMobileColorPicker::slotHueChanged);
        connect(m_alphaBar, &KisMobileAlphaBar::alphaChanged, this, &KisMobileColorPicker::slotAlphaChanged);

        connect(m_svSquare, &KisMobileSvSquare::commitRequested, this, &KisMobileColorPicker::slotCommit);
        connect(m_hueBar, &KisMobileHueBar::commitRequested, this, &KisMobileColorPicker::slotCommit);
        connect(m_alphaBar, &KisMobileAlphaBar::commitRequested, this, &KisMobileColorPicker::slotCommit);
    }

    QColor currentColor() const
    {
        QColor color = QColor::fromHsvF(qBound<qreal>(0.0, m_hue, 1.0),
                                        qBound<qreal>(0.0, m_saturation, 1.0),
                                        qBound<qreal>(0.0, m_value, 1.0));
        color.setAlphaF(qBound<qreal>(0.0, m_alpha, 1.0));
        return color;
    }

    void setCurrentColor(const QColor &color, bool notify)
    {
        int hue = 0;
        int saturation = 0;
        int value = 0;
        color.getHsv(&hue, &saturation, &value);

        if (hue >= 0) {
            m_hue = qreal(hue) / 360.0;
        }
        m_saturation = qreal(saturation) / 255.0;
        m_value = qreal(value) / 255.0;
        m_alpha = color.alphaF();

        updateChildren();

        if (notify) {
            Q_EMIT colorChanged(currentColor());
            Q_EMIT colorCommitted(currentColor());
        }
    }

    void setHistory(const QList<QColor> &colors)
    {
        for (QWidget *widget : m_historyWidgets) {
            delete widget;
        }
        m_historyWidgets.clear();

        const int maxItems = 10;
        const int start = qMax(0, colors.size() - maxItems);

        for (int i = start; i < colors.size(); ++i) {
            KisMobileColorSwatch *swatch = new KisMobileColorSwatch(m_historyHost);
            swatch->setFixedSize(30, 30);
            swatch->setColor(colors[i]);
            connect(swatch, &QAbstractButton::clicked, this, [this, swatch]() {
                setCurrentColor(swatch->color(), true);
            });
            m_historyLayout->insertWidget(m_historyLayout->count() - 1, swatch);
            m_historyWidgets.append(swatch);
        }
    }

Q_SIGNALS:
    void colorChanged(const QColor &color);
    void colorCommitted(const QColor &color);

private Q_SLOTS:
    void slotSvChanged(qreal saturation, qreal value)
    {
        m_saturation = saturation;
        m_value = value;
        updateChildren();
        Q_EMIT colorChanged(currentColor());
    }

    void slotHueChanged(qreal hue)
    {
        m_hue = hue;
        updateChildren();
        Q_EMIT colorChanged(currentColor());
    }

    void slotAlphaChanged(qreal alpha)
    {
        m_alpha = alpha;
        updateChildren();
        Q_EMIT colorChanged(currentColor());
    }

    void slotCommit()
    {
        Q_EMIT colorCommitted(currentColor());
    }

private:
    void updateChildren()
    {
        const QColor color = currentColor();
        m_svSquare->setHsv(m_hue, m_saturation, m_value);
        m_hueBar->setHue(m_hue);
        m_alphaBar->setBaseColor(color);
        m_alphaBar->setAlpha(m_alpha);
        m_previewSwatch->setColor(color);
        m_valueLabel->setText(QStringLiteral("#%1%2%3")
                              .arg(color.red(), 2, 16, QLatin1Char('0'))
                              .arg(color.green(), 2, 16, QLatin1Char('0'))
                              .arg(color.blue(), 2, 16, QLatin1Char('0')));
    }

private:
    KisMobileSvSquare *m_svSquare {nullptr};
    KisMobileHueBar *m_hueBar {nullptr};
    KisMobileAlphaBar *m_alphaBar {nullptr};
    KisMobileColorSwatch *m_previewSwatch {nullptr};
    QLabel *m_valueLabel {nullptr};
    QWidget *m_historyHost {nullptr};
    QHBoxLayout *m_historyLayout {nullptr};
    QList<QWidget *> m_historyWidgets;
    qreal m_hue {0.0};
    qreal m_saturation {0.0};
    qreal m_value {1.0};
    qreal m_alpha {1.0};
};

/******************************************************************/
/* KisMobileColorSheet                                            */
/******************************************************************/

KisMobileColorSheet::KisMobileColorSheet(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(8);

    m_picker = new KisMobileColorPicker(this);
    layout->addWidget(m_picker, 1);

    connect(m_picker, &KisMobileColorPicker::colorChanged, this, &KisMobileColorSheet::slotPickerColorChanged);
    connect(m_picker, &KisMobileColorPicker::colorCommitted, this, &KisMobileColorSheet::slotPickerColorCommitted);
}

void KisMobileColorSheet::setResourceProvider(KisCanvasResourceProvider *provider)
{
    if (m_provider) {
        disconnect(m_provider, nullptr, this, nullptr);
    }

    m_provider = provider;

    if (!m_provider) return;

    connect(m_provider, &KisCanvasResourceProvider::sigFGColorChanged,
            this, &KisMobileColorSheet::slotFgColorChanged);
    connect(m_provider, &KisCanvasResourceProvider::sigFGColorUsed,
            this, [this](const KoColor &color) {
                addHistoryColor(color.toQColor());
            });

    slotFgColorChanged(m_provider->fgColor());
}

void KisMobileColorSheet::aboutToShow()
{
    if (m_provider) {
        slotFgColorChanged(m_provider->fgColor());
    }
}

void KisMobileColorSheet::slotFgColorChanged(const KoColor &color)
{
    if (m_blockProviderUpdates) return;

    m_blockProviderUpdates = true;
    m_picker->setCurrentColor(color.toQColor(), false);
    m_blockProviderUpdates = false;
}

void KisMobileColorSheet::slotPickerColorChanged(const QColor &color)
{
    if (!m_provider || m_blockProviderUpdates) return;

    KoColor newColor = m_provider->fgColor();
    newColor.fromQColor(color);

    m_blockProviderUpdates = true;
    m_provider->setFGColor(newColor);
    m_blockProviderUpdates = false;
}

void KisMobileColorSheet::slotPickerColorCommitted(const QColor &color)
{
    slotPickerColorChanged(color);
    addHistoryColor(color);
}

void KisMobileColorSheet::addHistoryColor(const QColor &color)
{
    QColor opaque = color;
    opaque.setAlpha(255);

    // replace duplicates, newest color is last
    m_history.removeAll(opaque);
    m_history.append(opaque);

    while (m_history.size() > 10) {
        m_history.removeFirst();
    }

    m_picker->setHistory(m_history);
}

#include "kis_mobile_color_sheet.moc"