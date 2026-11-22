/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_STYLE_H
#define KIS_MOBILE_STYLE_H

#include <QColor>
#include <QString>

class QAbstractButton;
class QSlider;
class QToolButton;
class QWidget;

/**
 * Shared look-and-feel constants and helpers for the mobile UI.
 *
 * The mobile UI imitates smartphone painting applications (ibisPaint-style):
 * large touch targets, translucent dark floating bars over the canvas and
 * slide-up bottom sheets.
 */
namespace KisMobileStyle
{
    /// Metric constants (in device-independent pixels)
    const int barHeight = 64;
    const int buttonSize = 52;
    const int smallButtonSize = 44;
    const int iconSize = 26;
    const int smallIconSize = 20;
    const int sheetCornerRadius = 18;
    const int panelMargin = 10;
    const int sliderRowHeight = 44;
    const int dragHandleHeight = 22;

    /// Palette
    QColor barBackgroundColor();
    QColor sheetBackgroundColor();
    QColor scrimColor();
    QColor accentColor();
    QColor textColor();
    QColor dimTextColor();
    QColor separatorColor();
    QColor buttonBackgroundColor();
    QColor buttonPressedColor();
    QColor dangerColor();

    /// Style sheet applied to every root widget of the mobile UI
    QString rootStyleSheet();

    /// Apply/restore a dark phone-like theme to the whole application
    /// (dialog boxes and desktop widgets included).
    void applyApplicationTheme();
    void restoreApplicationTheme();

    /// Create a large square touch button (icon only)
    QToolButton *createIconButton(QWidget *parent,
                                  const QString &toolTip,
                                  int buttonSize = KisMobileStyle::buttonSize,
                                  int iconSize = KisMobileStyle::iconSize);

    /// Create a large touch button with an icon and a caption underneath
    QToolButton *createCaptionButton(QWidget *parent,
                                     const QString &caption,
                                     const QString &toolTip,
                                     int buttonSize = KisMobileStyle::buttonSize);

    /// Size a slider for comfortable finger use
    void tuneSlider(QSlider *slider);
}

#endif /* KIS_MOBILE_STYLE_H */
