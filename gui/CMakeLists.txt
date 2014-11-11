project(gavim)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)

file(GLOB HDRS include/*.h*)

set(SRCS
	main.cpp
	gavim.cpp
	login_dialog.cpp
)

set(UIS
	gavim.ui
	login_dialog.ui
)

qt5_wrap_ui(UI_HEADERS ${UIS})

add_executable(gavim WIN32 ${UI_HEADERS} ${HDRS} ${SRCS})
target_link_libraries(gavim Qt5::Widgets)

set_target_properties(gavim PROPERTIES AUTOMOC TRUE)
