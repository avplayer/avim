
file(GLOB UIS *.ui)
file(GLOB SRCS *.cpp *.h*)

qt5_wrap_ui(UI_SRCS ${UIS})

add_library(mainwindow STATIC ${SRCS} ${UI_SRCS})
target_link_libraries(mainwindow Qt5::Widgets)

target_include_directories(mainwindow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
