set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DQT_NO_KEYWORDS)

find_package(Qt5Widgets REQUIRED)

file(GLOB HDRS include/*.h*)
file(GLOB SRCS *.cpp *.h*)
file(GLOB UIS *.ui)

qt5_wrap_ui(UI_SRCS ${UIS})
qt5_add_resources(RC_SRCS avim.qrc)

set(SRCS ${SRCS} ${UI_SRCS} ${RC_SRCS})

if(APPLE)
	set(SRCS ${SRCS} avim.icns)
	set(MACOSX_BUNDLE_ICON_FILE avim.icns)
	set_source_files_properties(avim.icns PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources)
endif()

add_executable(avim WIN32 MACOSX_BUNDLE ${SRCS})
target_link_libraries(avim Qt5::Widgets)

if(APPLE)
	set_target_properties(avim PROPERTIES
		OUTPUT_NAME avim
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
	)
endif()
