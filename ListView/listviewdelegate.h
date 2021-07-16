#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include "listdatamodel.h"
#include <QObject>

class ListViewItem;


/**
 * 定义 ListView 的数据展示
 * 注意：实现此类的接口时，应确保可以访问对应的 ListDataModel 对象以获取数据。
 */
class ListViewDelegate
{
public:
    /**
     * 此函数被 ListView 用于向 ListViewDelegate 请求数据项或分组头视图的高度
     * 请注意区分数据项和分组头~
     * @param index 请求的数据项或分组头的索引
     * @param availableWidth ListView 提供的可用宽度
     * @return 返回高度值
     */
    virtual int heightForIndex(const ListIndex& index, int availableWidth);

    /**
     * 请求数据项的视图类型元数据，视图类型必须从 ListItemView 继承
     * 用于在 ListView 中生成可复用的数据项视图
     * @param index 请求的数据项索引
     * @return 返回用于展示对应索引的数据项的视图的 staticMetaObject
     */
    virtual const QMetaObject* viewMetaObjectForIndex(const ListIndex& index);

    /**
     * 准备数据项视图
     * 在 ListView 即将展示对应的数据项时会调用此函数。
     * @param index 数据项索引
     * @param view 数据项视图
     */
    virtual void prepareItemView(const ListIndex& index, ListViewItem* view);

    /**
     * 清理数据项视图
     * 在 ListView 即将回收对应的数据项时会调用此函数。
     * @param index 数据项索引
     * @param view 数据项视图，函数调用完成后此视图会被收回 ListView 的复用池。
     */
    virtual void cleanItemView(const ListIndex& index, ListViewItem* view);

    /**
     * 如果 index 指定的数据项可以被选中，请返回 true ，否则返回 false
     */
    virtual bool canSelectItem(const ListIndex& index);

    /**
     * 当 ListView 没有指定 DataModel 或 DataModel 中没有数据时，展示的视图
     */
    virtual QWidget* emptyView();




};

#endif
