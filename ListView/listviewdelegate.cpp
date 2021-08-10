#include "listviewdelegate.h"


void ListViewDelegate::prepareItemView(const ListIndex &, ListViewItem *)
{

}

void ListViewDelegate::cleanItemView(const ListIndex &, ListViewItem *)
{

}

bool ListViewDelegate::canSelectItem(const ListIndex &)
{
    return false;
}

bool ListViewDelegate::isMultipleSelection()
{
    return false;
}

QWidget *ListViewDelegate::headerViewForGroup(int )
{
    return nullptr;
}

QWidget *ListViewDelegate::emptyView()
{
    return nullptr;
}

bool ListViewDelegate::canItemHeightAffectedByWidth()
{
    return false;
}
