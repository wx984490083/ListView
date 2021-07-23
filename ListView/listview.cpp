#include "listview_p.h"
#include "listviewitem_p.h"
#include "listdatamodel_p.h"
#include "listview_p.h"
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QDebug>

ListView::ListView(QWidget *parent) : QWidget(parent), priv(new ListViewPriv)
{
    priv->owner = this;
    priv->setup();
}

ListView::~ListView()
{
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

std::set<ListIndex> ListView::selection()
{
    return priv->selection();
}

void ListView::setSelection(std::set<ListIndex> &&selection)
{
    auto tmp = std::move(selection);
    priv->setSelection(selection);
}

void ListView::setSelection(const std::set<ListIndex> &selection)
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
    scrollArea = new QScrollArea(owner);
    scrollArea->setAutoFillBackground(false);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollContent = new QWidget(scrollArea);
    scrollContent->setAutoFillBackground(false);
    scrollArea->setWidget(scrollContent);

    QObject::connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, owner, [=]{adjustLoadedItems();});
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
    auto modelp = model->getPriv();

    if (modelp == currentModel)
    {
        return;
    }

    clear();

    currentModel = modelp;

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

std::set<ListIndex> ListViewPriv::selection()
{
    return selected;
}

void ListViewPriv::setSelection(const std::set<ListIndex> &selection)
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
            auto newSelectedState = (selection.find(item.index) != selection.end());
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

    auto y = itemPosition(index);
    scrollArea->verticalScrollBar()->setValue(y);
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

void ListViewPriv::itemUpdated(const ListIndex &index)
{

}

void ListViewPriv::beginInsertItem(const ListIndex &insertIndex, size_t count)
{

}

void ListViewPriv::endInsertItem()
{

}

void ListViewPriv::beginInsertGroup(int groupIndex)
{

}

void ListViewPriv::endInsertGroup()
{

}

void ListViewPriv::beginRemoveItem(const ListIndex &removeIndex, size_t count)
{

}

void ListViewPriv::endRemoveItem()
{

}

void ListViewPriv::beginRemoveGroup(int groupIndex)
{

}

void ListViewPriv::endRemoveGroup()
{

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

    if (!currentModel->owner->numGroups())
    {
        setupEmptyView();
    }
    else
    {
        cacheHeaders();
        cacheHeightsAndAnchorPos();
        fixContentSize(false);
        adjustLoadedItems();
    }
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
            view->hide();
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
                anchorY = contentHeight;
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
    emptyView = currentDelegate->emptyView();
    if (emptyView)
    {
        emptyView->setParent(owner);
        emptyView->raise();
        emptyView->setGeometry(owner->rect());
        emptyView->show();
    }
}

void ListViewPriv::adjustLoadedItems()
{
    // TODO: 这里的逻辑有点儿简单粗暴，可以考虑细化一下~
    if (modelNotEmpty())
    {
        unloadOutOfViewportItems();
        loadUnderItems();
        loadAboveItems();
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
            qDebug("contentHeight = %d, scrollContentH = %d", contentHeight, scrollContent->height());
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

            view->setGeometry(0, item.y, width, item.h);
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
                headerView->setGeometry(0, nextY, width, nextHeight);
                headerView->show();
                loadedItems.push_front({nextIndex, nextY, nextHeight, nullptr});
            }
            else
            {
                auto view = generateItemView(nextIndex, nextY, nextHeight);
                loadedItems.push_front({nextIndex, nextY, nextHeight, view});
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
                auto view = generateItemView(nextIndex, nextY, nextHeight);
                loadedItems.push_back({nextIndex, nextY, nextHeight, view});
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
    result->selected = selected.find(index) != selected.end();

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
        auto findit = selected.lower_bound(index);
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

int ListViewPriv::itemPosition(const ListIndex &index)
{
    // 当前计算方式为从上往下递加
    // TODO: 如果发现此函数调用频率很高或对UI造成了明显的影响，可建立位置缓存，用于直接搜索结果或加速计算
    int result = 0;
    int currentHeight = 0;
    ListIndex currentIndex(0, ListIndex::InvalidItemIndex);
    while (currentIndex != index)
    {
        if (index.group > currentIndex.group && currentIndex.item == ListIndex::InvalidItemIndex)
        {
            result += groupHeights[currentIndex.group];
            currentIndex.group++;
        }
        else
        {
            currentHeight = currentIndex.item == ListIndex::InvalidItemIndex
                    ? headerHeight(currentIndex.group)
                    : itemHeights[currentIndex.group][currentIndex.item];
            result += currentHeight;
            currentIndex = increaseIndex(currentIndex);
        }
    }
    return result;
}
