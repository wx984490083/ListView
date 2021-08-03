#ifndef LISTDATAMODEL_IMPL_HPP
#define LISTDATAMODEL_IMPL_HPP

#include "listdatamodel.h"
#include <set>

class ListViewPriv;

class ListDataModelPriv
{
public:

    ~ListDataModelPriv();

    ListDataModel* owner;
    std::set<ListViewPriv*> listViewPrivs;
};

#endif
