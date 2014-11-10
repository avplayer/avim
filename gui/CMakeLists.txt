cmake_minimum_required(VERSION 2.8.9)

project(gavim)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${QT_INCLUDES})

QT5_WRAP_UI(UI_HEADERS gavim.ui)


add_executable(gavim ${UI_HEADERS} gavim.cpp gavim.h main.cpp gavim.qrc)
target_link_libraries(gavim ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

qt5_use_modules(gavim Widgets Core Gui)

set_target_properties(gavim PROPERTIES AUTOMOC TRUE)
