#ifndef LISTDATAMODEL_IMPL_HPP
#define LISTDATAMODEL_IMPL_HPP

#include "listdatamodel.h"
#include <list>

class ListViewPriv;

class ListDataModelPriv
{
public:

    ~ListDataModelPriv();

    ListDataModel* owner;
    std::list<ListViewPriv*> listViewPrivs;
};

#endif
