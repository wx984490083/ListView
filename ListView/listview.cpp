#include "listview_p.h"
#include "listviewitem_p.h"
#include "listdatamodel_p.h"
#include "listview_p.h"
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollBar>

QString ListViewScrollBarStyle = QStringLiteral(
            R"(
            QScrollBar:vertical {
                border: none;
                border-radius: 4px;
                background: "#10000000";
                max-width: 8px;
                margin: 0px 0px 0px 0px;
            }

            QScrollBar::handle:vertical {
                background: #808080;
                border-radius: 4px;
                min-height: 32px;
            }

            QScrollBar::add-line,QScrollBar::sub-line
            {
                height: 0;
            }

            QScrollBar::add-page,QScrollBar::sub-page
            {
                background: none;
            }
            )");

class OrderedListHelper
{
public:
    template<class T>
    static bool contains(const std::list<T>& list, const T& val)
    {
        auto it = std::lower_bound(list.begin(), list.end(), val);
        return (it != list.end() && *it == val);
    }

    template<class T>
    static typename std::list<T>::iterator find(std::list<T>& list, const T& val)
    {
        auto it = std::lower_bound(list.begin(), list.end(), val);
        if (it != list.end() && *it != val)
        {
            return list.end();
        }
        return it;
    }

    template<class T>
    static typename std::list<T>::const_iterator find(const std::list<T>& list, const T& val)
    {
        auto it = std::lower_bound(list.begin(), list.end(), val);
        if (it != list.end() && *it != val)
        {
            return list.end();
        }
        return it;
    }
};

ListView::ListView(QWidget *parent) : QWidget(parent), priv(new ListViewPriv)
{
    priv->owner = this;
    priv->setup();
}

ListView::~ListView()
{
    priv->cleanup();
    delete priv;
}

void ListView::setDataModel(ListDataModel *model)
{
    priv->setDataModel(model);
}

void ListView::setViewDelegate(ListViewDelegate *delegate)
{
    priv->setViewDelegate(delegate);
}

ListDataModel *ListView::dataModel() const
{
    return priv->dataModel();
}

ListViewDelegate *ListView::viewDelegate() const
{
    return priv->viewDelegate();
}

std::list<ListIndex> ListView::selection()
{
    return priv->selection();
}

void ListView::setSelection(std::list<ListIndex> &&selection)
{
    auto tmp = std::move(selection);
    priv->setSelection(selection);
}

void ListView::setSelection(const std::list<ListIndex> &selection)
{
    priv->setSelection(selection);
}

void ListView::scrollToItem(const ListIndex &index)
{
    priv->scrollToItem(index);
}

void ListView::scrollToTop()
{
    priv->scrollToTop();
}

void ListView::scrollToBottom()
{
    priv->scrollToBottom();
}

ListViewPriv *ListView::getPriv() const
{
    return priv;
}

void ListView::resizeEvent(QResizeEvent *event)
{
    priv->onResized(event->oldSize());
}



void ListViewPriv::setup()
{
    scrollArea = new SmoothScrollArea(owner);
    scrollArea->setAutoFillBackground(false);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollContent = new QWidget(scrollArea);
    scrollContent->setAutoFillBackground(false);
    scrollArea->setWidget(scrollContent);

    // TODO: 有时间可以研究一下
    // 假如这里直接 scrollArea->setStyleSheet ，如果父窗口有样式，会导致此处滚动条样式失效。
    // 但直接对 verticalScrollBar 设置就不会被影响。
    scrollArea->verticalScrollBar()-> setStyleSheet(ListViewScrollBarStyle);

    scrollArea->setAutoFillBackground(false);
    scrollContent->setAutoFillBackground(false);
    scrollArea->viewport()->setAutoFillBackground(false);

    QObject::connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, owner, [=]{adjustLoadedItems();});
}

void ListViewPriv::cleanup()
{
    if (currentModel)
    {
        currentModel->listViewPrivs.erase(this);
        currentModel = nullptr;
    }
}

void ListViewPriv::processItemClick(QMouseEvent *event, const ListIndex &index, ListViewItemPriv *item)
{
    if (event->button() == Qt::LeftButton)
    {
        if (currentDelegate->canSelectItem(index))
        {
            processItemSelection(index);
        }
        emit owner->itemLeftClicked(index, item->owner, event);
    }
    else if (event->button() == Qt::RightButton)
    {
        emit owner->itemRightClicked(index, item->owner, event);
    }
}

void ListViewPriv::setDataModel(ListDataModel *model)
{
    auto modelp = model ? model->getPriv() : nullptr;

    if (modelp == currentModel)
    {
        return;
    }

    clear();

    if (currentModel)
    {
        currentModel->listViewPrivs.erase(this);
    }

    currentModel = modelp;

    if (currentModel)
    {
        currentModel->listViewPrivs.insert(this);
    }

    reload();
}

void ListViewPriv::setViewDelegate(ListViewDelegate *delegate)
{
    clear();

    currentDelegate = delegate;

    reload();
}

ListDataModel *ListViewPriv::dataModel() const
{
    return currentModel->owner;
}

ListViewDelegate *ListViewPriv::viewDelegate() const
{
    return currentDelegate;
}

std::list<ListIndex> ListViewPriv::selection()
{
    return selected;
}

void ListViewPriv::setSelection(const std::list<ListIndex> &selection)
{
    if (selected == selection)
    {
        return;
    }

    // 修改所有已加载项的视图状态
    // 不要通过遍历 selected 来实现，如果之前是全选，并且数据量很大的话，会浪费 cpu 资源，可能还会卡！
    // 遍历 loadedItems 在一般情况下会浪费一丢丢 cpu （遍历到一些不需要改变的状态的 item），但是可以防止极端情况~
    for (auto& item : loadedItems)
    {
        if (item.view)
        {
            auto newSelectedState = OrderedListHelper::contains(selection, item.index);
            if (item.view->selected != newSelectedState)
            {
                item.view->selected = newSelectedState;
                item.view->owner->update();
            }
        }
    }
    selected = selection;
    emit owner->selectionChanged();
}

void ListViewPriv::scrollToItem(const ListIndex &index)
{
    if (!currentModel || currentModel->owner->isEmpty())
    {
        return;
    }

    if (index.group < 0 || index.group >= (int)itemHeights.size()
            || index.item < -1 || index.item >= (int)itemHeights[index.group].size())
    {
        // invalid index
        return;
    }

    int targetY;
    if (loadedItems.empty())
    {
        targetY = itemPosition(index, ListIndex(), 0);
    }
    else
    {
        const auto& item = loadedItems.front();
        const auto refIndex = item.index;
        int refY = item.y;
        targetY = itemPosition(index, refIndex, refY);
    }

    scrollArea->verticalScrollBar()->setValue(targetY);
}

void ListViewPriv::scrollToTop()
{
    scrollArea->verticalScrollBar()->setValue(0);
}

void ListViewPriv::scrollToBottom()
{
    auto vs = scrollArea->verticalScrollBar();
    vs->setValue(vs->maximum());
}

void ListViewPriv::requireReload()
{
    clear();
    reload();
}

void ListViewPriv::itemUpdated(const ListIndex &index)
{
    if (!currentDelegate)
    {
        return;
    }
    auto itemNewHeight = currentDelegate->heightForIndex(index, owner->width());
    auto itemOldHeight = itemHeights[index.group][index.item];
    auto dh = itemNewHeight - itemOldHeight;
    contentHeight += dh;
    itemHeights[index.group][index.item] = itemNewHeight;
    if (!loadedItems.empty())
    {
        auto it = std::lower_bound(loadedItems.begin(), loadedItems.end(), index, [](const LoadedItem& item, const ListIndex& idx)
        {
            return item.index < idx;
        });

        // adjust loadedItems's size and position
        if (it != loadedItems.end() && it->index == index)
        {
            it->view->owner->resize(owner->width(), itemNewHeight);
            it->h = itemNewHeight;

            while (++it != loadedItems.end())
            {
                auto& item = *it;
                item.y += dh;
                QWidget* view = (item.index.item == ListIndex::InvalidItemIndex)
                        ? headerViews[item.index.group]
                        : item.view->owner;
                if (view)
                {
                    view->move(0, item.y);
                }
            }
        }

        // 调整前断掉信号连接~
        auto vs = scrollArea->verticalScrollBar();
        vs->disconnect(owner);

        // 调整scrollContent 高度
        fixContentSize(false);

        // 如果变动的 item 是最后一个，就直接滚动到最底部
        if (currentModel->owner->maxIndex() == index)
        {
            vs->setValue(vs->maximum());
        }
        // 如果变动的 item 在 loadedItems 前面，那么可以滚动视图保持视觉不变
        else if (index < loadedItems.begin()->index)
        {
            vs->setValue(vs->value() + dh);
        }

        QObject::connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, owner, [=]{adjustLoadedItems();});
    }
    adjustLoadedItems();
}

void ListViewPriv::beginInsertItem(const ListIndex &insertIndex, size_t count)
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeNone);
    modifyInfo.mode = ModifyModeInsertItem;
    modifyInfo.index = insertIndex;
    modifyInfo.count = count;
}

void ListViewPriv::endInsertItem()
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeInsertItem);
    const auto width = owner->width();
    auto& groupItemHeights = itemHeights[modifyInfo.index.group];
    groupItemHeights.insert(groupItemHeights.begin() + modifyInfo.index.item, modifyInfo.count, 0);
    int insertedTotalHeight = 0;
    for (int item = modifyInfo.index.item; item < modifyInfo.index.item + modifyInfo.count; item++)
    {
        auto& itemHeight = groupItemHeights.at(item);
        itemHeight = currentDelegate->heightForIndex(ListIndex(modifyInfo.index.group, item), width);
        insertedTotalHeight += itemHeight;
    }
    contentHeight += insertedTotalHeight;

    while (!loadedItems.empty() && loadedItems.back().index >= modifyInfo.index)
    {
        auto& item = loadedItems.back();
        ListIndex newIndex = (item.index.group == modifyInfo.index.group)
                ? ListIndex(item.index.group, item.index.item + modifyInfo.count)
                : item.index;
        adjustItem(item, newIndex, item.y + insertedTotalHeight);
        pendingItems[newIndex] = item;
        loadedItems.pop_back();
    }

    // 重建选中列表，调整其中大于等于 modifyInfo.index 的索引号。
    auto selectedIt = std::lower_bound(selected.begin(), selected.end(), modifyInfo.index);
    while (selectedIt != selected.end() && selectedIt->group == modifyInfo.index.group)
    {
        selectedIt->item += modifyInfo.count;
        selectedIt++;
    }

    fixContentSize(false);
    adjustLoadedItems();
    modifyInfo.mode = ModifyModeNone;
    emit owner->itemsInserted(modifyInfo.index, modifyInfo.count);
}

void ListViewPriv::beginInsertGroup(int groupIndex)
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeNone);
    modifyInfo.mode = ModifyModeInsertGroup;
    modifyInfo.index = ListIndex(groupIndex);
    modifyInfo.count = 1;
}

void ListViewPriv::endInsertGroup()
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeInsertGroup);
    const auto width = owner->width();

    auto headerView = currentDelegate->headerViewForGroup(modifyInfo.index.group);
    if (headerView)
    {
        headerView->setParent(scrollContent);
    }
    headerViews.insert(headerViews.begin() + modifyInfo.index.group, headerView);

    auto& groupItemHeights = *(itemHeights.insert(itemHeights.begin() + modifyInfo.index.group, modifyInfo.count, {}));
    auto numItems = currentModel->owner->numItemsInGroup(modifyInfo.index.group);
    groupItemHeights.resize(numItems);
    int insertedTotalHeight = 0;
    for (int item = 0; item < numItems; item++)
    {
        auto& itemHeight = groupItemHeights[item];
        itemHeight = currentDelegate->heightForIndex(ListIndex(modifyInfo.index.group, item), width);
        insertedTotalHeight += itemHeight;
    }
    insertedTotalHeight += (headerView ? headerView->height() : 0);
    contentHeight += insertedTotalHeight;

    while (!loadedItems.empty() && loadedItems.back().index >= modifyInfo.index)
    {
        auto& item = loadedItems.back();
        ListIndex newIndex = ListIndex(item.index.group + 1, item.index.item);
        adjustItem(item, newIndex, item.y + insertedTotalHeight);
        pendingItems[newIndex] = item;
        loadedItems.pop_back();
    }

    // 重建选中列表，调整其中大于等于 modifyInfo.index 的索引号。
    auto selectedIt = std::lower_bound(selected.begin(), selected.end(), modifyInfo.index);
    while (selectedIt != selected.end())
    {
        selectedIt->group += 1;
        selectedIt++;
    }

    fixContentSize(false);
    adjustLoadedItems();
    modifyInfo.mode = ModifyModeNone;
    emit owner->groupInserted(modifyInfo.index.group);
}

void ListViewPriv::beginRemoveItem(const ListIndex &removeIndex, size_t count)
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeNone);
    modifyInfo.mode = ModifyModeRemoveItem;
    modifyInfo.index = removeIndex;
    modifyInfo.count = count;
}

void ListViewPriv::endRemoveItem()
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeRemoveItem);
    auto& groupItemHeights = itemHeights[modifyInfo.index.group];
    int deletedTotalHeight = 0;
    for (int item = modifyInfo.index.item; item < modifyInfo.index.item + modifyInfo.count; item++)
    {
        const auto& itemHeight = groupItemHeights.at(item);
        deletedTotalHeight += itemHeight;
    }
    contentHeight -= deletedTotalHeight;
    groupItemHeights.erase(groupItemHeights.begin() + modifyInfo.index.item, groupItemHeights.begin() + modifyInfo.index.item + modifyInfo.count);

    while (!loadedItems.empty() && loadedItems.back().index >= modifyInfo.index)
    {
        auto& item = loadedItems.back();
        if (item.index.group == modifyInfo.index.group && item.index.item < modifyInfo.index.item + modifyInfo.count)
        {
            // This item has been removed, now recycle the view.
            item.view->owner->hide();
            reusePool[item.view->owner->metaObject()].push_back(item.view);
        }
        else
        {
            ListIndex newIndex = (item.index.group == modifyInfo.index.group)
                    ? ListIndex(item.index.group, item.index.item - modifyInfo.count)
                    : item.index;
            adjustItem(item, newIndex, item.y - deletedTotalHeight);
            pendingItems[newIndex] = item;
        }
        loadedItems.pop_back();
    }

    // 重建选中列表，删除对应的索引，并调整其中大于等于 modifyInfo.index 的索引号。
    auto selectedIt = std::lower_bound(selected.begin(), selected.end(), modifyInfo.index);
    while (selectedIt != selected.end() && selectedIt->group == modifyInfo.index.group)
    {
        if (selectedIt->item < modifyInfo.index.item + modifyInfo.count)
        {
            selectedIt = selected.erase(selectedIt);
        }
        else
        {
            selectedIt->group -= modifyInfo.count;
            selectedIt++;
        }
    }

    fixContentSize(false);
    adjustLoadedItems();
    modifyInfo.mode = ModifyModeNone;
    emit owner->itemsRemoved(modifyInfo.index, modifyInfo.count);
}

void ListViewPriv::beginRemoveGroup(int groupIndex)
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeNone);
    modifyInfo.mode = ModifyModeRemoveGroup;
    modifyInfo.index = ListIndex(groupIndex);
    modifyInfo.count = 1;
}

void ListViewPriv::endRemoveGroup()
{
    if (!currentDelegate)
    {
        return;
    }
    Q_ASSERT(modifyInfo.mode == ModifyModeRemoveGroup);
    auto& groupItemHeights = itemHeights[modifyInfo.index.group];
    int deletedTotalHeight = 0;
    for (auto& height : groupItemHeights)
    {
        deletedTotalHeight += height;
    }
    if (auto& view = headerViews[modifyInfo.index.group])
    {
        deletedTotalHeight += view->height();
        view->height();
        delete view;
    }
    contentHeight -= deletedTotalHeight;
    itemHeights.erase(itemHeights.begin() + modifyInfo.index.group);
    headerViews.erase(headerViews.begin() + modifyInfo.index.group);

    while (!loadedItems.empty() && loadedItems.back().index >= modifyInfo.index)
    {
        auto& item = loadedItems.back();
        if (item.index.group == modifyInfo.index.group)
        {
            // This item has been removed, recycle the view.
            // Ignore the header view
            if (item.index.item != ListIndex::InvalidItemIndex)
            {
                item.view->owner->hide();
                reusePool[item.view->owner->metaObject()].push_back(item.view);
            }
        }
        else
        {
            ListIndex newIndex = ListIndex(item.index.group - 1, item.index.item);
            adjustItem(item, newIndex, item.y - deletedTotalHeight);
            pendingItems[newIndex] = item;
        }
        loadedItems.pop_back();
    }

    // 重建选中列表，删除对应的索引，调整其中大于等于 modifyInfo.index 的索引号。
    auto selectedIt = std::lower_bound(selected.begin(), selected.end(), modifyInfo.index);
    while (selectedIt != selected.end())
    {
        if (selectedIt->group == modifyInfo.index.group)
        {
            selectedIt = selected.erase(selectedIt);
        }
        else
        {
            selectedIt->group -= 1;
            selectedIt++;
        }
    }

    fixContentSize(false);
    adjustLoadedItems();
    modifyInfo.mode = ModifyModeNone;
    emit owner->groupRemoved(modifyInfo.index.group);
}

void ListViewPriv::onResized(const QSize& oldSize)
{
    scrollArea->verticalScrollBar()->disconnect(owner);
    scrollArea->setGeometry(owner->rect());

    if (emptyView)
    {
        emptyView->setGeometry(owner->rect());
    }

    fixContentSize(oldSize.width() != owner->width());

    adjustLoadedItems();
    QObject::connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, owner, [=]{adjustLoadedItems();});
}

void ListViewPriv::clear()
{
    if (emptyView)
    {
        delete emptyView;
        emptyView = nullptr;
    }

    for (auto& headerView : headerViews)
    {
        if (headerView)
        {
            delete headerView;
        }
    }
    headerViews.clear();

    for (auto& item : loadedItems)
    {
        if (item.index.item != ListIndex::InvalidItemIndex)
        {
            // put item views to reuse pool
            item.view->owner->hide();
            reusePool[item.view->owner->metaObject()].push_back(item.view);
        }
    }
    loadedItems.clear();
    selected.clear();
    groupHeights.clear();
    itemHeights.clear();
    contentHeight = 0;

    scrollArea->verticalScrollBar()->disconnect(owner);
    scrollContent->resize(owner->width(), owner->height());
    QObject::connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, owner, [=]{adjustLoadedItems();});
}

void ListViewPriv::reload()
{
    if (!currentModel || !currentDelegate)
    {
        return;
    }

    cacheHeaders();
    cacheHeightsAndAnchorPos();
    fixContentSize(false);
    adjustLoadedItems();
}

void ListViewPriv::cacheHeaders()
{
    const auto nGroups = currentModel->owner->numGroups();
    headerViews.reserve(nGroups);
    for (auto group = 0; group < nGroups; group++)
    {
        auto view = currentDelegate->headerViewForGroup(group);
        if (view)
        {
            view->setParent(scrollContent);
        }
        headerViews.push_back(view);
    }
}

int ListViewPriv::cacheHeightsAndAnchorPos(const ListIndex& anchorIndex)
{
    int anchorY = 0;
    const auto width = owner->width();
    const auto nGroups = currentModel->owner->numGroups();
    groupHeights.resize(nGroups);
    itemHeights.resize(nGroups);
    contentHeight = 0;
    for (auto group = 0; group < nGroups; group++)
    {
        auto& groupHeight = groupHeights[group];
        groupHeight = 0;
        if (anchorIndex == ListIndex(group))
        {
            anchorY = contentHeight;
        }
        auto headerView = headerViews[group];
        auto headerHeight = headerView ? headerView->height() : 0;
        groupHeight += headerHeight;

        const auto nItems = currentModel->owner->numItemsInGroup(group);
        auto& groupItemHeights = itemHeights[group];
        groupItemHeights.resize(nItems);
        for (auto item = 0; item < nItems; item++)
        {
            if (anchorIndex == ListIndex(group, item))
            {
                anchorY = contentHeight + groupHeight;
            }
            auto itemHeight = currentDelegate->heightForIndex(ListIndex(group, item), width);
            groupItemHeights[item] = itemHeight;
            groupHeight += itemHeight;
        }

        // TODO: 这里可能需要做加法溢出判断，如果有溢出，则修改加载逻辑，不加载任何东西~
        contentHeight += groupHeight;
    }
    return anchorY;
}

void ListViewPriv::setupEmptyView()
{
    if (!emptyView)
    {
        emptyView = currentDelegate->emptyView();
        if (emptyView)
        {
            emptyView->setParent(owner);
        }
    }
    if (emptyView)
    {
        emptyView->raise();
        emptyView->setGeometry(owner->rect());
        emptyView->show();
    }
}

void ListViewPriv::clearEmptyView()
{
    if (emptyView)
    {
        emptyView->hide();
        emptyView->deleteLater();
        emptyView = nullptr;
    }
}

void ListViewPriv::adjustLoadedItems()
{
    if (!currentDelegate)
    {
        return;
    }

    if (modelNotEmpty())
    {
        clearEmptyView();
        unloadOutOfViewportItems();
        loadUnderItems();
        loadAboveItems();
        recyclePreloadedItems();
    }
    else
    {
        setupEmptyView();
    }
}

void ListViewPriv::fixContentSize(bool widthChanged)
{
    auto width = owner->width();

    if (!widthChanged)
    {
        scrollContent->resize(width, contentHeight);
        return;
    }

    if (!currentModel || !currentDelegate)
    {
        return;
    }

    if (currentDelegate->canItemHeightAffectedByWidth())
    {
        // 在 contentHeight 计算完成并调整 scrollContent 高度之后，需要将所有已加载的数据项视图调整到正确的Y轴位置
        // 在此之前记录锚点视图，并记录其到视口顶点的距离
        // 然后滚动视图，可以使锚点视图在视口中保持其原来的位置。
        // 选取锚点的方法是：未滚动到底部时以第一个 item 的顶部作为锚点，已滚动到底部时以最后一个 item 的底部作为锚点
        ListIndex anchorIndex;
        int anchorDistance;

        auto vs = scrollArea->verticalScrollBar();
        auto viewportTop = vs->value();
        auto anchorIsTop = viewportTop != vs->maximum();

        if (!loadedItems.empty())
        {
            if (anchorIsTop)
            {
                anchorIndex = loadedItems.front().index;
                anchorDistance = loadedItems.front().y - viewportTop;
            }
            else
            {
                anchorIndex = loadedItems.back().index;
                // 此处可确定已滚动到底部，最后一个 item 底部到 viewport 底部距离为 0
                anchorDistance = 0;
            }
        }

        const auto newAnchorY = cacheHeightsAndAnchorPos(anchorIndex);
        scrollContent->resize(width, contentHeight);

        if (contentHeight != scrollContent->height())
        {
            // The contentHeight is too large... ( > QWIDGETSIZE_MAX)
        }

        if (loadedItems.empty())
        {
            return;
        }

        // 调整 loadedItems 的位置
        if (anchorIsTop)
        {
            // 以第一个 item 的顶部作为锚点，向下依次调整
            auto currentY = newAnchorY;
            for (auto it = loadedItems.begin(); it != loadedItems.end(); it++)
            {
                auto& item = *it;
                item.h = item.index.item == ListIndex::InvalidItemIndex
                        ? headerHeight(item.index.group)
                        : itemHeights[item.index.group][item.index.item];
                item.y = currentY;

                QWidget* view = (item.index.item == ListIndex::InvalidItemIndex)
                        ? headerViews[item.index.group]
                        : item.view->owner;

                if (view)
                {
                    view->setGeometry(0, item.y, width, item.h);
                }

                currentY = item.y + item.h;
            }
            vs->setValue(newAnchorY - anchorDistance);
        }
        else
        {
            // 以最后一个 item 的底部作为锚点，向上依次调整
            auto currentY = contentHeight;
            for (auto it = loadedItems.rbegin(); it != loadedItems.rend(); it++)
            {
                auto& item = *it;
                item.h = item.index.item == ListIndex::InvalidItemIndex
                        ? headerHeight(item.index.group)
                        : itemHeights[item.index.group][item.index.item];
                item.y = currentY - item.h;

                QWidget* view = (item.index.item == ListIndex::InvalidItemIndex)
                        ? headerViews[item.index.group]
                        : item.view->owner;

                if (view)
                {
                    view->setGeometry(0, item.y, width, item.h);
                }

                currentY = item.y;
            }
            auto maxScrollValue = vs->maximum();
            vs->setValue(maxScrollValue);
        }
    }
    else
    {
        // change the loaded item's width
        for (auto& item : loadedItems)
        {
            QWidget* view = (item.index.item == ListIndex::InvalidItemIndex)
                    ? headerViews[item.index.group]
                    : item.view->owner;

            if (view)
            {
                view->setGeometry(0, item.y, width, item.h);
            }
        }

        scrollContent->resize(width, contentHeight);
    }
}

void ListViewPriv::loadAboveItems()
{
    const auto viewportTop = scrollArea->verticalScrollBar()->value();
    const auto viewportBottom = viewportTop + scrollArea->height();
    const auto width = owner->width();

    auto nextIndex = loadedItems.empty() ? currentModel->owner->maxIndex() : decreaseIndex(loadedItems.front().index);

    if (nextIndex.isEmpty())
    {
        return;
    }

    int nextHeight = nextIndex.item == ListIndex::InvalidItemIndex
            ? headerHeight(nextIndex.group)
            : itemHeights[nextIndex.group][nextIndex.item];

    auto nextY = loadedItems.empty()
            ? viewportBottom - nextHeight
            : loadedItems.front().y - nextHeight;

    // Load items to fill the viewport
    while (nextY + nextHeight > viewportTop)
    {
        // Ensure this item is in the viewport
        if (nextY <= viewportBottom)
        {
            // make this item or header visible
            if (nextIndex.item == ListIndex::InvalidItemIndex)
            {
                auto& headerView = headerViews[nextIndex.group];
                if (headerView)
                {
                    headerView->setGeometry(0, nextY, width, nextHeight);
                    headerView->show();
                }
                loadedItems.push_front({nextIndex, nextY, nextHeight, nullptr});
            }
            else
            {
                auto pendingIt = pendingItems.find(nextIndex);
                if (pendingIt != pendingItems.end())
                {
                    auto& item = pendingIt->second;
                    Q_ASSERT(item.y == nextY);
                    Q_ASSERT(item.h == nextHeight);
                    loadedItems.push_front(item);
                }
                else
                {
                    auto view = generateItemView(nextIndex, nextY, nextHeight);
                    loadedItems.push_front({nextIndex, nextY, nextHeight, view});
                }
            }
        }

        // calculate next item's index, y and height
        nextIndex = decreaseIndex(nextIndex);
        if (nextIndex.isEmpty())
        {
            return;
        }
        if (nextIndex.item == ListIndex::InvalidItemIndex)
        {
            auto& headerView = headerViews[nextIndex.group];
            nextHeight = headerView ? headerView->height() : 0;
        }
        else
        {
            nextHeight = itemHeights[nextIndex.group][nextIndex.item];
        }
        nextY -= nextHeight;
    }
}

void ListViewPriv::loadUnderItems()
{
    const auto viewportTop = scrollArea->verticalScrollBar()->value();
    const auto viewportBottom = viewportTop + scrollArea->height();
    const auto width = owner->width();

    auto nextIndex = loadedItems.empty() ? ListIndex(0, ListIndex::InvalidItemIndex) : increaseIndex(loadedItems.back().index);

    if (nextIndex.isEmpty())
    {
        return;
    }

    auto nextY = loadedItems.empty() ? 0 : loadedItems.back().y + loadedItems.back().h;
    auto nextHeight = nextIndex.item == ListIndex::InvalidItemIndex
            ? headerHeight(nextIndex.group)
            : itemHeights[nextIndex.group][nextIndex.item];

    while (!nextIndex.isEmpty() && nextY < viewportBottom)
    {
        if (nextY + nextHeight >= viewportTop)
        {
            auto pendingIt = pendingItems.find(nextIndex);
            if (nextIndex.item == ListIndex::InvalidItemIndex)
            {
                auto& headerView = headerViews[nextIndex.group];
                if (headerView)
                {
                    headerView->setGeometry(0, nextY, width, nextHeight);
                    headerView->show();
                }
                loadedItems.push_back({nextIndex, nextY, nextHeight, nullptr});
            }
            else
            {
                if (pendingIt != pendingItems.end())
                {
                    auto& item = pendingIt->second;
                    Q_ASSERT(item.y == nextY);
                    Q_ASSERT(item.h == nextHeight);
                    loadedItems.push_back(item);
                }
                else
                {
                    auto view = generateItemView(nextIndex, nextY, nextHeight);
                    loadedItems.push_back({nextIndex, nextY, nextHeight, view});
                }
            }
            if (pendingIt != pendingItems.end())
            {
                pendingItems.erase(pendingIt);
            }
        }

        nextIndex = increaseIndex(nextIndex);
        if (nextIndex.isEmpty())
        {
            return;
        }
        nextY += nextHeight;
        if (nextIndex.item == ListIndex::InvalidItemIndex)
        {
            auto& headerView = headerViews[nextIndex.group];
            nextHeight = headerView ? headerView->height() : 0;
        }
        else
        {
            nextHeight = itemHeights[nextIndex.group][nextIndex.item];
        }
    }
}

void ListViewPriv::unloadOutOfViewportItems()
{
    auto isOutOfViewport = [](int y, int h, int viewportTop, int viewportBottom)
    {
        return y > viewportBottom || y + h < viewportTop;
    };
    auto vTop = scrollArea->verticalScrollBar()->value();
    auto vBottom = vTop + scrollArea->height();
    for (auto it = loadedItems.begin(); it != loadedItems.end(); )
    {
        auto& item = *it;
        if (isOutOfViewport(item.y, item.h, vTop, vBottom))
        {
            if (item.index.item == ListIndex::InvalidItemIndex)
            {
                if (auto& view = headerViews[item.index.group])
                {
                    view->hide();
                }
            }
            else
            {
                item.view->owner->hide();
                reusePool[item.view->owner->metaObject()].push_back(item.view);
            }

            it = loadedItems.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void ListViewPriv::recyclePreloadedItems()
{
    for (auto& pair : pendingItems)
    {
        auto& index = pair.first;
        auto& item = pair.second;
        if (index.item == ListIndex::InvalidItemIndex)
        {
            if (auto& headerView = headerViews[index.group])
            {
                headerView->hide();
            }
        }
        else
        {
            item.view->owner->hide();
            reusePool[item.view->owner->metaObject()].push_back(item.view);
        }
    }
    pendingItems.clear();
}

ListIndex ListViewPriv::increaseIndex(const ListIndex &index)
{
    int group, item;
    int nItemsInGroup = (int)itemHeights[index.group].size();
    if (index.item == nItemsInGroup - 1)
    {
        int nGroups = (int)itemHeights.size();
        if (index.group == nGroups - 1)
        {
            group = ListIndex::InvalidGroupIndex;
            item = ListIndex::InvalidItemIndex;
        }
        else
        {
            // ListIndex.item = ListIndex::InvalidItemIndex 代表一个分组的索引
            // 用于加载分组 header 视图~
            group = index.group + 1;
            item = ListIndex::InvalidItemIndex;
        }
    }
    else
    {
        group = index.group;
        item = index.item + 1;
    }
    return ListIndex(group, item);
}

ListIndex ListViewPriv::decreaseIndex(const ListIndex &index)
{
    int group, item;
    if (index.item == ListIndex::InvalidItemIndex)
    {
        if (index.group == 0)
        {
            group = ListIndex::InvalidGroupIndex;
            item = ListIndex::InvalidItemIndex;
        }
        else
        {
            group = index.group - 1;
            int nItemsInGroup = (int)itemHeights[group].size();
            item = nItemsInGroup - 1;
        }
    }
    else
    {
        group = index.group;
        item = index.item - 1;
    }
    return ListIndex(group, item);
}

void ListViewPriv::scrollWithoutNotify(int dy)
{
    auto vs = scrollArea->verticalScrollBar();
    vs->disconnect(owner);
    vs->setValue(vs->value() + dy);
    QObject::connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, owner, [=]{adjustLoadedItems();});
}

void ListViewPriv::adjustItem(ListViewPriv::LoadedItem &item, const ListIndex& newIndex, int y)
{
    item.y = y;
    item.index = newIndex;
    if (item.index.item == ListIndex::InvalidItemIndex)
    {
        auto& view = headerViews[item.index.group];
        if (view)
        {
            view->move(0, y);
        }
    }
    else
    {
        item.view->index = newIndex;
        item.view->owner->move(0, y);
    }
}

std::list<ListViewPriv::LoadedItem>::iterator ListViewPriv::loadedItemAt(const ListIndex &index)
{
    auto it = std::lower_bound(loadedItems.begin(), loadedItems.end(), index, [](const LoadedItem& item, const ListIndex& idx)
    {
        return item.index < idx;
    });
    auto found = (it != loadedItems.end() && it->index == index);
    return found ? it : loadedItems.end();
}

ListViewItemPriv *ListViewPriv::generateItemView(const ListIndex &index, int y, int height)
{
    ListViewItemPriv* result;
    auto meta = currentDelegate->viewMetaObjectForIndex(index);
    auto& list = reusePool[meta];
    if (list.empty())
    {
        auto item = dynamic_cast<ListViewItem*>(meta->newInstance(Q_ARG(QWidget*, scrollContent)));
        result = item->getPriv();
        result->listView = this;
    }
    else
    {
        result = list.front();
        list.pop_front();
    }
    result->owner->setGeometry(0, y, owner->width(), height);
    result->index = index;
    result->selected = OrderedListHelper::contains(selected, index);

    currentDelegate->prepareItemView(index, result->owner);

    result->owner->show();
    return result;
}

void ListViewPriv::processItemSelection(const ListIndex& index)
{
    auto setItemSelected = [this](const ListIndex& itemIndex, bool selected)
    {
        auto itemIt = std::lower_bound(loadedItems.begin(), loadedItems.end(), itemIndex, [](const LoadedItem& item, const ListIndex& idx)
        {
            return item.index < idx;
        });
        if (itemIt != loadedItems.end() && itemIt->index == itemIndex)
        {
            itemIt->view->selected = selected;
            itemIt->view->owner->update();
        }
    };

    if (currentDelegate->isMultipleSelection())
    {
        auto findit = OrderedListHelper::find(selected, index);
        if (findit != selected.end() && *findit == index)
        {
            selected.erase(findit);
            setItemSelected(index, false);
        }
        else
        {
            selected.insert(findit, index);
            setItemSelected(index, true);
        }
        emit owner->selectionChanged();
    }
    else
    {
        if (selected.size() == 1 && *selected.begin() == index)
        {
            return;
        }

        setSelection({index});
    }
}

bool ListViewPriv::modelNotEmpty()
{
    return currentModel && currentModel->owner->numGroups();
}

int ListViewPriv::headerHeight(int group)
{
    auto header = headerViews[group];
    int result = header ? header->height() : 0;
    return result;
}

int ListViewPriv::itemPosition(const ListIndex &index, const ListIndex& refIndex, int refY)
{
    int result = 0;
    int distance = 0;
    auto pair = std::minmax(index, refIndex);
    auto minIndex = pair.first;
    auto maxIndex = pair.second;
    while (minIndex != maxIndex)
    {
        if (maxIndex.group > minIndex.group && minIndex.item == ListIndex::InvalidItemIndex)
        {
            distance += groupHeights[minIndex.group];
            minIndex.group++;
        }
        else
        {
            auto currentHeight = minIndex.item == ListIndex::InvalidItemIndex
                    ? headerHeight(minIndex.group)
                    : itemHeights[minIndex.group][minIndex.item];
            distance += currentHeight;
            minIndex = increaseIndex(minIndex);
        }
    }
    result = index > refIndex ? refY + distance : refY - distance;
    return result;
}
