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

class QGridLayout;
class KisAction;
class KisViewManager;
class QToolButton;

/**
 * Grid with all available tools (from the KoToolRegistry), so every
 * Krita tool is reachable with a finger.
 */
class KisMobileToolsSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileToolsSheet(KisViewManager *viewManager, QWidget *parent = nullptr);
};

/**
 * Secondary actions: file operations, preferences, immersive mode
 * and the switch back to the desktop UI.
 */
class KisMobileMoreSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileMoreSheet(KisViewManager *viewManager, QWidget *parent = nullptr);

Q_SIGNALS:
    void hideUiRequested();
    void disableMobileUiRequested();

private:
    QToolButton *addActionButton(QGridLayout *layout, int row, int column,
                                 const QString &actionName, const QString &caption);
    void addCustomButton(QGridLayout *layout, int row, int column,
                         const QString &iconName, const QString &caption,
                         const std::function<void()> &callback);

private:
    QPointer<KisViewManager> m_viewManager;
};

#endif /* KIS_MOBILE_SHEETS_H */
