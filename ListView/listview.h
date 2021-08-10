#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <set>
#include <QWidget>
#include "listdatamodel.h"
#include "listviewdelegate.h"
#include "listviewitem.h"

/**
 * 😆 一个神奇的纵向列表视图类 😆
 *
 * 使用步骤：
 * 1. 定义一个类实现 ListDataModel 相关接口
 * 2. 定义一个类实现 ListViewDelegate 相关接口，可以与 ListDataModel 接口在同一个类实现，但不推荐
 * 3. 创建 ListView, ListDataModel子类, ListViewDelegate子类 对象各一个
 * 4. 调用此类对象的 setDataModel 将 ListDataModel 子类对象和它关联到一起
 * 5. 调用此类对象的 setViewDelegate 将 ListViewDelegate 子类对象和它关联到一起
 * 6. 像使用其他 QWidget 对象一样使用此类对象
 * 7. 使用完成后，以任意顺序删除第 3 步创建的对象。
 *
 */
class ListView : public QWidget
{
    Q_OBJECT
public:
    ListView(QWidget* parent);
    ~ListView();

    void setDataModel(ListDataModel* model);
    void setViewDelegate(ListViewDelegate* delegate);

    ListDataModel* dataModel() const;
    ListViewDelegate* viewDelegate() const;

    std::list<ListIndex> selection();
    void setSelection(std::list<ListIndex>&& selection);
    void setSelection(const std::list<ListIndex>& selection);

    /**
     * 滚动视图使 index 代表的数据项显示出来。
     * @param item 需要显示的数据项索引。
     */
    void scrollToItem(const ListIndex& index);

    void scrollToTop();
    void scrollToBottom();

    class ListViewPriv *getPriv() const;

signals:
    void selectionChanged();
    void itemsInserted(const ListIndex& index, int count);
    void itemsRemoved(const ListIndex& index, int count);
    void groupInserted(int group);
    void groupRemoved(int group);
    void itemLeftClicked(const ListIndex& index, ListViewItem* item, QMouseEvent* e);
    void itemRightClicked(const ListIndex& index, ListViewItem* item, QMouseEvent* e);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    class ListViewPriv* priv;
};


Q_DECLARE_METATYPE(ListIndex)


#endif
