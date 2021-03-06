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

void ListDataModel::requireReload()
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->requireReload();
    }
}

void ListDataModel::itemUpdated(const ListIndex &index)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->itemUpdated(index);
    }
}

void ListDataModel::beginInsertItems(const ListIndex &insertIndex, size_t count)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->beginInsertItem(insertIndex, count);
    }
}

void ListDataModel::endInsertItems()
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

void ListDataModel::beginRemoveItems(const ListIndex &removeIndex, size_t count)
{
    for (auto& listViewPriv : priv->listViewPrivs)
    {
        listViewPriv->beginRemoveItem(removeIndex, count);
    }
}

void ListDataModel::endRemoveItems()
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
    while (!listViewPrivs.empty())
    {
        (*listViewPrivs.begin())->setDataModel(nullptr);
    }
}

ListDataModelPriv *ListDataModel::getPriv() const
{
    return priv;
}
