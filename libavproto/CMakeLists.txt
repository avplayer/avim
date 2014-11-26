cmake_minimum_required(VERSION 2.8.9)

project(libavprotocol)

set(SRCS
	src/root_ca.cpp
	src/serialization.cpp
	src/avkernel.cpp
	src/avif.cpp
	src/avjackif.cpp
	src/message.cpp
)

file(GLOB HDRS include/*.h*)

add_library(avproto++ STATIC ${HDRS} ${SRCS})
set_target_properties(avproto++ PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(avproto++ avimproto ${PROTOBUF_LIBRARIES})

if(MSVC)
set_source_files_properties(src/avC_stub.cpp PROPERTIES COMPILE_FLAGS "/DDLL_EXPORTS=1")
endif()

