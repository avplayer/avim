cmake_minimum_required(VERSION 2.8.9)

project(libavprotocol)


add_library(avproto++ STATIC src/root_ca.cpp src/avkernel.cpp src/avif.cpp src/avtcpif.cpp)
set_target_properties(avproto++ PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(avproto++ avimproto-base ${PROTOBUF_LIBRARIES})

if(MSVC)
set_source_files_properties(src/avC_stub.cpp PROPERTIES COMPILE_FLAGS "/DDLL_EXPORTS=1")
endif()

if( BUILD_C_WRAPPER )

add_library( avproto SHARED src/avC_stub.cpp)
target_link_libraries(avproto avproto++)

if(MSVC)
if(CMAKE_CL_64)
set_target_properties(avproto
		PROPERTIES
		OUTPUT_NAME avproto64
		PREFIX ""
		CLEAN_DIRECT_OUTPUT 1)
else()
set_target_properties(avproto
		PROPERTIES
		OUTPUT_NAME avproto
		PREFIX ""
		CLEAN_DIRECT_OUTPUT 1)
endif()
endif()

endif( BUILD_C_WRAPPER )
