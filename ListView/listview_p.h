#ifndef LISTVIEW_IMPL_HPP
#define LISTVIEW_IMPL_HPP

#include "listview.h"
#include <QScrollArea>

class ListViewItemPriv;

class ListViewPriv
{
public:
    ListView* owner;

    void setup();
    void cleanup();

    void processItemClick(QMouseEvent *event, const ListIndex& index, ListViewItemPriv* item);
    void setDataModel(ListDataModel* model);
    void setViewDelegate(ListViewDelegate* delegate);

    ListDataModel* dataModel() const;
    ListViewDelegate* viewDelegate() const;

    std::list<ListIndex> selection();
    void setSelection(const std::list<ListIndex>& selection);
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

    void onResized(const QSize &oldSize);

private:
    struct LoadedItem
    {
        ListIndex index;
        int y;
        int h;
        ListViewItemPriv* view;
    };

    QScrollArea* scrollArea;
    QWidget* scrollContent;

    ListDataModelPriv* currentModel = nullptr;
    ListViewDelegate* currentDelegate = nullptr;

    std::map<const QMetaObject*, std::list<ListViewItemPriv*>> reusePool;

    std::list<LoadedItem> loadedItems;

    std::map<ListIndex, LoadedItem> pendingItems;

    std::list<ListIndex> selected;

    QWidget* emptyView = nullptr;
    std::vector<QWidget*> headerViews;
    std::vector<int> groupHeights;
    std::vector<std::vector<int>> itemHeights;
    int contentHeight = 0;

    enum ModifyMode
    {
        ModifyModeNone,
        ModifyModeInsertItem,
        ModifyModeRemoveItem,
        ModifyModeInsertGroup,
        ModifyModeRemoveGroup
    };

    struct ModifyInfo
    {
        int mode = ModifyModeNone;
        ListIndex index;
        int count = 0;
    }modifyInfo;

    void clear();
    void reload();

    void cacheHeaders();
    int cacheHeightsAndAnchorPos(const ListIndex &anchorIndex = ListIndex());

    void setupEmptyView();
    void clearEmptyView();

    void adjustLoadedItems();

    /**
     * ListView 尺寸改变后，对已加载的视图做一些调整
     * 可能会改变 contentHeight，并且改变滚动条位置。
     */
    void fixContentSize(bool widthChanged);

    void loadAboveItems();
    void loadUnderItems();
    void unloadOutOfViewportItems();
    void recyclePreloadedItems();

    ListViewItemPriv* generateItemView(const ListIndex& index, int y, int height);

    void processItemSelection(const ListIndex &index);

    // some helper functions
    bool modelNotEmpty();
    int headerHeight(int group);
    int itemPosition(const ListIndex& index);
    ListIndex increaseIndex(const ListIndex& index);
    ListIndex decreaseIndex(const ListIndex& index);
    void scrollWithoutNotify(int dy);

    void adjustItem(LoadedItem& item, const ListIndex &newIndex, int y);

    std::list<LoadedItem>::iterator loadedItemAt(const ListIndex& index);
};

#endif
