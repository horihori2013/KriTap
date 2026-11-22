/*
 *  SPDX-FileCopyrightText: 2025 KriTap contributors
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mobile_layers_sheet.h"

#include "kis_mobile_style.h"
#include "kis_mobile_widgets.h"

#include <QComboBox>
#include <QCursor>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <KoColorSpace.h>
#include <KoID.h>
#include <kis_base_node.h>
#include <kis_icon.h>
#include <klocalizedstring.h>

#include "KisDocument.h"
#include "KisIdleTasksManager.h"
#include "KisSelectionActionsAdapter.h"
#include "kis_selection_manager.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "canvas/kis_canvas2.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "kis_dummies_facade_base.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "kis_node_model.h"
#include "kis_shape_controller.h"
#include "widgets/kis_cmb_composite.h"

namespace
{
    const int layerRowHeight = 58;
    const int thumbnailSize = 46;
    const int propertyIconSize = 28;
    const int propertyIconSpacing = 34;

    QRect propertyRectForRow(const QRect &rowRect, int propertyCount, int index)
    {
        const int x = rowRect.right() - 8 - propertyIconSpacing * (propertyCount - index);
        return QRect(x, rowRect.top() + (rowRect.height() - propertyIconSize) / 2,
                     propertyIconSize, propertyIconSize);
    }

    int propertyIndexAtPos(const QRect &rowRect, const QList<KisBaseNode::Property> &properties, const QPoint &pos)
    {
        for (int i = 0; i < properties.size(); ++i) {
            if (!properties[i].isMutable) continue;
            if (propertyRectForRow(rowRect, properties.size(), i).adjusted(-4, -4, 4, 4).contains(pos)) {
                return i;
            }
        }
        return -1;
    }
}

/******************************************************************/
/* KisMobileLayerDelegate                                         */
/******************************************************************/

class KisMobileLayerDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit KisMobileLayerDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(120, layerRowHeight);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const bool active = index.data(KisNodeModel::ActiveRole).toBool();
        const bool grayed = index.data(KisNodeModel::ShouldGrayOutRole).toBool();
        const QRectF rowRect = QRectF(option.rect).adjusted(2, 2, -2, -2);

        painter->setPen(Qt::NoPen);
        if (active) {
            painter->setBrush(QColor(0, 162, 255, 72));
        } else if (option.state & QStyle::State_MouseOver) {
            painter->setBrush(QColor(255, 255, 255, 18));
        } else {
            painter->setBrush(Qt::NoBrush);
        }
        painter->drawRoundedRect(rowRect, 10, 10);

        // thumbnail
        const QRect thumbRect(option.rect.left() + 8, option.rect.top() + (option.rect.height() - thumbnailSize) / 2,
                              thumbnailSize, thumbnailSize);
        painter->setPen(QPen(KisMobileStyle::separatorColor(), 1.0));
        painter->setBrush(QColor(20, 20, 24, 210));
        painter->drawRoundedRect(QRectF(thumbRect).adjusted(0, 0, -1, -1), 6, 6);

        const QImage thumbnail = index.data(int(KisNodeModel::BeginThumbnailRole) + thumbnailSize + 8).value<QImage>();
        if (!thumbnail.isNull()) {
            const QImage scaled = thumbnail.scaled(thumbRect.size() - QSize(4, 4),
                                                   Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter->drawImage(thumbRect.x() + (thumbRect.width() - scaled.width()) / 2,
                               thumbRect.y() + (thumbRect.height() - scaled.height()) / 2,
                               scaled);
        }

        // name
        const QList<KisBaseNode::Property> properties =
                index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();

        int nameWidth = option.rect.width() - (thumbRect.right() - option.rect.left()) - 16;
        nameWidth -= propertyIconSpacing * properties.size();
        const QRect nameRect(thumbRect.right() + 8, option.rect.top(),
                             qMax(40, nameWidth), option.rect.height());

        QFont nameFont = option.font;
        nameFont.setBold(active);
        nameFont.setPixelSize(13);
        painter->setFont(nameFont);
        painter->setPen(grayed ? KisMobileStyle::dimTextColor() : KisMobileStyle::textColor());

        const QString name = painter->fontMetrics()
                .elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideMiddle, nameRect.width());
        painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, name);

        // mutable property icons (visibility, lock, ...)
        for (int i = 0; i < properties.size(); ++i) {
            const QRect rect = propertyRectForRow(option.rect, properties.size(), i);
            if (!properties[i].isMutable) continue;

            const QIcon icon = properties[i].state.toBool() ? properties[i].onIcon : properties[i].offIcon;
            const QPixmap pixmap = icon.pixmap(propertyIconSize, propertyIconSize);
            if (grayed) {
                painter->setOpacity(0.4);
            }
            painter->drawPixmap(rect.topLeft(), pixmap);
            painter->setOpacity(1.0);
        }

        painter->restore();
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() != QEvent::MouseButtonRelease) {
            return false;
        }

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            return false;
        }

        KisBaseNode::PropertyList properties =
                index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();

        const int propertyIndex = propertyIndexAtPos(option.rect, properties, mouseEvent->pos());
        if (propertyIndex < 0) {
            return false;
        }

        properties[propertyIndex].state = !properties[propertyIndex].state.toBool();
        model->setData(index, QVariant::fromValue(properties), KisNodeModel::PropertiesRole);
        return true;
    }
};

/******************************************************************/
/* KisMobileLayersSheet                                           */
/******************************************************************/

KisMobileLayersSheet::KisMobileLayersSheet(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(6);

    m_opacitySlider = new KisMobileSliderRow(i18n("Opacity"), this);
    m_opacitySlider->setRange(0, 100);

    QWidget *compositeRow = new QWidget(this);
    QHBoxLayout *compositeLayout = new QHBoxLayout(compositeRow);
    compositeLayout->setContentsMargins(8, 0, 8, 0);
    compositeLayout->setSpacing(10);

    QLabel *compositeLabel = new QLabel(i18n("Blending"), compositeRow);
    QFont compositeFont = compositeLabel->font();
    compositeFont.setBold(true);
    compositeFont.setPixelSize(12);
    compositeLabel->setFont(compositeFont);
    compositeLabel->setStyleSheet(QStringLiteral("color: rgba(158, 158, 166, 255);"));

    m_compositeOpCombo = new KisCompositeOpComboBox(compositeRow);

    compositeLayout->addWidget(compositeLabel);
    compositeLayout->addWidget(m_compositeOpCombo, 1);

    QWidget *buttonsRow = new QWidget(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsRow);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(6);

    QToolButton *addButton = addSmallButton(buttonsLayout, "document-new", i18n("Add Layer"));
    QToolButton *duplicateButton = addSmallButton(buttonsLayout, "duplicatelayer", i18n("Duplicate Layer"));
    QToolButton *deleteButton = addSmallButton(buttonsLayout, "deletelayer", i18n("Delete Layer"));
    QToolButton *upButton = addSmallButton(buttonsLayout, "arrowup", i18n("Move Layer Up"));
    QToolButton *downButton = addSmallButton(buttonsLayout, "arrowdown", i18n("Move Layer Down"));
    QToolButton *mergeButton = addSmallButton(buttonsLayout, "merge-layer-below", i18n("Merge with Layer Below"));
    QToolButton *renameButton = addSmallButton(buttonsLayout, "document-edit", i18n("Rename Layer"));

    m_treeView = new QTreeView(this);
    m_treeView->setHeaderHidden(true);
    m_treeView->setRootIsDecorated(true);
    m_treeView->setIndentation(16);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->setUniformRowHeights(true);

    m_delegate = new KisMobileLayerDelegate(m_treeView);
    m_treeView->setItemDelegate(m_delegate);

    m_nodeModel = new KisNodeModel(this, 0);
    m_treeView->setModel(m_nodeModel);

    m_opacityCompressor = new QTimer(this);
    m_opacityCompressor->setSingleShot(true);
    m_opacityCompressor->setInterval(120);

    layout->addWidget(m_opacitySlider);
    layout->addWidget(compositeRow);
    layout->addWidget(buttonsRow);
    layout->addWidget(m_treeView, 1);

    setStyleSheet(KisMobileStyle::rootStyleSheet());

    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &KisMobileLayersSheet::slotCurrentIndexChanged);
    connect(m_opacitySlider, &KisMobileSliderRow::valueChanged,
            this, &KisMobileLayersSheet::slotOpacityChanged);
    connect(m_opacityCompressor, &QTimer::timeout,
            this, &KisMobileLayersSheet::slotApplyOpacity);
    connect(m_compositeOpCombo, QOverload<int>::of(&QComboBox::activated),
            this, &KisMobileLayersSheet::slotCompositeOpChanged);
    connect(m_nodeModel, &QAbstractItemModel::rowsInserted, this, &KisMobileLayersSheet::slotUpdateUi);
    connect(m_nodeModel, &QAbstractItemModel::rowsRemoved, this, &KisMobileLayersSheet::slotUpdateUi);
    connect(m_nodeModel, &QAbstractItemModel::modelReset, this, &KisMobileLayersSheet::slotUpdateUi);

    // the buttons are wired to the actions in setViewManager()
    connect(addButton, &QAbstractButton::clicked, this, [this]() {
        QMenu menu(this);
        const QStringList actionNames {
            "add_new_paint_layer",
            "add_new_group_layer",
            "add_new_fill_layer",
            "add_new_adjustment_layer",
            "add_new_file_layer",
            "add_new_clone_layer",
        };
        for (const QString &name : actionNames) {
            KisAction *action = m_viewManager ? m_viewManager->actionManager()->actionByName(name) : nullptr;
            if (!action) continue;
            menu.addAction(action);
        }
        menu.exec(QCursor::pos());
    });

    connect(duplicateButton, &QAbstractButton::clicked, this, [this]() {
        KisAction *action = m_viewManager ? m_viewManager->actionManager()->actionByName("duplicatelayer") : nullptr;
        if (action) action->trigger();
    });
    connect(deleteButton, &QAbstractButton::clicked, this, [this]() {
        KisAction *action = m_viewManager ? m_viewManager->actionManager()->actionByName("remove_layer") : nullptr;
        if (action) action->trigger();
    });
    connect(upButton, &QAbstractButton::clicked, this, [this]() {
        KisAction *action = m_viewManager ? m_viewManager->actionManager()->actionByName("move_layer_up") : nullptr;
        if (action) action->trigger();
    });
    connect(downButton, &QAbstractButton::clicked, this, [this]() {
        KisAction *action = m_viewManager ? m_viewManager->actionManager()->actionByName("move_layer_down") : nullptr;
        if (action) action->trigger();
    });
    connect(mergeButton, &QAbstractButton::clicked, this, [this]() {
        KisAction *action = m_viewManager ? m_viewManager->actionManager()->actionByName("merge_layer") : nullptr;
        if (action) action->trigger();
    });
    connect(renameButton, &QAbstractButton::clicked, this, &KisMobileLayersSheet::slotRenameLayer);
}

KisMobileLayersSheet::~KisMobileLayersSheet()
{
    if (m_canvas) {
        m_nodeModel->setIdleTaskManager(nullptr);
        m_nodeModel->setDummiesFacade(nullptr, nullptr, nullptr, nullptr, nullptr);
    }
}

QToolButton *KisMobileLayersSheet::addSmallButton(QLayout *layout, const QString &iconName, const QString &toolTip)
{
    QToolButton *button = KisMobileStyle::createIconButton(this, toolTip,
                                                           KisMobileStyle::smallButtonSize,
                                                           KisMobileStyle::smallIconSize);
    button->setIcon(KisIconUtils::loadIcon(iconName));
    layout->addWidget(button);
    return button;
}

void KisMobileLayersSheet::setViewManager(KisViewManager *viewManager)
{
    m_viewManager = viewManager;
    m_nodeManager = viewManager ? viewManager->nodeManager() : nullptr;
}

void KisMobileLayersSheet::setCanvas(KisCanvas2 *canvas)
{
    if (m_canvas == canvas) return;

    if (m_canvas) {
        m_nodeModel->setIdleTaskManager(nullptr);
        m_nodeModel->setDummiesFacade(nullptr, nullptr, nullptr, nullptr, nullptr);
        m_selectionAdapter.reset();

        if (m_nodeManager) {
            disconnect(m_nodeManager, nullptr, this, nullptr);
            disconnect(m_nodeModel, nullptr, m_nodeManager, nullptr);
            m_nodeManager->slotSetSelectedNodes(KisNodeList());
        }
    }

    m_canvas = canvas;
    m_image = nullptr;

    if (!m_canvas) {
        setEnabled(false);
        return;
    }

    m_image = m_canvas->image();

    KisDocument *document = static_cast<KisDocument*>(m_canvas->imageView()->document());
    KisShapeController *shapeController =
            dynamic_cast<KisShapeController*>(document->shapeController());
    KisDummiesFacadeBase *dummiesFacade =
            static_cast<KisDummiesFacadeBase*>(shapeController);

    m_selectionAdapter.reset(new KisSelectionActionsAdapter(m_canvas->viewManager()->selectionManager()));
    m_nodeModel->setDummiesFacade(dummiesFacade,
                                  m_image,
                                  shapeController,
                                  m_selectionAdapter.get(),
                                  m_nodeManager);
    m_nodeModel->setIdleTaskManager(m_canvas->viewManager()->idleTasksManager());

    if (m_nodeManager) {
        connect(m_nodeManager, &KisNodeManager::sigUiNeedChangeActiveNode,
                this, &KisMobileLayersSheet::slotNodeActivated);
        connect(m_nodeModel, &KisNodeModel::toggleIsolateActiveNode,
                m_nodeManager, &KisNodeManager::toggleIsolateActiveNode);
        slotNodeActivated(m_nodeManager->activeNode());
    }

    setEnabled(true);
    slotUpdateUi();
}

void KisMobileLayersSheet::aboutToShow()
{
    if (m_canvas && m_canvas->viewManager()) {
        m_nodeModel->setIdleTaskManager(m_canvas->viewManager()->idleTasksManager());
    }
    updateFromActiveNode();
}

void KisMobileLayersSheet::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_nodeModel->setIdleTaskManager(nullptr);
}

void KisMobileLayersSheet::slotCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!current.isValid()) return;
    m_nodeModel->setData(current, true, KisNodeModel::ActiveRole);
}

void KisMobileLayersSheet::slotNodeActivated(KisNodeSP node)
{
    if (node && m_nodeModel->hasDummiesFacade()) {
        const QModelIndex index = m_nodeModel->indexFromNode(node);
        if (index.isValid() && m_treeView->currentIndex() != index) {
            const QSignalBlocker blocker(m_treeView->selectionModel());
            m_treeView->setCurrentIndex(index);
        }
    }
    updateFromActiveNode();
}

void KisMobileLayersSheet::updateFromActiveNode()
{
    KisNodeSP node;
    if (m_nodeManager) {
        node = m_nodeManager->activeNode();
    }

    if (!node) {
        m_opacitySlider->setEnabled(false);
        m_compositeOpCombo->setEnabled(false);
        return;
    }

    m_blockUiUpdates = true;

    m_opacitySlider->setEnabled(true);
    m_opacitySlider->setValue(node->opacity() * 100.0 / 255.0);

    const KoColorSpace *colorSpace = m_nodeManager->activeColorSpace();
    if (colorSpace) {
        m_compositeOpCombo->setEnabled(true);
        m_compositeOpCombo->validate(colorSpace);
        m_compositeOpCombo->selectCompositeOp(KoID(node->compositeOpId(), node->compositeOpId()));
    }

    m_blockUiUpdates = false;
}

void KisMobileLayersSheet::slotOpacityChanged(qreal value)
{
    if (m_blockUiUpdates) return;

    m_newOpacity = value;
    m_changedOpacityNode.clear();
    if (m_nodeManager) {
        m_changedOpacityNode = m_nodeManager->activeNode();
    }
    if (m_changedOpacityNode) {
        m_opacityCompressor->start();
    }
}

void KisMobileLayersSheet::slotApplyOpacity()
{
    if (!m_nodeManager || !m_changedOpacityNode) return;
    m_nodeManager->setNodeOpacity(m_changedOpacityNode, qRound(m_newOpacity * 255.0 / 100.0));
}

void KisMobileLayersSheet::slotCompositeOpChanged()
{
    if (m_blockUiUpdates || !m_nodeManager) return;

    const KoColorSpace *colorSpace = m_nodeManager->activeColorSpace();
    if (!colorSpace) return;

    m_nodeManager->nodeCompositeOpChanged(
            colorSpace->compositeOp(m_compositeOpCombo->selectedCompositeOp().id()));
}

void KisMobileLayersSheet::slotRenameLayer()
{
    KisNodeSP node;
    if (m_nodeManager) {
        node = m_nodeManager->activeNode();
    }
    if (!node) return;

    bool ok = false;
    const QString name = QInputDialog::getText(this,
                                               i18n("Rename Layer"),
                                               i18n("Layer name:"),
                                               QLineEdit::Normal,
                                               node->name(),
                                               &ok);
    if (!ok || name.isEmpty() || name == node->name()) return;

    if (!m_nodeModel->hasDummiesFacade()) return;

    const QModelIndex index = m_nodeModel->indexFromNode(node);
    if (index.isValid()) {
        m_nodeModel->setData(index, name, Qt::EditRole);
    }
}

void KisMobileLayersSheet::slotUpdateUi()
{
    const bool hasNode = m_nodeManager && m_nodeManager->activeNode();
    m_opacitySlider->setEnabled(hasNode);
    m_compositeOpCombo->setEnabled(hasNode);
}

#include "kis_mobile_layers_sheet.moc"