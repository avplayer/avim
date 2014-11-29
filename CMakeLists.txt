cmake_minimum_required(VERSION 2.8.11)

project(avim)

if(WIN32)
	add_definitions(-DBOOST_ALL_STATIC_LINK)
	add_definitions(-DBOOST_THREAD_USE_LIB)
	add_definitions(-DBOOST_FILESYSTEM_STATIC_LINK)
	add_definitions(-DBOOST_ALL_STATIC_LINK)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_COMPILER_IS_CLANGXX 1)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	if(UNIX)
		set(CMAKE_COMPILER_IS_UNIX_ICC 1)
	else()
		# CMAKE_COMPILER_IS_WIN_ICC 的 c++11 模式命令行参数不一样
		set(CMAKE_COMPILER_IS_WIN_ICC 1)
	endif()
endif()

if(MSVC)
	add_definitions(-DNOMINMAX)
    add_definitions("/wd4819")
	set(Boost_USE_MULTITHREADED TRUE)
	set(Boost_USE_STATIC_LIBS TRUE)

	if(NOT DEFINED PROTOBUF_SRC_ROOT_FOLDER)
		if(DEFINED ENV{PROTOBUF_SRC_ROOT_FOLDER})
			set(PROTOBUF_SRC_ROOT_FOLDER $ENV{PROTOBUF_SRC_ROOT_FOLDER})
		else()
			set(PROTOBUF_SRC_ROOT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf-2.6.1/)

			if( NOT EXISTS third_party/protobuf-2.6.1/Debug)
				message(FATAL_ERROR "please compile protobuf in VC -- open and press F7")
			endif()
		endif()
	endif()
endif()

option(RUN_PROTOC "run protoc to generate protobuf protocol parser" ON)
option(BUILD_C_WRAPPER "build a libavproto that exports C interface" OFF)
option(BUILD_GUI "build gui" ON)
option(ENABLE_QT_STATIC "static link to qt" ON)

set(BUILD_STATIC ${ENABLE_QT_STATIC})

include(tweakvcflasgs.cmake)

find_package(Threads REQUIRED)
find_package(Boost 1.54 REQUIRED COMPONENTS thread system program_options coroutine context regex date_time chrono filesystem )
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
#find_package(PkgConfig REQUIRED)

#pkg_check_modules(PKCS11 REQUIRED libpkcs11-helper-1 )

include_directories(
	${PROTOBUF_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIR}
	#${PKCS11_INCLUDE_DIRS}
	libavproto/include
)
link_directories(${Boost_LIB_DIRS})

link_libraries(${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${PKCS11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX OR CMAKE_COMPILER_IS_UNIX_ICC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
endif()

if(EXISTS ${PROJECT_SOURCE_DIR}/avim_proto/CMakeLists.txt )
	add_subdirectory(avim_proto)
else()
	message( FATAL_ERROR "please run 'git submodule update --init' to get avim_proto")
endif()

check_library_exists(rt clock_gettime "" HAVE_CLOCK_GETTIME)
if(HAVE_CLOCK_GETTIME)
        link_libraries(-lrt)
endif(HAVE_CLOCK_GETTIME)

add_subdirectory(libavproto)

link_libraries(avproto++)

if(BUILD_GUI)
add_subdirectory(gui)
endif()

add_subdirectory(wrapper)
