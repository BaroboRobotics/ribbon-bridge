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

set(ROBOT_PROTO_SOURCES robot.proto)
#set(ROBOT_PROTO_OPTIONS robot.options)
set(ROBOT_PROTO_C_FILE robot.pb.c)
set(ROBOT_PROTO_H_FILE robot.pb.h)

add_custom_command(OUTPUT ${ROBOT_PROTO_C_FILE} ${ROBOT_PROTO_H_FILE}
    COMMAND ${PROTOC} --nanopb_out=${CMAKE_CURRENT_BINARY_DIR} ${ROBOT_PROTO_SOURCES}
    MAIN_DEPENDENCY ${PROTO_SOURCE_DIR}/${ROBOT_PROTO_SOURCES}
    #    DEPENDS ${PROTO_SOURCE_DIR}/${ROBOT_PROTO_OPTIONS}
    WORKING_DIRECTORY ${PROTO_SOURCE_DIR}
    COMMENT "Generating Robot protocol buffers ..."
    VERBATIM)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
set_source_files_properties(method-fire.cpp gen-robot.pb.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
add_executable(method-fire method-fire.cpp gen-robot.pb.cpp ${ROBOT_PROTO_C_FILE})
target_include_directories(method-fire
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(method-fire rpc)
add_test(NAME method-fire COMMAND method-fire)

#set_source_files_properties(attribute-set.cpp gen-robot.pb.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
#add_executable(attribute-set attribute-set.cpp gen-robot.pb.cpp ${ROBOT_PROTO_C_FILE})
#target_include_directories(attribute-set PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#target_link_libraries(attribute-set rpc)
#add_test(NAME attribute-set COMMAND attribute-set)
