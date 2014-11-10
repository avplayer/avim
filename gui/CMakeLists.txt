project(gavim)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)

qt5_wrap_ui(UI_HEADERS gavim.ui)

add_executable(gavim WIN32 ${UI_HEADERS} gavim.cpp gavim.h main.cpp gavim.qrc)
target_link_libraries(gavim Qt5::Widgets)

set_target_properties(gavim PROPERTIES AUTOMOC TRUE)
