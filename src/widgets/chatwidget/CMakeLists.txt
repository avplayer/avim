
file(GLOB UIS *.ui)
file(GLOB SRCS *.cpp *.h*)

qt5_wrap_ui(UI_SRCS ${UIS})

add_library(chatwidget STATIC ${SRCS} ${UI_SRCS})
target_link_libraries(chatwidget richtext Qt5::Widgets)

target_include_directories(chatwidget INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
