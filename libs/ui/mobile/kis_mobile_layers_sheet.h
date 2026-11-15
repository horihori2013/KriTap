/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MOBILE_LAYERS_SHEET_H
#define KIS_MOBILE_LAYERS_SHEET_H

#include <QPointer>
#include <QWidget>

#include <kis_types.h>
#include <memory>

class KisCanvas2;
class KisCompositeOpComboBox;
class KisMobileSliderRow;
class KisNodeModel;
class KisNodeManager;
class KisSelectionActionsAdapter;
class KisViewManager;
class QModelIndex;
class QLayout;
class QToolButton;
class QTreeView;
class QTimer;
class KisMobileLayerDelegate;

/**
 * Bottom sheet with the layer stack: a thumbnail list, the opacity
 * slider and the blending mode combo of the active layer, plus row of
 * layer management buttons (add, duplicate, delete, move, merge, rename).
 */
class KisMobileLayersSheet : public QWidget
{
    Q_OBJECT
public:
    explicit KisMobileLayersSheet(QWidget *parent = nullptr);
    ~KisMobileLayersSheet() override;

    void setViewManager(KisViewManager *viewManager);
    void setCanvas(KisCanvas2 *canvas);

public Q_SLOTS:
    void aboutToShow();

protected:
    void hideEvent(QHideEvent *event) override;

private Q_SLOTS:
    void slotCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotNodeActivated(KisNodeSP node);
    void slotOpacityChanged(qreal value);
    void slotApplyOpacity();
    void slotCompositeOpChanged();
    void slotRenameLayer();
    void slotUpdateUi();

private:
    void updateFromActiveNode();
    QToolButton *addSmallButton(QLayout *layout, const QString &iconName, const QString &toolTip);

private:
    QTreeView *m_treeView {nullptr};
    KisMobileLayerDelegate *m_delegate {nullptr};
    KisNodeModel *m_nodeModel {nullptr};
    KisMobileSliderRow *m_opacitySlider {nullptr};
    KisCompositeOpComboBox *m_compositeOpCombo {nullptr};

    QPointer<KisViewManager> m_viewManager;
    KisNodeManager *m_nodeManager {nullptr};
    KisCanvas2 *m_canvas {nullptr};
    KisImageWSP m_image;
    std::unique_ptr<KisSelectionActionsAdapter> m_selectionAdapter;

    QTimer *m_opacityCompressor {nullptr};
    KisNodeSP m_changedOpacityNode;
    qreal m_newOpacity {100.0};
    bool m_blockUiUpdates {false};
};

#endif /* KIS_MOBILE_LAYERS_SHEET_H */