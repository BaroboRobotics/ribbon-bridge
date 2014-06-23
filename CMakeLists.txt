cmake_minimum_required(VERSION 2.8)
project(rpc-test)

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

set(PROTO_SOURCES rpc.proto)
set(PROTO_OPTIONS rpc.options)
set(PROTO_OUTPUT rpc.pb.c)

add_custom_command(OUTPUT ${PROTO_OUTPUT}
    COMMAND ${PROTOC} --nanopb_out=${PROJECT_BINARY_DIR} ${PROTO_SOURCES}
    MAIN_DEPENDENCY ${PROTO_SOURCES}
    DEPENDS ${PROTO_OPTIONS}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating protocol buffers ..."
    VERBATIM)

include_directories(deps/${NANOPB_BASENAME})
#include_directories(deps/simavr/simavr/sim/avr)
if(NOT UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${PROJECT_SOURCE_DIR}/deps/stlport-avr/stlport")
endif()
include_directories(include)
include_directories(src)
include_directories(${PROJECT_BINARY_DIR})

set(SOURCES
    src/rpc.cpp
    deps/${NANOPB_BASENAME}/pb_decode.c
    deps/${NANOPB_BASENAME}/pb_encode.c)

set_source_files_properties(src/rpc.cpp PROPERTIES
    COMPILE_FLAGS "-std=c++11 -D__STDC_FORMAT_MACROS")

add_executable(rpc ${SOURCES} ${PROTO_OUTPUT})

include(${PROJECT_SOURCE_DIR}/arm_macros.cmake)
extract_bin_file(rpc)
extract_ihex_file(rpc)
extract_listing(rpc)
symbol_table(rpc)

enable_testing()
add_subdirectory(tests)
