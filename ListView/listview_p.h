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

    void processItemClick(QMouseEvent *event, const ListIndex& index, ListViewItemPriv* item);
    void setDataModel(ListDataModel* model);
    void setViewDelegate(ListViewDelegate* delegate);

    ListDataModel* dataModel() const;
    ListViewDelegate* viewDelegate() const;

    std::set<ListIndex> selection();
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

    /**
     * 记录已加载的数据项视图
     * 开发过程需要保证：其中的数据项索引是严格递增的，不会出现跳跃
     */
    std::list<LoadedItem> loadedItems;

    std::set<ListIndex> selected;

    QWidget* emptyView = nullptr;
    std::vector<QWidget*> headerViews;
    std::vector<int> groupHeights;
    std::vector<std::vector<int>> itemHeights;
    int contentHeight;

    void clear();
    void reload();

    void cacheHeaders();
    int cacheHeightsAndAnchorPos(const ListIndex &anchorIndex = ListIndex());

    void setupEmptyView();

    void adjustLoadedItems();

    /**
     * ListView 尺寸改变后，对已加载的视图做一些调整
     * 可能会改变 contentHeight，并且改变滚动条位置。
     */
    void fixContentSize(bool widthChanged);

    void loadAboveItems();
    void loadUnderItems();
    void unloadOutOfViewportItems();

    ListViewItemPriv* generateItemView(const ListIndex& index, int y, int height);

    void processItemSelection(const ListIndex &index);

    // some helper functions
    bool modelNotEmpty();
    int headerHeight(int group);
    int itemPosition(const ListIndex& index);
    ListIndex increaseIndex(const ListIndex& index);
    ListIndex decreaseIndex(const ListIndex& index);
};

#endif
