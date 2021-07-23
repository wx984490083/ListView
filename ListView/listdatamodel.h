#ifndef LISTDATAMODEL_H
#define LISTDATAMODEL_H

#include <inttypes.h>
#include <QtGlobal>

class ListIndex
{
public:
    static const int InvalidGroupIndex = -1;
    static const int InvalidItemIndex = -1;
    ListIndex();
    explicit ListIndex(int group, int item = InvalidItemIndex);
    bool isHeader() const;
    bool isEmpty() const;
    bool operator<(const ListIndex& other) const;
    bool operator==(const ListIndex& other) const;
    bool operator>(const ListIndex& other) const;
    bool operator<=(const ListIndex& other) const;
    bool operator>=(const ListIndex& other) const;
    int32_t group = InvalidGroupIndex;
    int32_t item = InvalidItemIndex;
};


/**
 * 列表数据模型
 * ListView 用它来作为数据源。
 */
class ListDataModel
{
    /// 以下是子类可实现的公有接口
public:
    /**
     * 子类应实现此接口，返回数据的分组数量
     */
    virtual int numGroups() = 0;

    /**
     * 子类应实现此接口，返回分组中的数据项数目
     */
    virtual int numItemsInGroup(int group) = 0;

    /// 以下是子类可实现的保护接口
protected:
    /**
     * 子类可选择实现此函数，返回对应位置的数据对象指针
     * 此函数仅由公有接口 data() 调用
     * 默认实现返回 nullptr ，则对 data() 接口的调用是无意义的
     */
    virtual void* dataForIndex(const ListIndex& index);


public:
    ListDataModel();
    virtual ~ListDataModel();

    ///以下是本类实现的公有接口
public:
    template <class T>
    T* data(const ListIndex& index)
    {
        return reinterpret_cast<T*>(dataForIndex(index));
    }

    ListIndex maxIndex();
    bool isEmpty();


protected:
    /**
     * 请求更多顶部数据。
     * ListView 显示了 ListDataModel 中的第一项数据后会调用此函数。
     * 默认实现不做任何处理。
     * 场景：首次加载和滚动到第一条数据时，可能需要加载更多~
     */
    virtual void onRequestMoreLeadingData();

    /**
     * 请求更多底部数据。
     * ListView 显示了 ListDataModel 中的最后一项数据后会调用此函数。
     * 默认实现不做任何处理。
     * 场景：滚动到最后一条数据时，可能需要加载更多~
     */
    virtual void onRequestMoreTailingData();

    /// 以下是本类已实现的保护接口，供子类调用。
protected:
    /**
     * 当某项数据有更新，需要界面重新加载时，调用此函数
     */
    void itemUpdated(const ListIndex& index);

    /**
     * 通知 ListView 即将插入数据，仅支持在同一个分组中插入数据。
     * 如果要在多个分组中插入数据项，请根据分组索引分次插入。
     * 此函数与 endInsertItem 函数成对使用，在插入数据完成后，应调用 endInsertItem
     * 请确保在插入数据项之前调用此函数，这将使 ListView 及时更新。
     * 请确保本次插入的数据项与传入的参数一致，否则我也不知道会发生什么~。
     * @param insertIndex 从此索引开始插入，即将插入的所有数据项都应与此索引在同一个分组
     * @param count 插入数据项的数量
     */
    void beginInsertItem(const ListIndex& insertIndex, size_t count);

    /**
     * 请确保在插入数据项完成之后调用此函数，这将使 ListView 及时更新。
     * 通知 ListView 已完成数据更改。
     * 如果此前没有调用 beginInsertItem ，那么对此函数的调用是无效的。
     */
    void endInsertItem();

    /**
     * 通知 ListView 即将插入分组，此分组中可带有任意数量的数据项。
     * 此函数与 endInsertGroup 函数成对使用，在插入数据完成后，应调用 endInsertGroup
     * 请确保在插入分组之前调用此函数，这将使 ListView 及时更新。
     * 请确保本次插入的数据项与传入的参数一致，否则我也不知道会发生什么~。
     * @param groupIndex 插入的分组索引
     */
    void beginInsertGroup(int groupIndex);

    /**
     * 请确保在插入分组完成之后调用此函数，这将使 ListView 及时更新。
     * 通知 ListView 已完成数据更改。
     * 如果此前没有调用 beginInsertGroup ，那么对此函数的调用是无效的。
     */
    void endInsertGroup();

    /**
     * 通知 ListView 即将删除数据，仅支持在同一个分组中删除数据。
     * 如果要在多个分组中删除数据项，请根据分组索引分次删除。
     * 此函数与 endRemoveItem 函数成对使用，在删除数据完成后，应调用 endRemoveItem
     * 请确保在删除数据项之前调用此函数，这将使 ListView 及时更新。
     * 请确保本次删除的数据项与传入的参数一致，否则我也不知道会发生什么~。
     * @param insertIndex 从此索引开始删除，即将删除的所有数据项都应与此索引在同一个分组
     * @param count 删除数据项的数量，如果 removeIndex.
     */
    void beginRemoveItem(const ListIndex& removeIndex, size_t count);

    /**
     * 请确保在删除数据项完成之后调用此函数，这将使 ListView 及时更新。
     * 通知 ListView 已完成数据更改。
     * 如果此前没有调用 beginRemoveItem ，那么对此函数的调用是无效的。
     */
    void endRemoveItem();

    /**
     * 通知 ListView 即将删除分组，即删除此分组和其中的所有数据项。
     * 此函数与 endRemoveGroup 函数成对使用，在删除数据完成后，应调用 endRemoveGroup
     * 请确保在删除分组之前调用此函数，这将使 ListView 及时更新。
     * 请确保本次删除的数据项与传入的参数一致，否则我也不知道会发生什么~。
     * @param groupIndex 插入的分组索引
     */
    void beginRemoveGroup(int groupIndex);

    /**
     * 请确保在删除分组完成之后调用此函数，这将使 ListView 及时更新。
     * 通知 ListView 已完成数据更改。
     * 如果此前没有调用 beginRemoveGroup ，那么对此函数的调用是无效的。
     */
    void endRemoveGroup();


private:
    class ListDataModelPriv* priv;
public:
    ListDataModelPriv *getPriv() const;
};

#endif
