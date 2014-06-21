cmake_minimum_required(VERSION 2.8)
project(rpc-test)

set(PROTO_SOURCES rpc.proto)
set(PROTO_OPTIONS rpc.options)
set(PROTO_OUTPUT rpc.pb.c)

add_custom_command(OUTPUT ${PROTO_OUTPUT}
    COMMAND protoc --nanopb_out=${PROJECT_BINARY_DIR} ${PROTO_SOURCES}
    MAIN_DEPENDENCY ${PROTO_SOURCES}
    DEPENDS ${PROTO_OPTIONS}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating protocol buffers ..."
    VERBATIM)

include_directories(pb)
include_directories(${PROJECT_BINARY_DIR})

set(SOURCES
    rpc.cpp
    pb/pb_decode.c
    pb/pb_encode.c)

set_source_files_properties(rpc.cpp PROPERTIES COMPILE_FLAGS -std=c++11)

add_executable(rpc ${SOURCES} ${PROTO_OUTPUT})
