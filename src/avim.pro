
QT       += core gui widgets
CXXFLAGS += "c++11"
CONFIG += mobility no_keywords

INCLUDEPATH += /home/cai/android/boost_1_57_0
INCLUDEPATH += /home/cai/android/protobuf/include

INCLUDEPATH += $$PWD/../android-build/include
INCLUDEPATH += $$PWD/../libavim/include
INCLUDEPATH += $$PWD/widgets/chatwidget/
INCLUDEPATH += $$PWD/widgets/qrichtext/

TARGET = avim
TEMPLATE = app subdirs

SOURCES += main.cpp

HEADERS  += main_window.hpp

FORMS    += main_window.ui

CONFIG += mobility
MOBILITY =

SUBDIRS = widgets
