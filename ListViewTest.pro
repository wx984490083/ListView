QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    listviewtest/main.cpp \
    listviewtest/mainwindow.cpp

HEADERS += \
    listviewtest/mainwindow.h

FORMS += \
    listviewtest/mainwindow.ui

include(ListView.pri)

win32-msvc: QMAKE_CXXFLAGS += /utf-8

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32: LIBS += -L$$PWD'/../../Program Files (x86)/Visual Leak Detector/lib/Win32/' -lvld

#INCLUDEPATH += $$PWD'/../../Program Files (x86)/Visual Leak Detector/include'
#DEPENDPATH += $$PWD'/../../Program Files (x86)/Visual Leak Detector/include'
