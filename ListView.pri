QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


SOURCES += \   
    $$PWD/listview/listdatamodel.cpp \
    $$PWD/listview/listviewdelegate.cpp \
    $$PWD/listview/listview.cpp \
    $$PWD/listview/listviewitem.cpp

HEADERS += \
    $$PWD/listview/listdatamodel.h \
    $$PWD/listview/listviewdelegate.h \
    $$PWD/listview/listview.h \
    $$PWD/listview/listviewitem.h



