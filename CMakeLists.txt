cmake_minimum_required(VERSION 2.8)
project(rpc-test)

set(CMAKE_C_FLAGS "-Wall -Wextra -Os -fno-exceptions")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Os -fno-rtti -fno-exceptions")
if(NOT UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -mmcu=atmega128rfa1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -mmcu=atmega128rfa1")
    #add_definitions("-DPHY_ATMEGA128RFA1 -DHAL_ATMEGA128RFA1"
    #"-DPLATFORM_RCB128RFA1 -DF_CPU=16000000L -DVERSION_MAJOR=${VERSION_MAJOR} -DVERSION_MINOR=${VERSION_MINOR} -DVERSION_PATCH=${VERSION_PATCH}")
                #set(CMAKE_C_FLAGS "-Wall --std=gnu99 -O2 -fno-zero-initialized-in-bss -fdata-sections -ffunction-sections -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -mmcu=atmega128rfa1")
    set(CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega128rfa1 -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000")
endif()

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

include_directories(deps/pb)
include_directories(deps/simavr/simavr/sim/avr)
if(NOT UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${PROJECT_SOURCE_DIR}/deps/stlport-avr/stlport")
endif()
include_directories(include)
include_directories(src)
include_directories(${PROJECT_BINARY_DIR})

set(SOURCES
    src/rpc.cpp
    deps/pb/pb_decode.c
    deps/pb/pb_encode.c)

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
