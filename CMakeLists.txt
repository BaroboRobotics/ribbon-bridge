cmake_minimum_required(VERSION 2.8)
project(rpc)

set(AVR OFF)
if(CMAKE_SYSTEM_NAME MATCHES "AVR")
    set(AVR ON)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Os")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Os")

if(AVR)
    set(FEATURE_DEFS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmcu=atmega128rfa1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmcu=atmega128rfa1")
    # I think the _mmcu/.mmcu stuff was for simavr ...
    set(CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega128rfa1 -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000")
else()
    set(FEATURE_DEFS "-DHAVE_EXCEPTIONS -DHAVE_RTTI -DHAVE_STDLIB")
endif()

##############################################################################
# Set up nanopb

set(NANOPB_BASENAME nanopb-0.2.8-linux-x86)

set(PROTOC_INCLUDE_DIR
    "${PROJECT_SOURCE_DIR}/deps/${NANOPB_BASENAME}/generator/proto"
    CACHE PATH "protocol buffer compiler's include directory")

set(PROTOC_EXECUTABLE
    "${PROJECT_SOURCE_DIR}/deps/${NANOPB_BASENAME}/generator-bin/protoc"
    CACHE FILEPATH "protocol buffer compiler")

# rpc_compile_proto(<c-filepath-result-var>
#                   <h-filepath-result-var>
#                   <proto-filepath>)
#
# Compile a .proto file to a .c and a .h using nanopb's protocol buffer
# compiler. Set <c-filepath-result-var> and <h-filepath-result-var> to the
# absolute paths of the generated .c and .h files, respectively.
# <proto-filepath> is the input file; its path will be interpreted relative to
# the current source directory, if it is not absolute. <c-filepath-result-var>
# and <h-filepath-result-var> 
function(rpc_compile_proto cFilepathResultVar hFilepathResultVar protoFilepath)
    # Get the input .proto file's name, stripped of the last extension. Note:
    # using get_filename_component(... NAME_WE) instead of the funky regex
    # business would incorrectly strip all extensions. protoc only strips the
    # last, and we want to mimic its behavior.
    get_filename_component(_name ${protoFilepath} NAME)
    string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _basename ${_name})
    set(cOutput ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.pb.c)
    set(hOutput ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.pb.h)

    set(${cFilepathResultVar} ${cOutput} PARENT_SCOPE)
    set(${hFilepathResultVar} ${hOutput} PARENT_SCOPE)

    get_filename_component(_dir ${protoFilepath} DIRECTORY)
    add_custom_command(OUTPUT ${cOutput} ${hOutput}
        COMMAND ${PROTOC_EXECUTABLE} -I${_dir} -I${PROTOC_INCLUDE_DIR}
            --nanopb_out=${CMAKE_CURRENT_BINARY_DIR} ${protoFilepath}
        MAIN_DEPENDENCY ${protoFilepath}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.pb.{c,h} from ${protoFilepath}"
        VERBATIM)
endfunction()

# Eat own dog food.
rpc_compile_proto(RPC_PROTO_C_FILE RPC_PROTO_H_FILE proto/rpc.proto)

##############################################################################

set(SOURCES
    src/message.cpp
    src/status.cpp
    ${RPC_PROTO_C_FILE}
    deps/${NANOPB_BASENAME}/pb_decode.c
    deps/${NANOPB_BASENAME}/pb_encode.c)

set_source_files_properties(src/message.cpp src/status.cpp PROPERTIES
    COMPILE_FLAGS "-std=c++11 -D__STDC_FORMAT_MACROS ${FEATURE_DEFS}")

add_library(rpc STATIC ${SOURCES})

target_include_directories(rpc
    PUBLIC deps/${NANOPB_BASENAME}
    PUBLIC include
    PRIVATE src
    PUBLIC ${PROJECT_BINARY_DIR}
    )

##############################################################################

if(NOT AVR)
    enable_testing()
    add_subdirectory(tests)
endif()
