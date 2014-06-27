cmake_minimum_required(VERSION 2.8)
project(rpc)

set(CMAKE_C_FLAGS "-Wall -Wextra -Os -fno-exceptions")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Os -fno-rtti -fno-exceptions")
if(NOT UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -mmcu=atmega128rfa1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -mmcu=atmega128rfa1")
    # I think the _mmcu/.mmcu stuff was for simavr ...
    set(CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega128rfa1 -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000")
endif()

##############################################################################
# Set up nanopb

set(NANOPB_BASENAME nanopb-0.2.8-linux-x86)
set(PROTOC "${PROJECT_SOURCE_DIR}/deps/${NANOPB_BASENAME}/generator-bin/protoc")

set(PROTO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/proto)

set(RPC_PROTO_SOURCES rpc.proto)
set(RPC_PROTO_OPTIONS rpc.options)
set(RPC_PROTO_C_FILE rpc.pb.c)
set(RPC_PROTO_H_FILE rpc.pb.h)

add_custom_command(OUTPUT ${RPC_PROTO_C_FILE} ${RPC_PROTO_H_FILE}
    COMMAND ${PROTOC} --nanopb_out=${CMAKE_CURRENT_BINARY_DIR} ${RPC_PROTO_SOURCES}
    MAIN_DEPENDENCY ${PROTO_SOURCE_DIR}/${RPC_PROTO_SOURCES}
    DEPENDS ${PROTO_SOURCE_DIR}/${RPC_PROTO_OPTIONS}
    WORKING_DIRECTORY ${PROTO_SOURCE_DIR}
    COMMENT "Generating RPC protocol buffers ..."
    VERBATIM)

##############################################################################

#include_directories(deps/simavr/simavr/sim/avr)

set(SOURCES
    src/printmessage.cpp
    src/encode_decode.cpp
    ${RPC_PROTO_C_FILE}
    deps/${NANOPB_BASENAME}/pb_decode.c
    deps/${NANOPB_BASENAME}/pb_encode.c)

set_source_files_properties(src/printmessage.cpp src/encode_decode.cpp PROPERTIES
    COMPILE_FLAGS "-std=c++11 -D__STDC_FORMAT_MACROS")

add_library(rpc STATIC ${SOURCES})

target_include_directories(rpc
    PUBLIC deps/${NANOPB_BASENAME}
    PUBLIC include
    PRIVATE src
    PUBLIC ${PROJECT_BINARY_DIR}
    )

##############################################################################

enable_testing()
add_subdirectory(tests)
