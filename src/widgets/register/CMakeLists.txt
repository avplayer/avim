
file(GLOB UIS *.ui)
file(GLOB SRCS *.cpp *.h*)

qt5_wrap_ui(UI_SRCS ${UIS})

add_library(register STATIC ${SRCS} ${UI_SRCS})
target_link_libraries(register Qt5::Widgets)

target_include_directories(register INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
