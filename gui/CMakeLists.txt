set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)

find_package(Qt5Widgets REQUIRED)

option(ENABLE_QT_STATI "static link to qt" OFF)

if(WIN32 AND MSVC AND ENABLE_QT_STATI)
	# For static Windows builds, we also need to pull in the Qt5 Platform
	# support library, which is not exposed to CMake properly, unfortunately
	GET_TARGET_PROPERTY(QT_LIB_DIR "${Qt5Widgets_LIBRARIES}" LOCATION)
	GET_FILENAME_COMPONENT(QT_LIB_DIR "${QT_LIB_DIR}" PATH)
	FIND_LIBRARY(Qt5Platform_LIBRARIES Qt5PlatformSupport
	HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5QWindows_LIBRARIES qwindows
	HINTS "${QT_LIB_DIR}/../plugins/platforms"
	)
	ADD_DEFINITIONS(-DSTATIC_QT5)
	link_libraries(${Qt5Platform_LIBRARIES} ${Qt5QWindows_LIBRARIES})
endif()

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
