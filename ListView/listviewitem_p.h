#ifndef LISTVIEWITEM_P_H
#define LISTVIEWITEM_P_H

#include "listviewitem.h"
#include "listdatamodel_p.h"
#include <QtGlobal>

class ListViewPriv;

class ListViewItemPriv
{
public:
    ListViewItem* owner;

    ListIndex index;
    ListViewPriv* listView = nullptr;
    bool hover = false;
    bool pressed = false;
    Qt::MouseButtons pressedButton = Qt::NoButton;
    bool selected = false;
    QPoint pressPos;
    bool isLastItem = false;
};



#endif
