/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_COLOR_SHEET_H
#define KIS_MOBILE_COLOR_SHEET_H

#include <QColor>
#include <QList>
#include <QPointer>
#include <QWidget>

class KisCanvasResourceProvider;
class KisMobileColorPicker;
class KoColor;

/**
 * Bottom sheet with a smartphone friendly color picker:
 * saturation/value square, hue bar, opacity bar and a color history row.
 * Connected to the canvas resource provider's foreground color.
 */
class KisMobileColorSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileColorSheet(QWidget *parent = nullptr);

    void setResourceProvider(KisCanvasResourceProvider *provider);

public Q_SLOTS:
    void aboutToShow();

private Q_SLOTS:
    void slotFgColorChanged(const KoColor &color);
    void slotPickerColorChanged(const QColor &color);
    void slotPickerColorCommitted(const QColor &color);

private:
    void addHistoryColor(const QColor &color);

private:
    QPointer<KisCanvasResourceProvider> m_provider;
    KisMobileColorPicker *m_picker {nullptr};
    QList<QColor> m_history;
    bool m_blockProviderUpdates {false};
};

#endif /* KIS_MOBILE_COLOR_SHEET_H */
