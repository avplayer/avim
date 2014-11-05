cmake_minimum_required(VERSION 2.8.9)

project(libavprotocol)


add_library( avprotocol STATIC src/root_ca.cpp src/avkernel.cpp src/avif.cpp src/avtcpif.cpp)
set_target_properties(avprotocol PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library( avprotocolC SHARED src/avC_stub.cpp)
target_link_libraries( avprotocolC avprotocol)
