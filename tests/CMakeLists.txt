##############################################################################
# Boost

# FindBoost.cmake uses BOOST_ROOT as a hint for where to look.
if(NOT DEFINED BOOST_ROOT)
    if(DEFINED ENV{BOOST_ROOT})
        set(BOOST_ROOT "$ENV{BOOST_ROOT}")
    else()
        message(WARNING "BOOST_ROOT is undefined.\nIf Boost libraries fail to "
            "be found, try setting -DBOOST_ROOT=/path/to/boost on the CMake "
            "command line.\n")
    endif()
endif()

# Linking statically to Boost is easier for prototyping. For multi-module
# projects we'll want to use shared libraries and defined -DBOOST_ALL_DYN_LINK
# or the like.
set(Boost_USE_STATIC_LIBS ON)

# Boost.Log was introduced in Boost version 1.54.0.
find_package(Boost 1.54.0 REQUIRED COMPONENTS )

##############################################################################

set(WIDGET_PROTO_SOURCES widget.proto)
set(WIDGET_PROTO_C_FILE widget.pb.c)
set(WIDGET_PROTO_H_FILE widget.pb.h)

# The --nanopb_out parameter serves double duty in protoc:
#   --nanopb_out=[generator-parameter:]output-directory
# In this case I'm using the optional generator-parameter part of it to pass
# '-x rpc-options.proto' to the nanopb generator, which suppresses a
#   #include "rpc-options.pb.h"
# from being generated in output files which import rpc-options.proto.
# rpc-options.proto is never itself compiled, so no such header file will ever
# exist. Is this flag documented somewhere in nanopb? Of course not.
add_custom_command(OUTPUT ${WIDGET_PROTO_C_FILE} ${WIDGET_PROTO_H_FILE}
    COMMAND ${PROTOC} -I. -I${PROJECT_SOURCE_DIR}/deps/${NANOPB_BASENAME}/generator/proto
        --nanopb_out=-x\ rpc-options.proto:${CMAKE_CURRENT_BINARY_DIR} ${WIDGET_PROTO_SOURCES}
    MAIN_DEPENDENCY ${PROTO_SOURCE_DIR}/${WIDGET_PROTO_SOURCES}
    WORKING_DIRECTORY ${PROTO_SOURCE_DIR}
    COMMENT "Generating test Widget protocol buffers ..."
    VERBATIM)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

set_source_files_properties(
    get-attribute.cpp
    set-attribute.cpp
    broadcast-attribute.cpp
    fire-method.cpp
    gen-widget.pb.cpp
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -ggdb ${FEATURE_DEFS}")

add_executable(get-attribute get-attribute.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(get-attribute
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(get-attribute rpc)
add_test(NAME get-attribute COMMAND get-attribute)

add_executable(set-attribute set-attribute.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(set-attribute
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(set-attribute rpc)
add_test(NAME set-attribute COMMAND set-attribute)

add_executable(broadcast-attribute broadcast-attribute.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(broadcast-attribute
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(broadcast-attribute rpc)
add_test(NAME broadcast-attribute COMMAND broadcast-attribute)

add_executable(fire-method fire-method.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(fire-method
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(fire-method rpc)
add_test(NAME fire-method COMMAND fire-method)
