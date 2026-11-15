/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_sheets.h"

#include "kis_mobile_style.h"

#include <QGridLayout>
#include <QToolButton>

#include <algorithm>

#include <KoToolFactoryBase.h>
#include <KoToolRegistry.h>
#include <kactioncollection.h>
#include <kis_action.h>
#include <kis_icon.h>
#include <klocalizedstring.h>

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
}

/******************************************************************/
/* KisMobileToolsSheet                                            */
/******************************************************************/

KisMobileToolsSheet::KisMobileToolsSheet(KisViewManager *viewManager, QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *grid = new QGridLayout(this);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setSpacing(8);

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

    const int columns = 4;
    int row = 0;
    int column = 0;

    for (KoToolFactoryBase *factory : factories) {
        if (!factory) continue;

        QAction *action = viewManager->actionCollection()->action(factory->id());
        if (!action) continue;

        const QString caption = cleanText(action->text());
        QToolButton *button = KisMobileStyle::createCaptionButton(this, caption, caption, 78);
        if (action->icon().isNull()) {
            button->setIcon(KisIconUtils::loadIcon(factory->iconName()));
        } else {
            button->setIcon(action->icon());
        }

        connect(button, &QAbstractButton::clicked, action, &QAction::trigger);
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

    grid->setRowStretch(row + 1, 1);
}

/******************************************************************/
/* KisMobileMoreSheet                                             */
/******************************************************************/

KisMobileMoreSheet::KisMobileMoreSheet(KisViewManager *viewManager, QWidget *parent)
    : QWidget(parent)
    , m_viewManager(viewManager)
{
    QGridLayout *grid = new QGridLayout(this);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setSpacing(8);

    addActionButton(grid, 0, 0, "file_new", i18n("New"));
    addActionButton(grid, 0, 1, "file_open", i18n("Open"));
    addActionButton(grid, 0, 2, "file_save", i18n("Save"));
    addActionButton(grid, 1, 0, "file_save_as", i18n("Save As"));
    addActionButton(grid, 1, 1, "file_export_file", i18n("Export"));
    addActionButton(grid, 1, 2, "options_configure", i18n("Settings"));

    addCustomButton(grid, 2, 0, "view-fullscreen", i18n("Hide UI"),
                    [this]() { Q_EMIT hideUiRequested(); });
    addCustomButton(grid, 2, 1, "configure", i18n("Desktop UI"),
                    [this]() { Q_EMIT disableMobileUiRequested(); });

    grid->setRowStretch(3, 1);
}

QToolButton *KisMobileMoreSheet::addActionButton(QGridLayout *layout, int row, int column,
                                                 const QString &actionName, const QString &caption)
{
    QAction *action = m_viewManager ? m_viewManager->actionCollection()->action(actionName) : nullptr;
    if (!action && m_viewManager) {
        action = m_viewManager->actionManager()->actionByName(actionName);
    }
    if (!action) return nullptr;

    QToolButton *button = KisMobileStyle::createCaptionButton(this, caption, caption, 86);
    button->setIcon(action->icon());

    connect(button, &QAbstractButton::clicked, action, &QAction::trigger);
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
    QToolButton *button = KisMobileStyle::createCaptionButton(this, caption, caption, 86);
    button->setIcon(KisIconUtils::loadIcon(iconName));

    connect(button, &QAbstractButton::clicked, this, callback);

    layout->addWidget(button, row, column);
}
