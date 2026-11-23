/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_WIDGETS_H
#define KIS_MOBILE_WIDGETS_H

#include <QAbstractButton>
#include <QColor>
#include <QLabel>
#include <QPointer>
#include <QScrollArea>
#include <QWidget>

class QSlider;
class QVBoxLayout;

/**
 * A slide-up bottom sheet: rounded translucent panel with a title row
 * and a drag handle that can be dragged down to dismiss the sheet.
 */
class KisMobileSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileSheet(const QString &title, QWidget *parent = nullptr);
    ~KisMobileSheet() override;

    void setContentWidget(QWidget *widget);
    QWidget *contentWidget() const;

    void setTitle(const QString &title);
    QString title() const;

    void setPreferredHeight(int height);
    int preferredHeight() const;

    /// The close button in the title row
    QAbstractButton *closeButton() const;

Q_SIGNALS:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool inHeaderArea(const QPoint &pos) const;

private:
    QLabel *m_titleLabel {nullptr};
    QAbstractButton *m_closeButton {nullptr};
    QWidget *m_header {nullptr};
    QWidget *m_contentHost {nullptr};
    QVBoxLayout *m_contentLayout {nullptr};
    int m_preferredHeight {0};
    bool m_dragging {false};
    int m_dragStartY {0};
};

/**
 * Translucent overlay that catches clicks to dismiss a sheet.
 */
class KisMobileScrim : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileScrim(QWidget *parent = nullptr);

Q_SIGNALS:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

/**
 * A round-ish color swatch button showing the current foreground color.
 */
class KisMobileColorSwatch : public QAbstractButton
{
    Q_OBJECT
public:
    explicit KisMobileColorSwatch(QWidget *parent = nullptr);

    void setColor(const QColor &color);
    QColor color() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color {Qt::black};
};

/**
 * A labeled slider row with a large title, the slider and the current value.
 * The value is a floating point number inside [minimum(); maximum()].
 */
class KisMobileSliderRow : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileSliderRow(const QString &title, QWidget *parent = nullptr);

    void setRange(qreal minimum, qreal maximum, int steps = 1000);
    void setValue(qreal value);
    qreal value() const;

    void setTitle(const QString &title);

Q_SIGNALS:
    void valueChanged(qreal value);

private Q_SLOTS:
    void slotSliderMoved(int position);

private:
    QString formatValue(qreal value) const;

private:
    QLabel *m_titleLabel {nullptr};
    QLabel *m_valueLabel {nullptr};
    QSlider *m_slider {nullptr};
    qreal m_minimum {0.0};
    qreal m_maximum {1.0};
    int m_steps {1000};
};

/**
 * A scroll area with kinetic finger scrolling (drag anywhere inside,
 * including on top of buttons, without triggering them by accident).
 */
class KisMobileScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit KisMobileScrollArea(QWidget *parent = nullptr);
};

namespace KisMobileTouch
{
    /// Enable kinetic finger scrolling on every scrollable widget below root.
    void enableTouchScrolling(QWidget *root);
}

#endif /* KIS_MOBILE_WIDGETS_H */
