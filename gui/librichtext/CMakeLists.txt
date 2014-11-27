
file(GLOB SRCS *.cpp *.h*)

add_library(richtext STATIC ${SRCS})
target_link_libraries(richtext Qt5::Widgets)
