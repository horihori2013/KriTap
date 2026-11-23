/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_SHEETS_H
#define KIS_MOBILE_SHEETS_H

#include <QPointer>
#include <QWidget>

#include <functional>

class QAction;
class QGridLayout;
class QMenu;
class KisAction;
class KisViewManager;
class QToolButton;

/**
 * Scrollable grid with all available tools (from the KoToolRegistry), so
 * every Krita tool is reachable with a finger. Tools are grouped by their
 * sections and never overlap: buttons expand inside their grid cells and
 * the whole panel scrolls with a finger flick.
 */
class KisMobileToolsSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileToolsSheet(KisViewManager *viewManager, QWidget *parent = nullptr);

Q_SIGNALS:
    /// emitted after a tool was picked, so the sheet can close itself
    void finished();
};

/**
 * Secondary actions: file operations, preferences, immersive mode,
 * the switch back to the desktop UI and — most importantly — one entry
 * per top-level Krita menu, so nothing that lived in the menu bar or the
 * toolbars becomes unreachable on the phone.
 */
class KisMobileMoreSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileMoreSheet(KisViewManager *viewManager, QWidget *parent = nullptr);

Q_SIGNALS:
    void finished();
    void hideUiRequested();

private:
    QToolButton *addActionButton(QGridLayout *layout, int row, int column,
                                 const QString &actionName, const QString &caption);
    void addCustomButton(QGridLayout *layout, int row, int column,
                         const QString &iconName, const QString &caption,
                         const std::function<void()> &callback);
    QToolButton *makeGridButton(QWidget *parent, const QString &caption);
    void addMenuButtons(QGridLayout *layout, QWidget *parent);

private:
    QPointer<KisViewManager> m_viewManager;
};

#endif /* KIS_MOBILE_SHEETS_H */