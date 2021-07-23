#include "listdatamodel_p.h"
#include "listview_p.h"

ListIndex::ListIndex(){}

ListIndex::ListIndex(int group, int item) : group(group), item(item) {}

bool ListIndex::isHeader() const {return item == InvalidItemIndex;}

bool ListIndex::isEmpty() const {return group == InvalidGroupIndex;}

bool ListIndex::operator<(const ListIndex &other) const {return group < other.group || (group == other.group && item < other.item);}

bool ListIndex::operator==(const ListIndex &other) const {return group == other.group && item == other.item;}

bool ListIndex::operator!=(const ListIndex &other) const {return group != other.group || item != other.item;}

bool ListIndex::operator>(const ListIndex &other) const {return group > other.group || (group == other.group && item > other.item);}

bool ListIndex::operator<=(const ListIndex &other) const {return *this < other || *this == other;}

bool ListIndex::operator>=(const ListIndex &other) const {return *this > other || *this == other;}



ListDataModel::ListDataModel() : priv(new ListDataModelPriv)
{
    priv->owner = this;
    qDebug("this = %x, priv.addr = %x", this, &priv);
}

ListDataModel::~ListDataModel()
{
    delete priv;
}

ListIndex ListDataModel::maxIndex()
{
    auto maxGroup = numGroups() - 1;
    if (maxGroup >= 0)
    {
        auto maxItem = numItemsInGroup(maxGroup) - 1;
        return ListIndex(maxGroup, maxItem);
    }
    return ListIndex();
}

bool ListDataModel::isEmpty()
{
    auto result = numGroups() == 0;
    return result;
}

void *ListDataModel::dataForIndex(const ListIndex &)
{
    return nullptr;
}

void ListDataModel::onRequestMoreLeadingData()
{

}

void ListDataModel::onRequestMoreTailingData()
{

}

void ListDataModel::itemUpdated(const ListIndex &index)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->itemUpdated(index);
    }
}

void ListDataModel::beginInsertItem(const ListIndex &insertIndex, size_t count)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->beginInsertItem(insertIndex, count);
    }
}

void ListDataModel::endInsertItem()
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->endInsertItem();
    }
}

void ListDataModel::beginInsertGroup(int groupIndex)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->beginInsertGroup(groupIndex);
    }
}

void ListDataModel::endInsertGroup()
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->endInsertGroup();
    }
}

void ListDataModel::beginRemoveItem(const ListIndex &removeIndex, size_t count)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->beginRemoveItem(removeIndex, count);
    }
}

void ListDataModel::endRemoveItem()
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->endRemoveItem();
    }
}

void ListDataModel::beginRemoveGroup(int groupIndex)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->beginRemoveGroup(groupIndex);
    }
}

void ListDataModel::endRemoveGroup()
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->endRemoveGroup();
    }
}



ListDataModelPriv::~ListDataModelPriv()
{
    for (auto& listViewPriv : listViewPrivs)
    {
        listViewPriv->setDataModel(nullptr);
    }
}

ListDataModelPriv *ListDataModel::getPriv() const
{
    return priv;
}
