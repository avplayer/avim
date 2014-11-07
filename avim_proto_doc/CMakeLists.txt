cmake_minimum_required(VERSION 2.8.9)
# include(./FindProtobuf.cmake)

PROTOBUF_GENERATE_CPP(PROTO_MESSAGE PROTO_MESSAGE_HDRS message.proto)
PROTOBUF_GENERATE_CPP(PROTO_USER PROTO_USER_HDRS user.proto)
PROTOBUF_GENERATE_CPP(PROTO_OLD PROTO_OLD_HDRS old.proto)

add_library(avimproto STATIC ${PROTO_USER} ${PROTO_MESSAGE} ${PROTO_OLD})
target_link_libraries(avimproto ${PROTOBUF_LIBRARIES})

set_target_properties(avimproto PROPERTIES POSITION_INDEPENDENT_CODE ON)
