/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_sheets.h"

#include "kis_mobile_style.h"
#include "kis_mobile_widgets.h"

#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

#include <KoToolFactoryBase.h>
#include <KoToolRegistry.h>
#include <kactioncollection.h>
#include <kis_action.h>
#include <kis_icon.h>
#include <klocalizedstring.h>

#include "KisMainWindow.h"
#include "KisViewManager.h"
#include "kis_action_manager.h"

namespace
{
    QString cleanText(const QString &text)
    {
        QString result = text;
        result.remove(QLatin1Char('&'));
        return result;
    }

    /// uniform grid cell button that expands in its cell (never overlaps)
    QToolButton *makeTouchGridButton(QWidget *parent, const QString &caption)
    {
        QToolButton *button = KisMobileStyle::createCaptionButton(parent, caption, caption, 84);
        button->setMinimumWidth(84);
        button->setMinimumHeight(KisMobileStyle::buttonSize + 18);
        button->setMaximumHeight(KisMobileStyle::buttonSize + 30);
        button->setMaximumWidth(QWIDGETSIZE_MAX);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        return button;
    }

    QLabel *makeSectionHeader(QWidget *parent, const QString &title)
    {
        QLabel *header = new QLabel(title, parent);
        QFont font = header->font();
        font.setBold(true);
        font.setPixelSize(12);
        header->setFont(font);
        header->setStyleSheet(QStringLiteral("color: %1; padding: 6px 2px 2px 2px;")
                              .arg(KisMobileStyle::dimTextColor().name()));
        return header;
    }

    void addSection(QVBoxLayout *containerLayout, QWidget *container,
                    const QString &title, QGridLayout **gridOut)
    {
        containerLayout->addWidget(makeSectionHeader(container, title));
        QGridLayout *grid = new QGridLayout;
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(8);
        containerLayout->addLayout(grid);
        containerLayout->addSpacing(6);
        *gridOut = grid;
    }
}

/******************************************************************/
/* KisMobileToolsSheet                                            */
/******************************************************************/

KisMobileToolsSheet::KisMobileToolsSheet(KisViewManager *viewManager, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    KisMobileScrollArea *scroll = new KisMobileScrollArea(this);
    QWidget *container = new QWidget(scroll);
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(12, 8, 12, 16);
    containerLayout->setSpacing(0);

    layout->addWidget(scroll);
    scroll->setWidget(container);

    if (!viewManager) {
        return;
    }

    QList<KoToolFactoryBase *> factories = KoToolRegistry::instance()->values();
    std::sort(factories.begin(), factories.end(),
              [](KoToolFactoryBase *a, KoToolFactoryBase *b) {
                  if (a->section() != b->section()) {
                      return a->section() < b->section();
                  }
                  return a->priority() < b->priority();
                });

    const int columns = 3;
    QString currentSection;
    QGridLayout *grid = nullptr;
    int row = 0;
    int column = 0;

    for (KoToolFactoryBase *factory : factories) {
        if (!factory) continue;

        QAction *action = viewManager->actionCollection()->action(factory->id());
        if (!action) continue;

        const QString section = cleanText(factory->section());
        if (section != currentSection || !grid) {
            currentSection = section;
            addSection(containerLayout, container,
                       section.isEmpty() ? i18n("Tools") : section, &grid);
            row = 0;
            column = 0;
        }

        const QString caption = cleanText(action->text());
        QToolButton *button = makeTouchGridButton(container, caption);
        if (action->icon().isNull()) {
            button->setIcon(KisIconUtils::loadIcon(factory->iconName()));
        } else {
            button->setIcon(action->icon());
        }

        connect(button, &QAbstractButton::clicked, this, [this, action]() {
            action->trigger();
            Q_EMIT finished();
        });
        button->setEnabled(action->isEnabled());
        connect(action, &QAction::changed, button, [button, action]() {
            button->setEnabled(action->isEnabled());
        });

        grid->addWidget(button, row, column);

        if (++column == columns) {
            column = 0;
            ++row;
        }
    }

    containerLayout->addStretch(1);
}

/******************************************************************/
/* KisMobileMoreSheet                                             */
/******************************************************************/

KisMobileMoreSheet::KisMobileMoreSheet(KisViewManager *viewManager, QWidget *parent)
    : QWidget(parent)
    , m_viewManager(viewManager)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    KisMobileScrollArea *scroll = new KisMobileScrollArea(this);
    QWidget *container = new QWidget(scroll);
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(12, 8, 12, 16);
    containerLayout->setSpacing(0);

    layout->addWidget(scroll);
    scroll->setWidget(container);

    // quick actions
    QGridLayout *quickGrid = nullptr;
    addSection(containerLayout, container, i18n("Quick actions"), &quickGrid);

    addActionButton(quickGrid, 0, 0, "file_new", i18n("New"));
    addActionButton(quickGrid, 0, 1, "file_open", i18n("Open"));
    addActionButton(quickGrid, 0, 2, "file_save", i18n("Save"));
    addActionButton(quickGrid, 1, 0, "file_save_as", i18n("Save As"));
    addActionButton(quickGrid, 1, 1, "file_export_file", i18n("Export"));
    addActionButton(quickGrid, 1, 2, "options_configure", i18n("Settings"));

    addCustomButton(quickGrid, 2, 0, "view-fullscreen", i18n("Hide UI"),
                    [this]() {
                        Q_EMIT hideUiRequested();
                        Q_EMIT finished();
                    });

    // every top-level menu of Krita stays reachable
    QGridLayout *menusGrid = nullptr;
    addSection(containerLayout, container, i18n("Menus"), &menusGrid);
    addMenuButtons(menusGrid, container);

    containerLayout->addStretch(1);
}

QToolButton *KisMobileMoreSheet::makeGridButton(QWidget *parent, const QString &caption)
{
    return makeTouchGridButton(parent, caption);
}

void KisMobileMoreSheet::addMenuButtons(QGridLayout *layout, QWidget *parent)
{
    KisMainWindow *mainWindow = m_viewManager ? m_viewManager->mainWindow() : nullptr;
    QMenuBar *menuBar = mainWindow ? mainWindow->menuBar() : nullptr;
    if (!menuBar) return;

    const int columns = 3;
    int row = 0;
    int column = 0;

    const QList<QAction *> menuActions = menuBar->actions();
    for (QAction *menuAction : menuActions) {
        QMenu *menu = menuAction->menu();
        if (!menu || menu->isEmpty()) continue;

        const QString caption = cleanText(menuAction->text());
        QToolButton *button = makeGridButton(parent, caption);

        QIcon icon = menuAction->icon();
        if (icon.isNull() && !menu->actions().isEmpty()) {
            icon = menu->actions().first()->icon();
        }
        if (icon.isNull()) {
            icon = KisIconUtils::loadIcon("application-menu");
        }
        button->setIcon(icon);

        connect(button, &QAbstractButton::clicked, this, [menu, button]() {
            const QPoint pos = button->mapToGlobal(QPoint(0, 0));
            menu->popup(pos);
        });

        // close the sheet whenever something is picked from the menu
        connect(menu, &QMenu::triggered, this, [this]() {
            Q_EMIT finished();
        });

        layout->addWidget(button, row, column);

        if (++column == columns) {
            column = 0;
            ++row;
        }
    }
}

QToolButton *KisMobileMoreSheet::addActionButton(QGridLayout *layout, int row, int column,
                                                 const QString &actionName, const QString &caption)
{
    QAction *action = m_viewManager ? m_viewManager->actionCollection()->action(actionName) : nullptr;
    if (!action && m_viewManager) {
        action = m_viewManager->actionManager()->actionByName(actionName);
    }
    if (!action) return nullptr;

    QToolButton *button = makeGridButton(this, caption);
    button->setIcon(action->icon());

    connect(button, &QAbstractButton::clicked, this, [this, action]() {
        action->trigger();
        Q_EMIT finished();
    });
    button->setEnabled(action->isEnabled());
    connect(action, &QAction::changed, button, [button, action]() {
        button->setEnabled(action->isEnabled());
    });

    layout->addWidget(button, row, column);
    return button;
}

void KisMobileMoreSheet::addCustomButton(QGridLayout *layout, int row, int column,
                                         const QString &iconName, const QString &caption,
                                         const std::function<void()> &callback)
{
    QToolButton *button = makeGridButton(this, caption);
    button->setIcon(KisIconUtils::loadIcon(iconName));

    connect(button, &QAbstractButton::clicked, this, callback);

    layout->addWidget(button, row, column);
}