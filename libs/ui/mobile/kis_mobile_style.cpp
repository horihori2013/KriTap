/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_style.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPalette>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

namespace
{
    QString rgbaString(const QColor &color)
    {
        return QStringLiteral("rgba(%1, %2, %3, %4)")
                .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    }

    QPalette s_savedPalette;
    bool s_themeApplied = false;

    QPalette mobileDarkPalette()
    {
        QPalette palette;
        const QColor background(26, 26, 31);
        const QColor base(34, 34, 40);
        const QColor alt(42, 42, 49);
        palette.setColor(QPalette::Window, background);
        palette.setColor(QPalette::WindowText, KisMobileStyle::textColor());
        palette.setColor(QPalette::Base, base);
        palette.setColor(QPalette::AlternateBase, alt);
        palette.setColor(QPalette::ToolTipBase, KisMobileStyle::sheetBackgroundColor());
        palette.setColor(QPalette::ToolTipText, KisMobileStyle::textColor());
        palette.setColor(QPalette::Text, KisMobileStyle::textColor());
        palette.setColor(QPalette::Button, alt);
        palette.setColor(QPalette::ButtonText, KisMobileStyle::textColor());
        palette.setColor(QPalette::BrightText, QColor(255, 120, 120));
        palette.setColor(QPalette::Link, KisMobileStyle::accentColor());
        palette.setColor(QPalette::Highlight, KisMobileStyle::accentColor());
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette.setColor(QPalette::PlaceholderText, KisMobileStyle::dimTextColor());
#endif
        return palette;
    }
}

namespace KisMobileStyle
{

QColor barBackgroundColor()      { return QColor(30, 30, 34, 230); }
QColor sheetBackgroundColor()    { return QColor(38, 38, 44, 250); }
QColor scrimColor()              { return QColor(0, 0, 0, 110); }
QColor accentColor()             { return QColor(0, 162, 255); }
QColor textColor()               { return QColor(236, 236, 236); }
QColor dimTextColor()            { return QColor(158, 158, 166); }
QColor separatorColor()          { return QColor(255, 255, 255, 32); }
QColor buttonBackgroundColor()   { return QColor(255, 255, 255, 18); }
QColor buttonPressedColor()      { return QColor(255, 255, 255, 44); }
QColor dangerColor()             { return QColor(255, 90, 90); }

QString rootStyleSheet()
{
    return QStringLiteral(
        "* {"
        "   color: %1;"
        "   font-size: 13px;"
        "}"
        "QLabel {"
        "   background: transparent;"
        "   border: none;"
        "}"
        "QFrame[frameShape=\"4\"], QFrame[frameShape=\"5\"] {"
        "   color: %2;"
        "}"
        "QToolButton {"
        "   background-color: %3;"
        "   border: none;"
        "   border-radius: 12px;"
        "   padding: 4px;"
        "}"
        "QToolButton:hover {"
        "   background-color: %4;"
        "}"
        "QToolButton:pressed, QToolButton:checked {"
        "   background-color: %5;"
        "}"
        "QToolButton:disabled {"
        "   color: %6;"
        "}"
        "QToolButton::menu-indicator {"
        "   image: none;"
        "}"
        "QSlider::groove:horizontal {"
        "   height: 6px;"
        "   border-radius: 3px;"
        "   background-color: rgba(255, 255, 255, 48);"
        "}"
        "QSlider::sub-page:horizontal {"
        "   border-radius: 3px;"
        "   background-color: %5;"
        "}"
        "QSlider::handle:horizontal {"
        "   width: 28px;"
        "   height: 28px;"
        "   margin: -11px 0;"
        "   border-radius: 14px;"
        "   background-color: #ffffff;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   background-color: %5;"
        "}"
        "QSlider:disabled::handle:horizontal {"
        "   background-color: %6;"
        "}"
        "QComboBox {"
        "   background-color: %3;"
        "   border: 1px solid %2;"
        "   border-radius: 8px;"
        "   padding: 6px 10px;"
        "   min-height: 24px;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: 28px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: %7;"
        "   border: 1px solid %2;"
        "   selection-background-color: %5;"
        "}"
        "QMenu {"
        "   background-color: %7;"
        "   border: 1px solid %2;"
        "   padding: 6px;"
        "}"
        "QMenu::item {"
        "   padding: 8px 24px;"
        "   border-radius: 6px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: %5;"
        "}"
        "QScrollBar:vertical {"
        "   width: 6px;"
        "   background: transparent;"
        "}"
        "QScrollBar::handle:vertical {"
        "   min-height: 30px;"
        "   border-radius: 3px;"
        "   background-color: rgba(255, 255, 255, 60);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        "QTreeView, QListView {"
        "   background-color: transparent;"
        "   border: none;"
        "   outline: none;"
        "}"
        "QLineEdit, QSpinBox, QDoubleSpinBox {"
        "   background-color: %3;"
        "   border: 1px solid %2;"
        "   border-radius: 8px;"
        "   padding: 6px 10px;"
        "}"
        ).arg(rgbaString(textColor()),
              rgbaString(separatorColor()),
              rgbaString(buttonBackgroundColor()),
              rgbaString(buttonPressedColor()),
              rgbaString(accentColor()),
              rgbaString(dimTextColor()),
              rgbaString(QColor(46, 46, 54, 255)));
}

QToolButton *createIconButton(QWidget *parent, const QString &toolTip, int buttonSize, int iconSize)
{
    QToolButton *button = new QToolButton(parent);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIconSize(QSize(iconSize, iconSize));
    button->setFixedSize(buttonSize, buttonSize);
    button->setToolTip(toolTip);
    button->setAutoRaise(false);
    button->setAttribute(Qt::WA_AcceptTouchEvents, true);
    return button;
}

QToolButton *createCaptionButton(QWidget *parent, const QString &caption, const QString &toolTip, int buttonSize)
{
    QToolButton *button = new QToolButton(parent);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIconSize(QSize(iconSize, iconSize));
    button->setFixedSize(buttonSize, buttonSize + 16);
    button->setText(caption);
    button->setToolTip(toolTip);
    QFont font = button->font();
    font.setPixelSize(10);
    button->setFont(font);
    button->setAttribute(Qt::WA_AcceptTouchEvents, true);
    return button;
}

void tuneSlider(QSlider *slider)
{
    slider->setOrientation(Qt::Horizontal);
    slider->setMinimumHeight(32);
    slider->setAttribute(Qt::WA_AcceptTouchEvents, true);
}

void applyApplicationTheme()
{
    QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (!app || s_themeApplied) return;

    s_savedPalette = app->palette();
    app->setPalette(mobileDarkPalette());
    s_themeApplied = true;
}

void restoreApplicationTheme()
{
    QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (!app || !s_themeApplied) return;

    app->setPalette(s_savedPalette);
    s_themeApplied = false;
}

}
