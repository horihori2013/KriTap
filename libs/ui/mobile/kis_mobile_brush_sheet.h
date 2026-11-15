/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_BRUSH_SHEET_H
#define KIS_MOBILE_BRUSH_SHEET_H

#include <QPointer>
#include <QWidget>

#include <KoResource.h>

class KisCanvasResourceProvider;
class KisPaintopBox;
class KisPresetChooser;

/**
 * Bottom sheet with the brush preset grid (KisPresetChooser).
 * Selecting a preset applies it through the paintop box.
 */
class KisMobileBrushSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileBrushSheet(QWidget *parent = nullptr);

    void setResourceProvider(KisCanvasResourceProvider *provider);
    void setPaintOpBox(KisPaintopBox *paintOpBox);

public Q_SLOTS:
    void aboutToShow();

Q_SIGNALS:
    /// emitted when a preset was applied and the sheet should close
    void finished();

private Q_SLOTS:
    void slotPresetSelected(KoResourceSP resource);

private:
    KisPresetChooser *m_chooser {nullptr};
    QPointer<KisCanvasResourceProvider> m_provider;
    QPointer<KisPaintopBox> m_paintOpBox;
};

#endif /* KIS_MOBILE_BRUSH_SHEET_H */