#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include "listdatamodel.h"
#include <QObject>

class ListViewItem;


/**
 * 定义 ListView 的数据展示
 * 注意：实现此类的接口时，应确保可以访问对应的 ListDataModel 对象以获取数据。
 * BUG: 当前受 QWIDGETSIZE_MAX(16777215) 影响，如果数据项过多，即总高度过高，会导致后面的数据无法被显示出来
 * TODO: 优化时将放弃使用 QScrollArea ，采用手动计算位置的方式来避免 QWIDGETSIZE_MAX 带来的影响。
 */
class ListViewDelegate
{
public:

    /**
     * 此函数被 ListView 用于向 ListViewDelegate 请求数据项视图的高度
     * @param index 请求的数据项或分组头的索引
     * @param availableWidth ListView 提供的可用宽度
     * @return 返回高度值
     */
    virtual int heightForIndex(const ListIndex& index, int availableWidth) = 0;

    /**
     * 请求数据项的视图类型元数据，视图类型必须从 ListItemView 继承
     * 用于在 ListView 中生成可复用的数据项视图
     * @param index 请求的数据项索引
     * @return 返回用于展示对应索引的数据项的视图的 staticMetaObject
     */
    virtual const QMetaObject* viewMetaObjectForIndex(const ListIndex& index) = 0;

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
     * 单选 false ，多选 true
     * 默认实现为单选
     * 这只影响鼠标点击 item 时的行为。
     * 即使此函数定义为单选，开发者也可以使用 ListView::setSelection 强行将选中项设为多个，此后鼠标点选时将回到单选模式。
     */
    virtual bool isMultipleSelection();

    /**
     * 如果想要在某个分组的开头展示一个视图，重写这个函数中并返回非空的视图指针
     * ListView 将接管它的生命周期。
     * 默认实现返回 nullptr
     */
    virtual QWidget* headerViewForGroup(int group);

    /**
     * 当 ListView 没有指定 DataModel 或 DataModel 中没有数据时，展示的视图
     * ListView 将接管它的生命周期
     * 此视图会被用于填充整个 ListView ，请注意处理其 resize 事件
     * 默认实现返回 nullptr
     */
    virtual QWidget* emptyView();

    /**
     * 当数据项高度受其宽度影响时，返回 true，默认实现返回 false
     * 如果返回 true，ListView 在宽度改变时会重新计算每个数据项的高度，会比较耗性能哦。
     */
    virtual bool canItemHeightAffectedByWidth();

};

#endif
