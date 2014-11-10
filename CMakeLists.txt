cmake_minimum_required(VERSION 2.8.11)

project(avim)

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
	set(Boost_USE_MULTITHREADED TRUE)
	set(Boost_USE_STATIC_LIBS TRUE)
	if(DEFINED PROTOBUF_SRC_ROOT_FOLDER)
		set(PROTOBUF_SRC_ROOT_FOLDER $ENV{PROTOBUF_SRC_ROOT_FOLDER})
	endif()
endif()

option(RUN_PROTOC "run protoc to generate protobuf protocol parser" ON)
option(BUILD_C_WRAPPER "build a libavproto that exports C interface" OFF)
option(BUILD_GUI "build gui" ON)

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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(EXISTS ${PROJECT_SOURCE_DIR}/avim_proto/CMakeLists.txt )
	add_subdirectory(avim_proto)
else()
	message( FATAL_ERROR "please run 'git submodule update --init' to get avim_proto")
endif()


add_subdirectory(libavproto)

link_libraries(avproto++)


add_subdirectory(demo)

if(BUILD_GUI)
add_subdirectory(gui)
endif()

check_library_exists(rt clock_gettime "" HAVE_CLOCK_GETTIME)
if(HAVE_CLOCK_GETTIME)
        link_libraries(-lrt)
endif(HAVE_CLOCK_GETTIME)
