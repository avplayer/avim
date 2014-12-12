
file(GLOB UIS *.ui)
file(GLOB SRCS *.cpp *.h*)

qt5_wrap_ui(UI_SRCS ${UIS})

add_library(login STATIC ${SRCS} ${UI_SRCS})
target_link_libraries(login Qt5::Widgets)

target_include_directories(login INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
