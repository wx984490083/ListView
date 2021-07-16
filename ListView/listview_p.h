#ifndef LISTVIEW_IMPL_HPP
#define LISTVIEW_IMPL_HPP

#include "listview.h"

class ListViewItemPriv;

class ListViewPriv
{
public:
    ListView* owner;

    void processItemClick(QMouseEvent *event, const ListIndex& index, ListViewItemPriv* item);
    void setDataModel(ListDataModel* model);
    void setViewDelegate(ListViewDelegate* delegate);
    std::set<ListIndex> selection();
    void setSelection(std::set<ListIndex>&& selection);
    void setSelection(const std::set<ListIndex>& selection);
    void scrollToItem(const ListIndex& index);
    void scrollToTop();
    void scrollToBottom();

    void itemUpdated(const ListIndex& index);
    void beginInsertItem(const ListIndex& insertIndex, size_t count);
    void endInsertItem();
    void beginInsertGroup(int groupIndex);
    void endInsertGroup();
    void beginRemoveItem(const ListIndex& removeIndex, size_t count);
    void endRemoveItem();
    void beginRemoveGroup(int groupIndex);
    void endRemoveGroup();

private:
    ListDataModelPriv* currentModel = nullptr;
    ListViewDelegate* currentDelegate = nullptr;
    std::set<ListIndex> selected;

    std::vector<std::vector<int>> heightsCache;

    void clear();
    void reload();

    void cacheGeometries();

    void addSelection(std::set<ListIndex>&& selection);
};

#endif
