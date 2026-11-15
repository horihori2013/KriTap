/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_brush_sheet.h"

#include "kis_mobile_style.h"

#include <QVBoxLayout>

#include <kis_paintop_preset.h>

#include "kis_canvas_resource_provider.h"
#include "kis_paintop_box.h"
#include "widgets/kis_preset_chooser.h"

KisMobileBrushSheet::KisMobileBrushSheet(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(8);

    m_chooser = new KisPresetChooser(this);
    m_chooser->showTaggingBar(false);
    m_chooser->setViewModeToThumbnail();

    layout->addWidget(m_chooser, 1);

    setStyleSheet(KisMobileStyle::rootStyleSheet());

    connect(m_chooser, &KisPresetChooser::resourceSelected,
            this, &KisMobileBrushSheet::slotPresetSelected);
}

void KisMobileBrushSheet::setResourceProvider(KisCanvasResourceProvider *provider)
{
    m_provider = provider;
}

void KisMobileBrushSheet::setPaintOpBox(KisPaintopBox *paintOpBox)
{
    m_paintOpBox = paintOpBox;
}

void KisMobileBrushSheet::aboutToShow()
{
    if (!m_provider) return;

    const KisPaintOpPresetSP preset = m_provider->currentPreset();
    if (preset) {
        m_chooser->setCurrentResource(preset);
    }
}

void KisMobileBrushSheet::slotPresetSelected(KoResourceSP resource)
{
    if (!resource) return;

    if (m_paintOpBox) {
        m_paintOpBox->resourceSelected(resource);
    } else if (m_provider) {
        KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();
        if (preset) {
            m_provider->setPaintOpPreset(preset);
        }
    }

    Q_EMIT finished();
}