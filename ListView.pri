QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


SOURCES += \   
    $$PWD/ListView/smoothscrollarea.cpp \
    $$PWD/ListView/listdatamodel.cpp \
    $$PWD/ListView/listviewdelegate.cpp \
    $$PWD/ListView/listview.cpp \
    $$PWD/ListView/listviewitem.cpp

HEADERS += \
    $$PWD/ListView/smoothscrollarea.h \
    $$PWD/ListView/listdatamodel.h \
    $$PWD/ListView/listviewdelegate.h \
    $$PWD/ListView/listview.h \
    $$PWD/ListView/listviewitem.h

INCLUDEPATH += $$PWD


