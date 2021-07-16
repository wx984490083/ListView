#include "listview_p.h"
#include "listviewitem_p.h"
#include "listdatamodel_p.h"
#include <QMouseEvent>

ListView::ListView(QWidget *parent) : QWidget(parent), priv(new ListViewPriv)
{
    priv->owner = this;
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

std::set<ListIndex> ListView::selection()
{
    return priv->selection();
}

void ListView::setSelection(std::set<ListIndex> &&selection)
{
    priv->setSelection(std::move(selection));
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



void ListViewPriv::processItemClick(QMouseEvent *event, const ListIndex &index, ListViewItemPriv *item)
{
    if (event->button() == Qt::LeftButton)
    {
        if (currentDelegate->canSelectItem(index))
        {
            addSelection({index});
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
    // ListDataModel 的第一个（也是唯一）成员是 ListDataModelPriv
    // 它们的地址相同，所以直接转换~
    auto modelp = reinterpret_cast<ListDataModelPriv*>(model);

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
    currentDelegate = delegate;
}

std::set<ListIndex> ListViewPriv::selection()
{
    return selected;
}

void ListViewPriv::setSelection(std::set<ListIndex> &&selection)
{
    selected = std::move(selection);

    // TODO: 添加判断确认是否真的有改变~
    emit owner->selectionChanged();
}

void ListViewPriv::setSelection(const std::set<ListIndex> &selection)
{
    selected = selection;

    // TODO: 添加判断确认是否真的有改变~
    emit owner->selectionChanged();
}

void ListViewPriv::scrollToItem(const ListIndex &index)
{
    if (!currentModel || currentModel->owner->isEmpty())
    {
        return;
    }

}

void ListViewPriv::scrollToTop()
{
    scrollToItem(ListIndex(0, ListIndex::GroupHeaderIndex));
}

void ListViewPriv::scrollToBottom()
{
    if (!currentModel)
    {
        return;
    }
    auto bottomIndex = currentModel->owner->maxIndex();
    scrollToItem(bottomIndex);
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

void ListViewPriv::clear()
{

}

void ListViewPriv::reload()
{
    if (!currentModel || !currentDelegate)
    {
        return;
    }

    cacheGeometries();
}

void ListViewPriv::cacheGeometries()
{

}

void ListViewPriv::addSelection(std::set<ListIndex> &&selection)
{

}
